# Copyright (c) 2023 Patrick S. Klein (@libklein)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import copy
import time

import routingblocks as rb
import random


def create_random_solution(evaluation: rb.Evaluation, instance: rb.Instance):
    customer_vertex_ids = [x.vertex_id for x in instance.customers]
    random.shuffle(customer_vertex_ids)

    # Draw a sequence of positions where to split
    number_of_splits = random.randint(1, len(customer_vertex_ids) // 2)
    split_positions = [0, *sorted(random.sample(range(1, len(customer_vertex_ids) - 1), number_of_splits)),
                       len(customer_vertex_ids)]
    # Create routes according to the split positions. Each route is a list of customer vertex ids.
    routes = [customer_vertex_ids[route_start_index:route_end_index] for route_start_index, route_end_index in
              zip(split_positions, split_positions[1:])]
    # Create RoutingBlocks Route objects
    routes = [rb.create_route(evaluation, instance, route) for route in routes]
    # Create RoutingBlocks Solution object
    return rb.Solution(evaluation, instance, routes)


def alns(instance: rb.Instance, vehicle_storage_capacity: float, vehicle_battery_capacity_time: float,
         number_of_iterations: int = 100, min_vertex_removal_factor: float = 0.2,
         max_vertex_removal_factor: float = 0.4):
    evaluation = rb.adptw.Evaluation(vehicle_battery_capacity_time, vehicle_storage_capacity)
    # Set the penalty factors used to penalize violations of the time window, the
    # vehicle capacity, and the resource constraints
    evaluation.overload_penalty_factor = 100.
    evaluation.resource_penalty_factor = 100.
    evaluation.time_shift_penalty_factor = 100.

    local_search = rb.LocalSearch(instance, evaluation, None, rb.BestImprovementPivotingRule())
    # Create a set of allowed arcs
    arc_set = rb.ArcSet(instance.number_of_vertices)

    # Create a set of operators that will be used later when calling the local search
    operators = [
        rb.operators.SwapOperator_0_1(instance, arc_set),
        rb.operators.SwapOperator_1_1(instance, arc_set),
        rb.operators.InsertStationOperator(instance),
        rb.operators.RemoveStationOperator(instance),
    ]

    # Create a random engine and seed it with the current time
    randgen = rb.Random(time.time_ns())
    # Create an ALNS solver.
    # Smoothing factor determines the weight of historic performance when selecting an operator.
    smoothing_factor = 0.4
    alns = rb.AdaptiveLargeNeighborhood(randgen, smoothing_factor)

    # Register some operators with the ALNS solver
    alns.add_repair_operator(rb.operators.RandomInsertionOperator(randgen))
    alns.add_repair_operator(rb.operators.BestInsertionOperator(instance,
                                                                rb.operators.blink_selector_factory(
                                                                    blink_probability=0.1, randgen=randgen)))
    alns.add_destroy_operator(rb.operators.RandomRemovalOperator(randgen))
    alns.add_destroy_operator(rb.operators.WorstRemovalOperator(instance,
                                                                rb.operators.blink_selector_factory(
                                                                    blink_probability=0.1, randgen=randgen)))

    # Generate a random starting solution
    best_solution = create_random_solution(evaluation, instance)
    for i in range(1, number_of_iterations + 1):
        current_solution = copy.copy(best_solution)
        # Perturb the current solution
        number_of_vertices_to_remove = int(random.uniform(min_vertex_removal_factor, max_vertex_removal_factor) * sum(
            len(route) - 2 for route in current_solution))
        picked_operators = alns.generate(evaluation, current_solution, number_of_vertices_to_remove)

        # Search the neighborhood of the current solution. This modifies the solution in-place.
        local_search.optimize(current_solution, operators)

        if current_solution.cost < best_solution.cost:
            best_solution = current_solution
            print(f"New best solution found: {best_solution.cost} ({best_solution.feasible})")
            # Update the ALNS solver with the performance of the operators used in the last iteration
            # We assign a score of '4' to the operators that were used to improve the solution
            alns.collect_score(*picked_operators, 4)
        else:
            # Update the ALNS solver with the performance of the operators used in the last iteration
            # We assign a score of '0' to the operators that were not used to improve the solution
            alns.collect_score(*picked_operators, 0)

        # Calculate new operator weights based on the last period
        if i % 20 == 0:
            alns.adapt_operator_weights()

    return best_solution
