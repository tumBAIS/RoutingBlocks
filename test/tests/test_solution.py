from __future__ import annotations

import itertools
import random
from collections import defaultdict
from copy import copy, deepcopy
from pathlib import Path
from typing import Tuple, Callable, Dict, List, TypeVar

import pytest

import helpers

from fixtures import *

try:
    import routingblocks as evrptw
except ModuleNotFoundError:
    pass


def assert_cost_correct(solution: evrptw.Solution, routes: List[evrptw.Route] = None):
    target_cost = sum((route.cost for route in routes), 0) if routes is not None else \
        sum((route.cost for route in solution), 0)
    assert solution.cost == target_cost


def compute_expected_locations(routes: List[evrptw.Route]):
    expected_locations = defaultdict(set)
    for route_index, route in enumerate(routes):
        for position, node in enumerate(route):
            expected_locations[node.vertex_id].add((route_index, position))
    return expected_locations


def assert_positions_correct(solution: evrptw.Solution, expected_locations=None):
    if expected_locations is None:
        expected_locations = compute_expected_locations(list(solution))

    for v_id, expected_vertex_locations in expected_locations.items():
        actual_vertex_locations = solution.find(v_id)
        assert set((location.route, location.position) for location in
                   actual_vertex_locations) == expected_vertex_locations, f'Solution {solution} does not track vertex with {v_id} at expected locations {expected_vertex_locations} correctly'


def test_empty_construction(adptw_instance: evrptw.Instance, mock_evaluation):
    instance: evrptw.Instance = adptw_instance
    num_routes = instance.fleet_size
    solution = evrptw.Solution(mock_evaluation, instance, num_routes)
    assert len(solution) == instance.fleet_size
    for route in solution:
        assert route.empty
    assert_cost_correct(solution)


def test_construction_from_routes(adptw_instance: evrptw.Instance, mock_evaluation):
    instance: evrptw.Instance = adptw_instance
    route = evrptw.create_route(mock_evaluation, instance, [x.id for x in instance.customers])

    routes = [route,
              *[evrptw.Route(mock_evaluation, instance) for _ in range(instance.fleet_size - 1)]]

    solution = evrptw.Solution(mock_evaluation, instance, routes)

    assert len(solution) == instance.fleet_size
    for expected_route, actual_route in zip(routes, solution):
        assert expected_route == actual_route
    assert_cost_correct(solution, routes)


@pytest.mark.parametrize('accessor', ['getitem', 'routes', 'iter'])
def test_solution_route_accessor(adptw_instance: evrptw.Instance, mock_evaluation, accessor):
    instance: evrptw.Instance = adptw_instance
    solution = evrptw.Solution(mock_evaluation, instance, [evrptw.Route(mock_evaluation, instance) for _ in range(5)])
    routes = list(solution)

    def access_route(index):
        if accessor == 'getitem':
            return solution[index]
        elif accessor == 'routes':
            return next(itertools.islice(solution.routes, index, index + 1))
        elif accessor == 'iter':
            return next(itertools.islice(solution, index, index + 1))

    for i in range(len(solution)):
        assert routes[i] is access_route(i)


@pytest.mark.parametrize("strategy", ['by_index', 'by_value'])
def test_solution_route_remove(random_solution, strategy):
    *_, solution = random_solution
    original_routes = deepcopy(list(solution))
    while len(solution) > 0:
        index = random.randint(0, len(solution) - 1)
        # It is possible (e.g., when we have two identical routes), that index points to the route with the higher
        # index. In this case, by_value will fail because the route with the lower index compares equal to the route
        # with the higher index, such that the one with the lower index will be removed.
        for i, route in enumerate(solution):
            if route == solution[index]:
                index = i
                break
        expected_routes = original_routes[:index] + original_routes[index + 1:]
        if strategy == 'by_index':
            del solution[index]
        elif strategy == 'by_value':
            solution.remove_route(solution[index])
        assert expected_routes == list(solution)
        assert_positions_correct(solution)
        assert_cost_correct(solution)

        original_routes = expected_routes


@pytest.mark.parametrize("prev_routes", [[[1, 2]], [[1, 2], []]])
@pytest.mark.parametrize("added_route", [[], [3, 4], None])
def test_solution_add_empty_route(instance, mock_evaluation, prev_routes, added_route):
    *_, instance = instance

    def assert_route_added(solution: evrptw.Solution, expected_routes: List[evrptw.Route]):
        # Should be equal by value
        assert list(solution) == expected_routes
        # But not by id
        assert solution[len(solution) - 1] is not expected_routes[len(expected_routes) - 1]
        assert_positions_correct(solution)
        assert_cost_correct(solution)

    def create_route(raw_route):
        if len(raw_route) > 0:
            return evrptw.create_route(mock_evaluation, instance, raw_route)
        else:
            return evrptw.Route(mock_evaluation, instance)

    prev_routes = [
        create_route(raw_route) for raw_route in prev_routes
    ]
    solution = evrptw.Solution(mock_evaluation, instance, prev_routes)
    if added_route is not None:
        added_route = create_route(added_route)
        solution.add_route(added_route)
    else:
        added_route = create_route([])
        solution.add_route()
    expected_routes = prev_routes + [added_route]
    assert_route_added(solution, expected_routes)


