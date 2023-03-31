from __future__ import annotations

from copy import copy
from itertools import islice

from fixtures import *
import pytest

try:
    import routingblocks as alns
    from routingblocks import niftw
    from routingblocks import InsertionCache, InsertionMove
except ImportError:
    pass


def iter_vertices(instance: alns.Instance):
    for i in range(instance.number_of_vertices):
        yield instance.get_vertex(i)


def assert_cache_equal(instance: alns.Instance, solution: alns.Solution, cache1: InsertionCache,
                       cache2: InsertionCache):
    def assert_move_equal(move1: InsertionMove, move2: InsertionMove):
        try:
            assert cache1_move.after_node.position == cache2_move.after_node.position
            assert cache1_move.after_node.route == cache2_move.after_node.route
        except AssertionError as e:
            if not (pytest.approx(cache1_move.delta_cost, abs=1e4) == cache2_move.delta_cost and
                    solution[cache1_move.after_node.route].empty and solution[cache2_move.after_node.route].empty):
                raise e

    for vertex in iter_vertices(instance):
        vertex_id = vertex.id
        assert cache1.tracks_vertex(vertex_id) == cache2.tracks_vertex(vertex_id)
        if not cache1.tracks_vertex(vertex_id):
            continue
        for cache1_move, cache2_move in zip(cache1.get_best_insertions_for_vertex(vertex_id),
                                            cache2.get_best_insertions_for_vertex(vertex_id)):
            assert_move_equal(cache1_move, cache2_move)

    for cache1_move, cache2_move in zip(cache1.moves_in_order, cache2.moves_in_order):
        assert_move_equal(cache1_move, cache2_move)


def build_solution(evaluation: alns.Evaluation, instance: alns.Instance, raw_routes: List[List[int]]):
    return alns.Solution(evaluation, instance, [alns.create_route(evaluation, instance, route) for route in raw_routes])


@pytest.mark.parametrize("raw_routes,vertices_to_insert,expected_number_of_locations", [
    ([[1, 6, 3], [8, 2, 7]], [4, 5], 8),  # D - 1, 1 - 6, 6 - 3, 3 - D, D - 8, 8 - 2, 2 - 7, 7 - D
    ([[], [], [1, 6, 3], [8, 2, 7]], [4, 5], 10)  # Insertion into every empty route is tracked
])
def test_insertion_cache(instance, raw_routes, vertices_to_insert, expected_number_of_locations):
    py_instance, instance = instance
    evaluation = niftw.Evaluation(py_instance.parameters.battery_capacity_time, py_instance.parameters.capacity,
                                  0.0)
    cache = InsertionCache(instance)

    solution = build_solution(evaluation, instance, raw_routes)

    cache.rebuild(evaluation, solution, vertices_to_insert)

    # Test that the right number of moves is tracked for each vertex
    for vertex in iter_vertices(instance):
        vertex_id = vertex.id
        if vertex_id in vertices_to_insert:
            assert cache.tracks_vertex(vertex_id)
            assert sum(1 for _ in cache.get_best_insertions_for_vertex(vertex_id)) == expected_number_of_locations
        else:
            assert not cache.tracks_vertex(vertex_id)
    # Ensure that the right vertices are tracked
    assert set(cache.tracked_vertices) == set(vertices_to_insert)
    # Ensure that moves are sorted correctly
    assert all(
        pred.delta_cost <= succ.delta_cost for pred, succ in
        zip(cache.moves_in_order, islice(cache.moves_in_order, 1, None)))
    for vertex in iter_vertices(instance):
        vertex_id = vertex.id
        if vertex_id not in vertices_to_insert:
            continue
        assert all(
            pred.delta_cost <= succ.delta_cost for pred, succ in
            zip(cache.get_best_insertions_for_vertex(vertex_id),
                islice(cache.get_best_insertions_for_vertex(vertex_id), 1, None)))

    # Ensure that the costs are correct
    for vertex in iter_vertices(instance):
        vertex_id = vertex.id
        if vertex_id not in vertices_to_insert:
            continue
        for move in cache.get_best_insertions_for_vertex(vertex_id):
            tmp_solution = copy(solution)
            tmp_solution.insert_vertex_after(move.after_node, vertex_id)
            assert pytest.approx(move.delta_cost, abs=1e4) == tmp_solution.cost - solution.cost


@pytest.mark.parametrize("raw_routes,vertices_to_insert", [
    ([[1, 6, 3], [8, 2, 7]], [4, 5]),
    ([[], [], [1, 6, 3], [8, 2, 7]], [4, 5]),
])
def test_insertion_cache_stop_tracking(instance, raw_routes, vertices_to_insert):
    py_instance, instance = instance
    evaluation = niftw.Evaluation(py_instance.parameters.battery_capacity_time, py_instance.parameters.capacity,
                                  0.0)
    cache = InsertionCache(instance)

    solution = build_solution(evaluation, instance, raw_routes)

    cache.rebuild(evaluation, solution, vertices_to_insert)

    for vertex_id in vertices_to_insert:
        assert cache.tracks_vertex(vertex_id)
        cache.stop_tracking(vertex_id)
        assert not cache.tracks_vertex(vertex_id)


@pytest.mark.parametrize("raw_routes,vertices_to_insert", [
    ([[1, 6, 3], [8, 2, 7]], [4, 5]),
    ([[2, 6, 3], [], [8, 1], []], [4, 5])
])
def test_insertion_cache_update(instance, vertices_to_insert, raw_routes):
    py_instance, instance = instance
    evaluation = niftw.Evaluation(py_instance.parameters.battery_capacity_time, py_instance.parameters.capacity,
                                  0.0)

    expected_cache = InsertionCache(instance)
    cache = InsertionCache(instance)

    solution = build_solution(evaluation, instance, raw_routes)

    cache.rebuild(evaluation, solution, vertices_to_insert)
    expected_cache.rebuild(evaluation, solution, vertices_to_insert)

    # Invalidating an unchanged route should not change the cache
    cache.invalidate_route(solution[0], 0)
    assert_cache_equal(instance, solution, cache, expected_cache)

    # Actually insert the vertex
    while len(vertices_to_insert) > 0:
        vertex_id = vertices_to_insert.pop()
        insertion_point = alns.NodeLocation(0, 1)
        solution.insert_vertex_after(insertion_point, vertex_id)
        cache.stop_tracking(vertex_id)

        cache.invalidate_route(solution[insertion_point.route], insertion_point.route)
        expected_cache.rebuild(evaluation, solution, vertices_to_insert)
        assert_cache_equal(instance=instance, solution=solution, cache1=cache, cache2=expected_cache)
