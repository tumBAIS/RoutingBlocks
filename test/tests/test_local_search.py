from __future__ import annotations

import copy
from itertools import islice

import pytest
from typing import Optional

import helpers
import routingblocks

from fixtures import *

from routingblocks import adptw


class MockLSOperator(routingblocks.LocalSearchOperator):
    def __init__(self):
        routingblocks.LocalSearchOperator.__init__(self)
        self.ops = []

    def finalize_search(self) -> None:
        self.ops.append("finalize_search")

    def find_next_improving_move(self, arg0: routingblocks.Evaluation, arg2, arg3):
        self.ops.append("find_next_improving_move")
        return None

    def prepare_search(self, arg0) -> None:
        self.ops.append("prepare_search")

    def reset(self) -> None:
        self.ops = []


class MockPivotingRule(routingblocks.PivotingRule):
    def __init__(self, always_abort: bool = True):
        routingblocks.PivotingRule.__init__(self)
        self.always_abort = always_abort
        self.ops = []

    def continue_search(self, found_improving_move: routingblocks.Move, delta_cost: float,
                        solution: routingblocks.Solution) -> bool:
        self.ops.append("continue_search")
        return not self.always_abort

    def select_move(self, solution: routingblocks.Solution) -> Optional[routingblocks.Move]:
        self.ops.append("select_move")
        return None


import routingblocks as rb


class SplitRouteMove(rb.Move):
    """
    Splits a route into two routes. The passed location will be the first node of the second route.
    """

    def __init__(self, location: rb.NodeLocation):
        rb.Move.__init__(self)
        self.location = location

    def apply(self, instance: rb.Instance, solution: rb.Solution) -> None:
        # Create a new route
        solution.add_route()
        new_route_index = len(solution) - 1

        # Swap the segment [location.position, end] of the route to be split with an empty segment of the new route
        solution.exchange_segment(self.location.route, self.location.position, len(solution[self.location.route]) - 1,
                                  new_route_index, 1, 1)

    def get_cost_delta(self, evaluation: rb.Evaluation, instance: rb.Instance,
                       solution: rb.Solution) -> float:
        split_route = solution[self.location.route]
        cost_of_first_route_after_split = rb.evaluate_splice(evaluation, instance, split_route,
                                                             self.location.position, len(split_route) - 1)
        cost_of_second_route_after_split = rb.evaluate_splice(evaluation, instance, split_route,
                                                              1, self.location.position)

        original_route_cost = solution[self.location.route].cost
        return cost_of_first_route_after_split + cost_of_second_route_after_split - original_route_cost


class SplitRouteOperator(rb.LocalSearchOperator):
    def __init__(self, instance: rb.Instance):
        rb.LocalSearchOperator.__init__(self)
        self.instance = instance

    def _increment_location(self, solution: rb.Solution, location: rb.NodeLocation):
        """
        Increments the given location to the next possible split location. Modifies the passed location in-place.
        Returns None if no further splits are possible.
        :param solution: The solution to be split
        :param location: The location to be incremented
        :return: The incremented location or None if the solution is exhausted
        """
        location.position += 1
        # Move to the next route if the current one is exhausted
        if location.position > len(solution[location.route]) - 1:
            location.route += 1
            location.position = 1
        # No further splits possible
        if location.route >= len(solution):
            return None
        return location

    def _recover_from_move(self, solution: rb.Solution, move: Optional[SplitRouteMove]) -> Optional[rb.NodeLocation]:
        """
        Recovers the state of the search from the given move.
        """
        # If no move was given, start at the beginning
        if move is None:
            return rb.NodeLocation(0, 1)

        # Otherwise continue at the next location
        next_location = self._increment_location(solution, copy.copy(move.location))
        return next_location

    def finalize_search(self) -> None:
        # No cleanup needed
        pass

    def prepare_search(self, solution: rb.Solution) -> None:
        # No preparation needed
        pass

    def find_next_improving_move(self, evaluation: rb.Evaluation, solution: rb.Solution,
                                 last_evaluated_move: rb.Move) -> Optional[rb.Move]:
        assert isinstance(last_evaluated_move, SplitRouteMove) or last_evaluated_move is None
        next_move_location = self._recover_from_move(solution, last_evaluated_move)

        # Iterate over all possible split locations
        while next_move_location is not None:
            next_move = SplitRouteMove(next_move_location)
            # Evaluate the corresponding move
            if next_move.get_cost_delta(evaluation, self.instance, solution) < -1e-2:
                # If the move is improving, return it
                return next_move
            # Otherwise continue with the next location
            next_move_location = self._increment_location(solution, next_move_location)
        # Terminate the search if no improving move was found
        return None