def test_vertex_lookup(adptw_instance: evrptw.Instance, random_routes_factory, mock_evaluation):
    instance: evrptw.Instance = adptw_instance
    # One station is missing, others may be present multiple times
    missing_station = list(instance.stations)[-1]
    vertices = [*instance.customers] + random.choices(list(instance.stations)[:-1],
                                                      k=int(instance.number_of_stations * 1.5))
    routes = random_routes_factory(instance=instance, vertices=vertices, n_routes=instance.fleet_size,
                                   evaluation=mock_evaluation)

    expected_locations = defaultdict(set)
    for route_index, route in enumerate(routes):
        for position, node in enumerate(route):
            expected_locations[node.vertex_id].add((route_index, position))

    solution = evrptw.Solution(mock_evaluation, instance, routes)

    assert_positions_correct(solution, expected_locations)

    assert solution.find(missing_station.id) == []


def create_full_solution(instance: evrptw.Instance, routes: List[evrptw.Route], evaluation: evrptw.Evaluation):
    return evrptw.Solution(evaluation, instance,
                           [*routes,
                            *[evrptw.Route(evaluation, instance) for _ in
                              range(instance.fleet_size - len(routes))]])


def test_interroute_exchange(adptw_instance: evrptw.Instance, mock_evaluation):
    instance: evrptw.Instance = adptw_instance
    customers = list(instance.customers)
    stations = list(instance.stations)
    route_1_vertices = [customers[0], stations[0], customers[1]]
    route_2_vertices = [customers[2], customers[3], stations[1], customers[4]]

    route_1 = evrptw.create_route(mock_evaluation, instance, [x.id for x in route_1_vertices])
    route_2 = evrptw.create_route(mock_evaluation, instance, [x.id for x in route_2_vertices])
    solution = create_full_solution(instance, [route_1, route_2], mock_evaluation)

    # exchange customers[0] with [customers[2], customers[3]]
    solution.exchange_segment(0, 1, 2, 1, 1, 3)
    assert [x.vertex_id for x in solution[0]][1:-1] == [customers[2].id, customers[3].id, stations[0].id,
                                                        customers[1].id]
    assert [x.vertex_id for x in solution[1]][1:-1] == [customers[0].id, stations[1].id, customers[4].id]

    # Locations should be updated as well. Will not be correct for depot vertices
    assert_positions_correct(solution)
    assert_cost_correct(solution)


def test_interroute_move(adptw_instance: evrptw.Instance, mock_evaluation: evrptw.Evaluation):
    instance: evrptw.Instance = adptw_instance
    customers = list(instance.customers)
    stations = list(instance.stations)
    route_1_vertices = [customers[0], stations[0], customers[1]]
    route_2_vertices = [customers[2], customers[3], stations[1], customers[4]]

    route_1 = evrptw.create_route(mock_evaluation, instance, [x.id for x in route_1_vertices])
    route_2 = evrptw.create_route(mock_evaluation, instance, [x.id for x in route_2_vertices])
    solution = create_full_solution(instance, [route_1, route_2], mock_evaluation)

    # exchange customers[0] with [] at position 1, effectively inserting customers[0] after the depot
    solution.exchange_segment(0, 1, 2, 1, 1, 1)
    assert [x.vertex_id for x in solution[0]][1:-1] == [stations[0].id, customers[1].id]
    assert [x.vertex_id for x in solution[1]][1:-1] == [customers[0].id, customers[2].id, customers[3].id,
                                                        stations[1].id, customers[4].id]

    # Locations should be updated as well. Will not be correct for depot vertices
    assert_positions_correct(solution)
    assert_cost_correct(solution)


