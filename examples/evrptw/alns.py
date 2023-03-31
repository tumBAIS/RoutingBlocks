import copy
import math
import random
import time
from copy import deepcopy
import itertools
import sys

from routingblocks.operators import best_insert, WorstRemovalOperator, blink_selector_factory, first_move_selector
from routingblocks.operators.route_removal import RouteRemoveOperator
from .operators import create_shaw_remove_operator, create_related_remove_operator
from .instance import Instance as ADPTWInstance
from .utility import distribute_randomly
from .parameters import ALNSParams

import routingblocks


def create_reduced_arc_set(instance: routingblocks.Instance, py_instance: ADPTWInstance, n_neighbours: int) -> routingblocks.ArcSet:
    arc_set = routingblocks.ArcSet(instance.number_of_vertices)
    for i in range(1, instance.number_of_vertices):
        sorted_arcs = sorted(
            ((j, py_instance.arcs[instance.get_vertex(i).str_id, instance.get_vertex(j).str_id]) for j in
             range(1, instance.number_of_vertices)), key=lambda arc: arc[1].cost)
        for j, _ in sorted_arcs[n_neighbours:]:
            arc_set.forbid_arc(i, j)
    return arc_set


class CostComponentTracker:
    def __init__(self, window_length: int):
        self._last_penalites = []
        self._window_length = window_length

    def _get_cost_components(self, solution: routingblocks.Solution):
        return solution.cost_components[1:]

    @property
    def window_feasibility_ratios(self):
        assert len(self._last_penalites) > 0
        return [
            sum(1 for pen in self._last_penalites if pen[i] <= 0.01) / len(self._last_penalites)
            for i in range(len(self._last_penalites[0]))
        ]

    def register(self, solution: routingblocks.Solution):
        self._last_penalites.append(self._get_cost_components(solution))
        if len(self._last_penalites) > self._window_length:
            self._last_penalites.pop(0)