def test_local_search_op(instance):
    py_instance: helpers.Instance = instance[0]
    instance: routingblocks.Instance = instance[1]
    evaluation = adptw.Evaluation(py_instance.parameters.battery_capacity_time,
                                  sum(x.demand for x in py_instance.customers) // 2)
    evaluation.overload_penalty_factor = 10000.
    evaluation.resource_penalty_factor = 0.
    evaluation.time_shift_penalty_factor = 0.

    customers = list(instance.customers)
    route = routingblocks.create_route(evaluation, instance,
                                       [x.vertex_id for x in customers])
    print(route, route.cost, route.cost_components)
    solution = routingblocks.Solution(evaluation, instance, [route])
    assert solution.cost > 0
    # Should find exactly two moves
    operator = SplitRouteOperator(instance)
    pivoting_rule = rb.BestImprovementPivotingRule()

    local_search = routingblocks.LocalSearch(instance, evaluation, None, pivoting_rule)

    local_search.optimize(solution, [operator])


class BestImprovementWithBlinksPivotingRule(routingblocks.PivotingRule):
    def __init__(self, blink_probability: float, randgen: routingblocks.Random):
        routingblocks.PivotingRule.__init__(self)
        self._blink_probability = blink_probability
        self._randgen = randgen
        self._best_move = None
        self._best_delta_cost = -1e-2

    def continue_search(self, found_improving_move: routingblocks.Move, delta_cost: float,
                        solution: routingblocks.Solution) -> bool:
        if delta_cost < self._best_delta_cost:
            self._best_move = found_improving_move
            self._best_delta_cost = delta_cost
            # If we do not blink, we can stop the search. Otherwise we continue.
            # This ensures that we always return the best found move, even if only one is found and that one is blinked.
            if self._randgen.uniform(0.0, 1.0) >= self._blink_probability:
                return False
        return True

    def select_move(self, solution: routingblocks.Solution) -> Optional[routingblocks.Move]:
        move = self._best_move
        self._best_move = None
        self._best_delta_cost = -1e-2
        return move


def test_local_search_blink_pivot(instance):
    py_instance: helpers.Instance = instance[0]
    instance: routingblocks.Instance = instance[1]
    evaluation = adptw.Evaluation(py_instance.parameters.battery_capacity_time, py_instance.parameters.capacity)

    route = routingblocks.create_route(evaluation, instance,
                                       [station.vertex_id for station in islice(instance.stations, 1, 3)])
    solution = routingblocks.Solution(evaluation, instance, [route])
    assert solution.cost > 0
    # Should find exactly two moves
    operator = routingblocks.operators.RemoveStationOperator(instance)
    pivoting_rule = BestImprovementWithBlinksPivotingRule(blink_probability=0.5, randgen=routingblocks.Random(0))

    local_search = routingblocks.LocalSearch(instance, evaluation, None, pivoting_rule)

    local_search.optimize(solution, [operator])


@pytest.mark.parametrize("always_abort,expected_operations", [
    (True, ["continue_search", "select_move"]),
    (False, ["continue_search", "continue_search", "select_move"])
])
def test_local_search_custom_pivot_rule(instance, always_abort, expected_operations):
    py_instance: helpers.Instance = instance[0]
    instance: routingblocks.Instance = instance[1]
    evaluation = adptw.Evaluation(py_instance.parameters.battery_capacity_time, py_instance.parameters.capacity)

    route = routingblocks.create_route(evaluation, instance,
                                       [station.vertex_id for station in islice(instance.stations, 1, 3)])
    solution = routingblocks.Solution(evaluation, instance, [route])
    assert solution.cost > 0
    # Should find exactly two moves
    operator = routingblocks.operators.RemoveStationOperator(instance)
    pivoting_rule = MockPivotingRule(always_abort=always_abort)

    local_search = routingblocks.LocalSearch(instance, evaluation, None, pivoting_rule)

    local_search.optimize(solution, [operator])
    assert pivoting_rule.ops == expected_operations


@pytest.fixture
def local_search_and_solution(instance, random_solution_factory, randgen):
    py_instance: helpers.Instance = instance[0]
    instance: routingblocks.Instance = instance[1]
    evaluation = adptw.Evaluation(py_instance.parameters.battery_capacity_time, py_instance.parameters.capacity)
    solution = random_solution_factory(instance=instance, evaluation=evaluation)
    pivoting_rule = routingblocks.BestImprovementPivotingRule()
    return routingblocks.LocalSearch(instance, evaluation, None, pivoting_rule), solution


def test_local_search_optimize_inplace(local_search_and_solution):
    local_search, solution = local_search_and_solution
    route = solution[0]
    local_search.optimize(solution, [])
    assert solution[0] is route


def test_local_search_custom_operator(local_search_and_solution):
    local_search, solution = local_search_and_solution
    operator = MockLSOperator()

    local_search.optimize(solution, [operator])
    assert operator.ops == ["prepare_search", "find_next_improving_move", "finalize_search"]


def test_local_search_custom_operators_lifetime(local_search_and_solution):
    local_search, solution = local_search_and_solution
    local_search.optimize(solution, [MockLSOperator()])


def test_neighborhood_iterator(random_solution):
    *_, solution = random_solution
    expected_moves = set()
    for i, route_i in enumerate(solution):
        for u, node_u in enumerate(route_i):
            for j, route_j in enumerate(solution):
                for v, node_v in enumerate(route_j):
                    expected_moves.add((node_u, node_v))

    actual_moves = set((x.origin_node, x.target_node) for x in routingblocks.iter_neighborhood(solution))
    assert actual_moves == expected_moves
