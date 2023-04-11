import copy

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


def perturb(solution: rb.Solution, max_exchanges: int) -> rb.Solution:
    # Create a new solution by copying the current solution
    new_solution = copy.copy(solution)

    # Exchange random sequences between routes
    num_exchanges = random.randint(0, max_exchanges)
    for _ in range(num_exchanges):
        # Select two random routes
        route_1 = random.choice(new_solution)
        route_2 = random.choice(new_solution)
        if route_1 is route_2:
            continue
        # Select a random sequence of customers in route 1 that does not include the depot
        start_index_1 = random.randint(1, len(route_1) - 2)
        # end_index is exclusive
        end_index_1 = random.randint(start_index_1, len(route_1) - 1)
        # Do the same for the second route
        # Select a random sequence of customers in route 1 that does not include the depot
        start_index_2 = random.randint(1, len(route_2) - 2)
        # end_index is exclusive
        end_index_2 = random.randint(start_index_2, len(route_2) - 1)
        # Exchange the sequences
        new_solution.exchange_segment(route_1, start_index_1, end_index_1,
                                      route_2, start_index_2, end_index_2)
    return new_solution


def iterated_local_search(instance: rb.Instance, vehicle_storage_capacity: float, vehicle_battery_capacity_time: float):
    evaluation = rb.adptw.Evaluation(vehicle_battery_capacity_time, vehicle_storage_capacity)

    local_search = rb.LocalSearch(instance, evaluation, None)
    # Configure the local search to use a best-improvement pivoting rule
    local_search.set_use_best_improvement(True)
    # Create a set of allowed arcs
    arc_set = rb.ArcSet(instance.number_of_vertices)

    # Create a set of operators that will be used later when calling the local search
    operators = [
        rb.SwapOperator_0_1(instance, arc_set),
        rb.SwapOperator_1_1(instance, arc_set),
        rb.InsertStationOperator(instance),
        rb.RemoveStationOperator(instance),
    ]

    best_solution = create_random_solution(evaluation, instance)
    current_solution = copy.copy(best_solution)
    for i in range(10):
        # Search the neighborhood of the current solution. This modifies the solution in-place.
        local_search.optimize(current_solution, operators)
        if current_solution.cost < best_solution.cost:
            best_solution = current_solution
            print(f"New best solution found: {best_solution.cost}")

        # Perturb the current solution
        current_solution = perturb(current_solution, len(current_solution) // 2)

    return best_solution