def test_randomized_exchange(adptw_instance: evrptw.Instance, random_routes_factory,
                             mock_evaluation: evrptw.Evaluation):
    instance: evrptw.Instance = adptw_instance
    routes: List[evrptw.Route] = random_routes_factory(instance=instance,
                                                       vertices=[*instance.customers, *instance.stations,
                                                                 *instance.stations, *instance.stations],
                                                       n_routes=instance.fleet_size, evaluation=mock_evaluation)
    solution: evrptw.Solution = evrptw.Solution(mock_evaluation, instance, routes)

    def pick_route_segment(route):
        start = random.randint(1, len(route) - 2)
        end = random.randint(start + 1, len(route) - 1)
        return start, end

    # Exchange random segments
    num_tests = 100
    while num_tests > 0:
        route_1 = random.randrange(len(solution))
        route_2 = random.randrange(len(solution))

        if routes[route_1].empty or routes[route_2].empty:
            continue

        start_1, end_1 = pick_route_segment(solution[route_1])
        start_2, end_2 = pick_route_segment(solution[route_2])

        if route_1 == route_2:
            # Pick such that intervals do not overlap
            if len(solution[route_1]) < 5:
                continue
            positions = sorted(random.sample(range(1, len(solution[route_1])), k=4))
            start_1, end_1, start_2, end_2 = positions

        solution.exchange_segment(route_1, start_1, end_1, route_2, start_2, end_2)
        # Expected move
        routes[route_1].exchange_segments(start_1, end_1, start_2, end_2, routes[route_2])

        assert solution[route_1] == routes[route_1]
        assert solution[route_2] == routes[route_2]

        assert_cost_correct(solution, routes)
        assert_positions_correct(solution, compute_expected_locations(routes))
        num_tests -= 1


def test_intraroute_exchange(adptw_instance: evrptw.Instance, mock_evaluation: evrptw.Evaluation):
    instance: evrptw.Instance = adptw_instance
    customers = list(instance.customers)
    route_vertices = [customers[0], customers[1], customers[2]]
    route = evrptw.create_route(mock_evaluation, instance, [x.id for x in route_vertices])
    solution = create_full_solution(instance, [route], mock_evaluation)

    # Test special case of exchaging [customers[0]] with [customers[1], customers[2]].
    # Here, begin of second segment will equal end of first segment
    begin_1 = 1
    end_1 = 2
    begin_2 = 2
    end_2 = 4
    solution.exchange_segment(0, begin_1, end_1, 0, begin_2, end_2)
    assert [x.vertex_id for x in solution[0]][1:-1] == [customers[1].id, customers[2].id, customers[0].id]

    assert_cost_correct(solution)
    assert_positions_correct(solution)


def test_solution_copy(random_solution):
    *_, solution = random_solution

    ref_solution = solution
    assert id(ref_solution) == id(solution)
    copy_solution = copy(solution)
    assert id(copy_solution) != id(solution)
    assert copy_solution == solution

    deepcopy_solution = deepcopy(solution)
    assert id(deepcopy_solution) != id(solution)
    assert deepcopy_solution == solution


@pytest.mark.parametrize('repeat', range(100))
def test_vertex_removal(mock_evaluation: evrptw.Evaluation, adptw_instance: evrptw.Instance, random_routes_factory,
                        repeat):
    instance: evrptw.Instance = adptw_instance
    routes: List[evrptw.Route] = random_routes_factory(evaluation=mock_evaluation, instance=instance,
                                                       n_routes=instance.fleet_size)
    solution = evrptw.Solution(mock_evaluation, instance, routes)

    # Removed vertex should not be in solution anymore
    while True:
        node_positions = set(
            (route_id, pos) for route_id, route in enumerate(routes) for pos, x in enumerate(route) if
            x.vertex_id != instance.depot.id)
        if len(node_positions) == 0:
            break

        route_id, pos = node_positions.pop()
        route = routes[route_id]
        removed_vertex_id = solution[route_id][pos].vertex_id

        expected_positions = solution.find(removed_vertex_id)
        expected_positions.remove(evrptw.NodeLocation(route_id, pos))
        for expected_position in expected_positions:
            if expected_position.route == route_id and expected_position.position > pos:
                expected_position.position -= 1

        solution.remove_vertex(evrptw.NodeLocation(route_id, pos))

        # Expected
        route.remove_segment(pos, pos + 1)

        # Node should not be in solution anymore
        assert solution[route_id] == route
        assert solution.find(removed_vertex_id) == expected_positions
        # Solution should be updated
        assert_cost_correct(solution, routes)