class ALNS:
    def __init__(self, evaluation: routingblocks.Evaluation, py_instance: ADPTWInstance, cpp_instance: routingblocks.Instance,
                 params: ALNSParams, seed: int):
        self._evaluation = evaluation
        self._py_instance = py_instance
        self._cpp_instance = cpp_instance
        self._params = params
        # Initialize random engines
        self._random = routingblocks.Random(seed)
        self._py_random = random.Random(seed)

        # Create and configure algorithmic components
        self._adaptive_large_neighborhood = routingblocks.AdaptiveLargeNeighborhood(self._random,
                                                                            self._params.adaptive_smoothing_factor)
        self._local_search = routingblocks.LocalSearch(self._cpp_instance, evaluation, None)
        self._local_search.set_use_best_improvement(self._params.use_best_improvement)

        # Compute the granular neighborhood
        self._reduced_arc_set = create_reduced_arc_set(self._cpp_instance, self._py_instance, self._params.granularity)

        # Create specialized FRVCP solver
        self._frvcp = routingblocks.adptw.FRVCP(self._cpp_instance, self._py_instance.parameters.battery_capacity_time)

        # Create cost component tracker
        self._cost_component_tracker = \
            CostComponentTracker(self._params.penalty_period_length)

        # Set the initial penalty factors
        self._evaluation.penalty_factors = [1., *self._params.initial_penalties]

        # Initialize state
        self._current_solution: routingblocks.Solution = None
        self._best_solution = None
        self._best_feasible_solution = None
        self._start_time = None
        self._iters = 0
        self._ls_iters = 0
        self._iters_since_improvement = 0

        # Initialize state for vehicle minimization mechanic
        self._reached_vehicle_lb = False
        self._last_vehicle_decrease_iter = 0
        self._saved_penalties = None
        self._boosted_penalties = False
        self._vehicle_lb = int(math.ceil(
            sum(x.demand for x in py_instance.customers) / py_instance.parameters.capacity))

        # Create/Register operators
        self._configure_local_search_operators()
        self._configure_repair_operators()
        self._configure_destroy_operators()

    def _configure_local_search_operators(self):
        self._operators = [
            routingblocks.SwapOperator_0_1(self._cpp_instance, self._reduced_arc_set),
            routingblocks.SwapOperator_0_2(self._cpp_instance, self._reduced_arc_set),
            routingblocks.SwapOperator_0_3(self._cpp_instance, self._reduced_arc_set),
            routingblocks.SwapOperator_1_1(self._cpp_instance, self._reduced_arc_set),
            routingblocks.InterRouteTwoOptOperator(self._cpp_instance, self._reduced_arc_set),
            routingblocks.InsertStationOperator(self._cpp_instance),
            routingblocks.RemoveStationOperator(self._cpp_instance)
        ]

    def _configure_destroy_operators(self):
        self._adaptive_large_neighborhood.add_destroy_operator(routingblocks.RandomRemoveOperator(self._random))
        self._adaptive_large_neighborhood.add_destroy_operator(RouteRemoveOperator(self._random))
        self._adaptive_large_neighborhood.add_destroy_operator(
            create_related_remove_operator(self._py_instance, self._cpp_instance, self._random,
                                           self._params.tw_shift_weight, self._params.slack_weight))
        self._adaptive_large_neighborhood.add_destroy_operator(create_shaw_remove_operator(
            self._py_instance, self._cpp_instance, self._random,
            distance_weight=self._params.distance_weight, demand_weight=self._params.demand_weight,
            time_weight=self._params.time_weight, shaw_exponent=self._params.shaw_exponent))
        self._adaptive_large_neighborhood.add_destroy_operator(
            WorstRemovalOperator(self._cpp_instance,
                                 blink_selector_factory(self._params.worst_removal_blink_probability,
                                                        self._random)))

    def _configure_repair_operators(self):
        self._adaptive_large_neighborhood.add_repair_operator(routingblocks.RandomInsertionOperator(self._random))
        self._adaptive_large_neighborhood.add_repair_operator(
            best_insert.BestInsertionOperator(self._cpp_instance, first_move_selector))
        self._adaptive_large_neighborhood.add_repair_operator(
            best_insert.BestInsertionOperator(self._cpp_instance,
                                              blink_selector_factory(self._params.best_insertion_blink_probability,
                                                                     self._random)))

    def _apply_dp(self, _solution: routingblocks.Solution) -> routingblocks.Solution:
        optimized_routes = [routingblocks.create_route(self._evaluation, self._cpp_instance,
                                               self._frvcp.optimize([x.vertex_id for x in route])[1:-1]) for route
                            in
                            _solution]
        return routingblocks.Solution(self._evaluation, self._cpp_instance,
                              [route for route in optimized_routes if not route.empty or not _solution.feasible])

    def _generate_random_solution(self):
        customers = [x.vertex_id for x in self._cpp_instance.customers]
        while True:
            sol = routingblocks.Solution(self._evaluation, self._cpp_instance,
                                 [routingblocks.create_route(self._evaluation, self._cpp_instance, r) for r in
                                  distribute_randomly(customers, self._cpp_instance.fleet_size,
                                                      self._random)])
            self._local_search.optimize(sol, self._operators)
            yield self._apply_dp(sol)

    @property
    def _current_obj(self):
        return self._current_solution.cost if self._current_solution else sys.float_info.max

    @property
    def _best_obj(self):
        return self._best_solution.cost if self._best_solution else sys.float_info.max

    @property
    def _best_feasible_obj(self):
        return self._best_feasible_solution.cost if self._best_feasible_solution else sys.float_info.max

    def _make_feasible(self, solution: routingblocks.Solution):
        penalty_factors = self._evaluation.penalty_factors
        self._evaluation.penalty_factors = [penalty_factors[0], penalty_factors[1] * 100.,
                                            penalty_factors[2] * 100.,
                                            penalty_factors[3] * 100.]
        self._local_search.optimize(solution, self._operators)
        self._evaluation.penalty_factors = penalty_factors

    def _remove_vehicle(self, solution: routingblocks.Solution):
        # Reset penalty
        current_penalties = self._evaluation.penalty_factors
        for i in range(1, len(current_penalties)):
            current_penalties[i] = max(current_penalties[i], self._params.initial_penalties[i - 1] * 100)
        self._evaluation.penalty_factors = current_penalties
        self._boosted_penalties = True

        # Remove route
        fewest_customer_route = min(solution, key=lambda r: len(r))
        customers = [x.vertex_id for x in fewest_customer_route if x.vertex.is_customer]

        solution.remove_route(fewest_customer_route)

        reinsertion_operator = best_insert.BestInsertionOperator(self._cpp_instance, first_move_selector)
        reinsertion_operator.apply(self._evaluation, solution, customers)

    def _accept_solution(self, solution: routingblocks.Solution):
        score = 0
        solution_cost = solution.cost
        if solution_cost < self._current_obj:
            self._current_solution = copy.deepcopy(solution)
            score = self._params.new_improvement_score
        if solution_cost < self._best_obj:
            self._best_solution = copy.deepcopy(solution)
            print(
                f"[{self.elapsed:.2f}s {self._iters}, {self._iters_since_improvement}]: Found new best solution: {solution_cost} ({self._best_obj}, {len(solution)})")
            score = self._params.new_best_score

            if not solution.feasible:
                self._make_feasible(solution)
                solution_cost = solution.cost
        if solution.feasible:
            accept = False
            if self._best_feasible_solution is None:
                accept = True
            elif (solution_cost < self._best_feasible_obj and len(solution) == len(
                    self._best_feasible_solution)) or len(
                solution) < len(self._best_feasible_solution):
                accept = True

            if accept:
                if self._best_feasible_solution is None or len(self._best_feasible_solution) > len(solution):
                    self._last_vehicle_decrease_iter = self._iters
                    self._reached_vehicle_lb = False
                self._best_feasible_solution = copy.deepcopy(solution)
                if len(self._best_feasible_solution) > len(self._best_solution):
                    self._best_solution = copy.deepcopy(solution)
                if self._boosted_penalties:
                    self._boosted_penalties = False
                    self._evaluation.penalty_factors = self._saved_penalties
                print(
                    f"[{self.elapsed:.2f}s, {self._iters}, {self._iters_since_improvement}]: Found new best feasible solution: {solution_cost} ({self._best_feasible_obj}, {len(solution)})")
                score = self._params.new_best_feasible_score
        return score

    @property
    def elapsed(self):
        return time.time() - self._start_time

    def _remove_empty_routes(self, solution):
        new_routes = []
        for route in solution:
            if any(x.vertex.is_customer for x in route):
                new_routes.append(route)
        return routingblocks.Solution(self._evaluation, self._cpp_instance, new_routes)

    def _generate_solution_from_lns(self, seed_solution: routingblocks.Solution):
        sol = deepcopy(seed_solution)
        destroy_op, repair_op = self._adaptive_large_neighborhood.generate(self._evaluation, sol,
                                                                           int(self._cpp_instance.number_of_customers * self._random.uniform(
                                                                               self._params.min_removed_customer_percentage,
                                                                               self._params.max_removed_customer_percentage)))
        return sol, destroy_op, repair_op

    @property
    def _best_dist(self):
        return self._best_solution.cost_components[0]

    def run(self, time_limit: float, max_iterations: int, max_iterations_since_last_improvement: int):
        self._start_time = time.time()
        self._iters = self._iters_since_improvement = 1
        self._accept_solution(
            min(itertools.islice(self._generate_random_solution(), self._params.num_starting_solutions),
                key=lambda x: x.cost))
        self._current_solution = self._best_solution
        self._cost_component_tracker.register(self._current_solution)

        while (self._start_time + time_limit > time.time()) and (self._iters < max_iterations) and (
                self._iters_since_improvement < max_iterations_since_last_improvement):
            # Adaptively sample a solution from the
            _solution, *applied_lns_operators = self._generate_solution_from_lns(
                seed_solution=self._current_solution)

            # Optimize
            if (candidate_dist := _solution.cost_components[0]) < self._best_dist * (
                    1. + self._params.delta_local_search):
                if self._params.shuffle_operators:
                    self._py_random.shuffle(self._operators)
                self._local_search.optimize(_solution, self._operators)
                if candidate_dist < self._best_dist * (1. + self._params.delta_frvcp):
                    _solution = self._apply_dp(_solution)
                else:
                    _solution = self._remove_empty_routes(_solution)
                self._ls_iters += 1

            # Register the current solution with the cost component tracker
            self._cost_component_tracker.register(self._current_solution)

            # Accept solution and update weights
            score = self._accept_solution(_solution)
            self._adaptive_large_neighborhood.collect_score(*applied_lns_operators, score)
            # Track iterations
            if score == self._params.new_best_feasible_score:
                self._iters_since_improvement = 1
            else:
                self._iters_since_improvement += 1
            self._iters += 1

            # Adapt operator weights
            if self._iters % self._params.adaptive_period_length == 0:
                self._adaptive_large_neighborhood.adapt_operator_weights()

            # Adapt penalties
            if self._iters % self._params.penalty_period_length == 0:
                feasibility_rations = self._cost_component_tracker.window_feasibility_ratios
                new_factors = [1.] + [max(0.1, min(10000., penalty *
                                                   (
                                                       self._params.penalty_increase_factor if actual < target
                                                       else self._params.penalty_decrease_factor)))
                                      for actual, target, penalty in
                                      zip(feasibility_rations,
                                          self._params.target_feasibility_ratios,
                                          self._evaluation.penalty_factors[1:])
                                      ]
                new_factors[-1] = max(new_factors[-1], new_factors[-2])
                new_factors[-2] = new_factors[-1]
                self._evaluation.penalty_factors = new_factors

            # Trigger the vehicle minimization procedure
            if not self._reached_vehicle_lb:
                if (self._iters - self._last_vehicle_decrease_iter > self._params.vehicle_decrease_period_length) \
                        and (self._best_feasible_solution is not None):
                    self._last_vehicle_decrease_iter = self._ls_iters
                    self._reached_vehicle_lb = True
                    if not self._boosted_penalties:
                        self._saved_penalties = list(self._evaluation.penalty_factors)
                    if len(self._best_feasible_solution) > self._vehicle_lb:
                        while len(self._current_solution) > len(self._best_feasible_solution) - 1:
                            self._remove_vehicle(self._current_solution)
                        self._best_solution = self._current_solution
                        print(f"Decreased max number of vehicles to {len(self._best_feasible_solution) - 1}")
            else:
                if len(self._best_feasible_solution) > len(
                        self._current_solution) and (
                        self._ls_iters - self._last_vehicle_decrease_iter) > self._params.vehicle_decreased_search_period_length:
                    self._current_solution = copy.deepcopy(self._best_feasible_solution)
                    self._evaluation.penalty_factors = self._saved_penalties
                    self._boosted_penalties = False
                    self._best_solution = copy.deepcopy(self._best_feasible_solution)
                    print(f"Increased max number of vehicles to {len(self._current_solution)}")
        if self._best_feasible_solution is not None:
            return self._best_feasible_solution
        else:
            return self._best_solution
