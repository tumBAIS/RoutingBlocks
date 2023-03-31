from __future__ import annotations

from copy import copy
from itertools import islice

from fixtures import *
import pytest

import vrpis
from vrpis import niftw


def iter_vertices(instance: vrpis.Instance):
    for i in range(instance.number_of_vertices):
        yield instance.get_vertex(i)


def assert_cache_equal(cache1: vrpis.RemovalCache, cache2: vrpis.RemovalCache):
    assert list(cache1.moves_in_order) == list(cache2.moves_in_order)


def build_solution(evaluation: vrpis.Evaluation, instance: vrpis.Instance, raw_routes: List[List[int]]):
    return vrpis.Solution(evaluation, instance,
                          [vrpis.create_route(evaluation, instance, route) for route in raw_routes])


@pytest.mark.parametrize("raw_routes,expected_number_of_moves", [
    ([[1, 6, 3], [8, 2, 7]], 6),
    ([[], [8, 2, 7]], 3),
])
def test_removal_cache_build(instance, raw_routes, expected_number_of_moves):
    py_instance, instance = instance
    evaluation = niftw.Evaluation(py_instance.parameters.battery_capacity_time, py_instance.parameters.capacity,
                                  0.0)
    cache = vrpis.RemovalCache(instance)

    solution: vrpis.Solution = build_solution(evaluation, instance, raw_routes)

    cache.rebuild(evaluation, solution)

    # Should contain all moves
    assert sum(1 for _ in cache.moves_in_order) == expected_number_of_moves
    # Moves should be sorted
    assert all(
        pred.delta_cost < succ.delta_cost for pred, succ in
        zip(cache.moves_in_order, islice(cache.moves_in_order, 1, None)))
    # Cost should be correct
    for move in cache.moves_in_order:
        tmp_sol = copy(solution)
        tmp_sol.remove_vertex(move.node_location)
        delta_removal_cost = tmp_sol.cost - solution.cost
        assert pytest.approx(move.delta_cost, abs=1e4) == delta_removal_cost


@pytest.mark.parametrize("raw_routes", [
    [[1, 6, 3], [8, 2, 7]],
    [[], [8, 2, 7]],
])
def test_removal_cache_update(instance, raw_routes):
    py_instance, instance = instance
    evaluation = niftw.Evaluation(py_instance.parameters.battery_capacity_time, py_instance.parameters.capacity,
                                  0.0)
    updated_cache = vrpis.RemovalCache(instance)
    expected_cache = vrpis.RemovalCache(instance)

    solution = build_solution(evaluation, instance, raw_routes)

    updated_cache.rebuild(evaluation, solution)
    expected_cache.rebuild(evaluation, solution)

    # Invalidating an unchanged route does not modify the cache
    updated_cache.invalidate_route(solution[0], 0)
    assert_cache_equal(updated_cache, expected_cache)

    # Cache properly updates invalidated routes
    vertices = [x for route in raw_routes for x in route]
    while len(vertices) > 0:
        vertex_id = vertices.pop()
        location: vrpis.NodeLocation = solution.find(vertex_id)[0]
        solution.remove_vertex(location)
        updated_cache.invalidate_route(solution[location.route], location.route)

        expected_cache.rebuild(evaluation, solution)

        assert_cache_equal(expected_cache, updated_cache)