@pytest.mark.parametrize("sort_positions", [True, False])
def test_solution_remove_vertices(sort_positions: bool, random_solution_factory, adptw_instance: evrptw.Instance,
                                  mock_evaluation: evrptw.Evaluation):
    instance: evrptw.Instance = adptw_instance
    solution = random_solution_factory(instance, mock_evaluation,
                                       list(instance.customers) + list(instance.stations), 2)
    positions = [evrptw.NodeLocation(route, pos) for route in range(len(solution)) for pos in
                 range(1, len(solution[route]) - 1)]
    random.shuffle(positions)
    positions = positions[:int(len(positions) / 2)]

    if sort_positions:
        positions.sort()
    else:
        # Make sure that the positions are not sorted
        while all(x < y for x, y in zip(positions, positions[1:])):
            random.shuffle(positions)

    py_pos = {(x.route, x.position) for x in positions}
    expected_routes = [
        [x.vertex_id for pos, x in enumerate(solution[route]) if (route, pos) not in py_pos] for route in
        range(len(solution))
    ]

    solution.remove_vertices(positions)
    assert [[x.vertex_id for x in route] for route in solution] == expected_routes
    assert_positions_correct(solution)
    assert_cost_correct(solution)


@pytest.mark.parametrize("sorted_locations", [True, False])
def test_solution_insert_vertices(sorted_locations: bool, adptw_instance: evrptw.Instance, random_solution_factory,
                                  mock_evaluation):
    instance: evrptw.Instance = adptw_instance
    vertices = [x.vertex_id for x in itertools.chain(instance.customers, instance.stations)]
    vertices_in_solution = random.choices(vertices, k=len(vertices) // 2)

    solution = random_solution_factory(instance=instance, evaluation=mock_evaluation,
                                       vertices=vertices_in_solution, n_routes=2)

    vertices_to_insert = random.choices([x for x in vertices if x not in vertices_in_solution],
                                        k=len(vertices_in_solution))
    positions = [evrptw.NodeLocation(route, pos) for route in range(len(solution)) for pos in
                 range(1, len(solution[route]) - 1)]
    random.shuffle(positions)

    sorted_index_order = sorted(range(len(positions)), key=lambda index: positions[index], reverse=True)

    expected_solution = copy(solution)
    for index in sorted_index_order:
        pos, vertex_id = positions[index], vertices_to_insert[index]
        expected_solution.insert_vertex_after(pos, vertex_id)

    actual_solution = copy(solution)
    if sorted_locations:
        vertices_and_positions = [(vertices_to_insert[index], positions[index]) for index in sorted_index_order]
    else:
        vertices_and_positions = list(zip(vertices_to_insert, positions))

    actual_solution.insert_vertices_after(vertices_and_positions)

    assert expected_solution == actual_solution

    assert_positions_correct(solution)
    assert_cost_correct(solution)


@pytest.mark.parametrize("raw_routes", [
    [],  # Empty solution
    [[], []],  # Solution with only empty routes
    [[1, 3], [2, 6], [7, 4, 5], []]
])
def test_solution_node_iterators(adptw_instance: evrptw.Instance, mock_evaluation: evrptw.Evaluation,
                                 raw_routes: List[List[int]]):
    instance = adptw_instance
    solution = evrptw.Solution(mock_evaluation, instance,
                               [routingblocks.create_route(mock_evaluation, instance, route) for route in raw_routes])
    expected_number_of_non_depot_nodes = sum(len(route) for route in raw_routes)
    expected_number_of_insertion_points = expected_number_of_non_depot_nodes + len(
        raw_routes)  # Begin depot is a valid insertion point
    # number of nodes should be correct
    assert solution.number_of_non_depot_nodes == expected_number_of_non_depot_nodes
    # number of insertion spots should be correct
    assert solution.number_of_insertion_points == expected_number_of_insertion_points
    expected_non_depot_nodes = []
    expected_insertion_points = []
    for route_index, raw_route in enumerate(raw_routes):
        for pos, vertex_id in enumerate(raw_route):
            expected_non_depot_nodes.append(evrptw.NodeLocation(route_index, pos + 1))
            expected_insertion_points.append(evrptw.NodeLocation(route_index, pos + 1))
        # Begin depot is a valid insertion point
        expected_insertion_points.append(evrptw.NodeLocation(route_index, 0))

    def assert_unordered_list_equal(list1: List, list2: List):
        for i in list1:
            assert i in list2
        assert len(list1) == len(list2)

    # node_locations should be correct
    assert_unordered_list_equal(solution.insertion_points, sorted(expected_insertion_points))
    assert len(solution.insertion_points) == expected_number_of_insertion_points
    # insertion points should be correct
    assert_unordered_list_equal(solution.non_depot_nodes, expected_non_depot_nodes)
    assert len(solution.non_depot_nodes) == expected_number_of_non_depot_nodes


def test_solution_lookup_by_location(adptw_instance: evrptw.Instance, random_solution_factory, mock_evaluation):
    instance = adptw_instance
    solution = random_solution_factory(instance, mock_evaluation, [x.vertex_id for x in instance.customers])
    for route_index, route in enumerate(solution):
        for pos, node in enumerate(route):
            assert solution.lookup(evrptw.NodeLocation(route_index, pos)) is node
