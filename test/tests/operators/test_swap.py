from __future__ import annotations

from collections import defaultdict
from dataclasses import dataclass
from typing import Tuple, Callable, Dict, List, TypeVar, Union

import pytest

import helpers

from fixtures import *

import routingblocks

try:
    import routingblocks as evrptw
except ModuleNotFoundError:
    pass


def create_solution(evaluation: evrptw.Evaluation, instance: evrptw.Instance, routes: List[evrptw.Route]):
    solution = evrptw.Solution(evaluation, instance,
                               [*routes, *[evrptw.Route(evaluation, instance) for _ in
                                           range(instance.fleet_size - len(routes))]])
    return solution


def recreate_solution(evaluation: evrptw.Evaluation, instance: evrptw.Instance, customers):
    route_a = evrptw.create_route(evaluation, instance, [customers[0].id, customers[1].id])
    route_b = evrptw.create_route(evaluation, instance,
                                  [customers[2].id, customers[3].id, customers[4].id])
    return create_solution(evaluation=evaluation, instance=instance,
                           routes=[route_a, route_b])


def build_solution(evaluation: routingblocks.Evaluation, instance: routingblocks.Instance, raw_routes: List[List[int]]):
    return routingblocks.Solution(evaluation, instance,
                          [routingblocks.create_route(evaluation, instance, route) for route in raw_routes])


def to_propagated_arcs(forward_propagation_sequence: List[routingblocks.Node]) -> List[Tuple[int, int]]:
    return [(i.vertex_id, j.vertex_id) for i, j in zip(forward_propagation_sequence,
                                                       forward_propagation_sequence[1:])]


def _assert_forward_evaluations(first_forward_evaluation, second_forward_evaluation, captured_evaluation_operations):
    expected_forward_propagations = to_propagated_arcs(first_forward_evaluation) + \
                                    to_propagated_arcs(second_forward_evaluation)
    assert_forward_propagations(captured_evaluation_operations, expected_forward_propagations)

    expected_concatenations = [first_forward_evaluation[-1].vertex_id,
                               second_forward_evaluation[-1].vertex_id]
    assert_concatenations(captured_evaluation_operations, expected_concatenations)


def generate_neigborhood(solution: evrptw.Solution, origin_segment_length: int, target_segment_length: int,
                         interroute: bool, intraroute: bool):
    for origin_route_index in range(len(solution)):
        for target_route_index in range(len(solution)):
            if origin_route_index != target_route_index and not interroute:
                continue
            if origin_route_index == target_route_index and not intraroute:
                continue
            for origin_node_index in range(len(solution[origin_route_index]) - origin_segment_length - 1):
                for target_node_index in range(1, len(solution[target_route_index]) - target_segment_length):
                    yield origin_route_index, target_route_index, origin_node_index, target_node_index


@pytest.mark.parametrize("raw_routes", [
    [[1, 7, 2, 6, 6, 3, 8], [6, 4, 7, 8, 5, 8]],
    [[], [1, 7, 2, 6, 6, 3, 8]]
])
@pytest.mark.parametrize("move_type,origin_segment_length,target_segment_length", [
    (routingblocks.SwapOperatorMove_0_1, 0, 1),
    (routingblocks.SwapOperatorMove_0_2, 0, 2),
    (routingblocks.SwapOperatorMove_1_1, 1, 1),
    (routingblocks.SwapOperatorMove_1_2, 1, 2),
    (routingblocks.SwapOperatorMove_2_1, 2, 1),
])
def test_swap_interroute_apply(adptw_instance: evrptw.Instance, mock_evaluation, move_type, origin_segment_length,
                               target_segment_length, raw_routes):
    instance: evrptw.Instance = adptw_instance

    def _recreate_solution():
        return build_solution(mock_evaluation, instance, raw_routes)

    solution_before_move = _recreate_solution()

    for origin_route_index, target_route_index, \
            origin_node_index, target_node_index in generate_neigborhood(solution_before_move,
                                                                         origin_segment_length=origin_segment_length,
                                                                         target_segment_length=target_segment_length,
                                                                         interroute=True, intraroute=False):
        solution_after_move = _recreate_solution()

        swap_operator_move = move_type(
            evrptw.NodeLocation(origin_route_index, origin_node_index),
            evrptw.NodeLocation(target_route_index, target_node_index))
        swap_operator_move.apply(instance, solution_after_move)

        # Should move target_node behind origin_node.
        for route_id, (original_route, actual_route) in enumerate(zip(solution_before_move, solution_after_move)):
            if route_id == origin_route_index:

                moved_segment_vids = [x.vertex_id for x in solution_before_move[target_route_index]][
                                     target_node_index:target_node_index + target_segment_length]

                original_route_vids = [x.vertex_id for x in original_route]

                expected_route = original_route_vids[:origin_node_index + 1] + \
                                 moved_segment_vids + \
                                 original_route_vids[origin_node_index + 1 + origin_segment_length:]
                assert [x.vertex_id for x in actual_route] == expected_route
            elif route_id == target_route_index:
                expected_route = [x.vertex_id for x in original_route]
                moved_segment_vids = [x.vertex_id for x in solution_before_move[origin_route_index]][
                                     origin_node_index + 1:origin_node_index + 1 + origin_segment_length]
                expected_route = expected_route[:target_node_index] + moved_segment_vids + expected_route[
                                                                                           target_node_index + target_segment_length:]
                assert [x.vertex_id for x in actual_route] == expected_route
            else:
                assert original_route == actual_route


@pytest.mark.parametrize("raw_routes", [
    [[1, 5, 2, 6, 6, 3, 8, 6, 4, 7, 6, 5, 7]],
    [[]],
    [[1]]
])
@pytest.mark.parametrize("move_type,origin_segment_length,target_segment_length", [
    (routingblocks.SwapOperatorMove_0_1, 0, 1),
    (routingblocks.SwapOperatorMove_0_2, 0, 2),
    (routingblocks.SwapOperatorMove_1_1, 1, 1),
    (routingblocks.SwapOperatorMove_1_2, 1, 2),
    (routingblocks.SwapOperatorMove_2_1, 2, 1),
])
def test_swap_intraroute_apply(mock_evaluation, adptw_instance: evrptw.Instance, move_type, origin_segment_length,
                               target_segment_length, raw_routes):
    instance: evrptw.Instance = adptw_instance

    def _recreate_solution():
        return build_solution(mock_evaluation, instance, raw_routes)

    before_move_solution = _recreate_solution()

    for route_index, _, origin_node_index, target_node_index \
            in generate_neigborhood(before_move_solution,
                                    origin_segment_length=origin_segment_length,
                                    target_segment_length=target_segment_length,
                                    interroute=False, intraroute=True):
        # Ensure that there is no overlap between the segments
        if not (origin_node_index + 1 + origin_segment_length < target_node_index
                or origin_node_index + origin_segment_length + 1 < target_node_index):
            continue

        after_move_solution = _recreate_solution()

        swap_operator_move = move_type(
            evrptw.NodeLocation(route_index, origin_node_index),
            evrptw.NodeLocation(route_index, target_node_index))
        swap_operator_move.apply(instance, after_move_solution)

        # Should move target_node behind origin_node.
        original_vids = [x.vertex_id for x in before_move_solution[route_index]]
        origin_segment_vids = original_vids[origin_node_index + 1:origin_node_index + 1 + origin_segment_length]
        target_segment_vids = original_vids[target_node_index:target_node_index + target_segment_length]

        if origin_node_index < target_node_index:
            expected_route = original_vids[:origin_node_index + 1] + \
                             target_segment_vids + \
                             original_vids[origin_node_index + 1 + origin_segment_length:target_node_index] + \
                             origin_segment_vids + \
                             original_vids[target_node_index + target_segment_length:]
        else:
            expected_route = original_vids[:target_node_index] + \
                             origin_segment_vids + \
                             original_vids[target_node_index + target_segment_length:origin_node_index + 1] + \
                             target_segment_vids + \
                             original_vids[origin_node_index + 1 + origin_segment_length:]

        actual_route = [x.vertex_id for x in after_move_solution[route_index]]
        assert actual_route == expected_route
        # Other routes remain unchanged
        assert all(
            x == y for i, (x, y) in enumerate(zip(before_move_solution, after_move_solution)) if i != route_index)


@pytest.mark.parametrize("raw_routes", [
    [[1, 7, 2, 6, 6, 3, 8], [6, 4, 7, 8, 5, 8]],
    [[], [1, 7, 2, 6, 6, 3, 8]]
])
@pytest.mark.parametrize("move_type,origin_segment_length,target_segment_length", [
    (routingblocks.SwapOperatorMove_0_1, 0, 1),
    (routingblocks.SwapOperatorMove_0_2, 0, 2),
    (routingblocks.SwapOperatorMove_1_1, 1, 1),
    (routingblocks.SwapOperatorMove_1_2, 1, 2),
    (routingblocks.SwapOperatorMove_2_1, 2, 1),
])
def test_swap_interroute_evaluation(mock_evaluation, adptw_instance, move_type, origin_segment_length,
                                    target_segment_length, raw_routes):
    instance: evrptw.Instance = adptw_instance

    def _recreate_solution():
        return build_solution(mock_evaluation, instance, raw_routes)

    solution_before_move = _recreate_solution()

    for origin_route_index, target_route_index, \
            origin_node_index, target_node_index in generate_neigborhood(solution_before_move,
                                                                         origin_segment_length=origin_segment_length,
                                                                         target_segment_length=target_segment_length,
                                                                         interroute=True, intraroute=False):
        # Skip symmetric moves
        if origin_segment_length == target_segment_length:
            if origin_route_index > target_route_index:
                continue
        # Create the move
        swap_operator_move = move_type(
            evrptw.NodeLocation(origin_route_index, origin_node_index),
            evrptw.NodeLocation(target_route_index, target_node_index))

        # Evaluate the move recording calls to evaluation
        mock_evaluation.reset()
        swap_operator_move.get_cost_delta(mock_evaluation, instance, solution_before_move)
        captured_evaluation_operations = list(mock_evaluation.ops)

        # Apply the move to get the resulting solution for checking
        solution_after_move = _recreate_solution()
        swap_operator_move.apply(instance, solution_after_move)
        origin_route = solution_after_move[origin_route_index]
        target_route = solution_after_move[target_route_index]

        # ---------- Origin
        # Check that the evaluation was called with the correct arguments
        # Expected evaluations: origin_node -> target_node -> ... -> last_target_node -> after_origin_node
        after_origin_node_index = origin_node_index + target_segment_length + 1
        origin_forward_propagation_sequence = [origin_route[origin_node_index]] + \
                                              list(origin_route)[origin_node_index + 1:after_origin_node_index + 1]
        # concatenation at last node of origin_forward_propagation_sequence
        # --------- Target
        # Expected valuations: target_node -> origin_node -> ... -> last_origin_node -> after_target_node
        before_target_node_index = target_node_index - 1
        after_origin_segment_node_index = target_node_index + origin_segment_length
        target_forward_propagation_sequence = [target_route[before_target_node_index]] + \
                                              list(target_route)[
                                              target_node_index:after_origin_segment_node_index + 1]
        # Concatenation at last node of target_forward_propagation_sequence

        # Operator could first evaluate the origin route and then the target route
        try:
            _assert_forward_evaluations(origin_forward_propagation_sequence, target_forward_propagation_sequence,
                                        captured_evaluation_operations)
        except AssertionError:
            # or the other way around
            _assert_forward_evaluations(target_forward_propagation_sequence, origin_forward_propagation_sequence,
                                        captured_evaluation_operations)


@pytest.mark.parametrize("raw_routes", [
    [[1, 2, 3, 4, 5, 6, 7, 8]],
    [[]],
    [[1]]
])
@pytest.mark.parametrize("move_type,origin_segment_length,target_segment_length", [
    (routingblocks.SwapOperatorMove_0_1, 0, 1),
    (routingblocks.SwapOperatorMove_0_2, 0, 2),
    (routingblocks.SwapOperatorMove_1_1, 1, 1),
    (routingblocks.SwapOperatorMove_1_2, 1, 2),
    (routingblocks.SwapOperatorMove_2_1, 2, 1),
])
def test_swap_intraroute_evaluation(mock_evaluation, adptw_instance: evrptw.Instance, move_type, origin_segment_length,
                                    target_segment_length, raw_routes):
    instance: evrptw.Instance = adptw_instance

    assert all(len(set(route)) == len(route) for route in raw_routes), "Test does not work with duplicate vertex ids"

    def _recreate_solution():
        return build_solution(mock_evaluation, instance, raw_routes)

    solution_before_move = _recreate_solution()

    for route_index, _, origin_node_index, target_node_index \
            in generate_neigborhood(solution_before_move,
                                    origin_segment_length=origin_segment_length,
                                    target_segment_length=target_segment_length,
                                    interroute=False, intraroute=True):
        # Ensure that there is no overlap between the segments
        if not (origin_node_index + 1 + origin_segment_length < target_node_index
                or origin_node_index + origin_segment_length + 1 < target_node_index):
            continue

        # Create the move
        swap_operator_move = move_type(
            evrptw.NodeLocation(route_index, origin_node_index),
            evrptw.NodeLocation(route_index, target_node_index))

        # Evaluate the move recording calls to evaluation
        mock_evaluation.reset()
        swap_operator_move.get_cost_delta(mock_evaluation, instance, solution_before_move)
        captured_evaluation_operations = list(mock_evaluation.ops)

        # Apply the move to get the resulting solution for checking
        solution_after_move = _recreate_solution()
        swap_operator_move.apply(instance, solution_after_move)
        route = solution_after_move[route_index]
        original_route = solution_before_move[route_index]

        # Find first point where the routes missmatch.
        for first_change_fwd_index in range(len(route)):
            if route[first_change_fwd_index].vertex_id != original_route[first_change_fwd_index].vertex_id:
                break

        # Find first point where the routes missmatch.
        for first_change_bwd_index in reversed(range(len(route))):
            if route[first_change_bwd_index].vertex_id != original_route[first_change_bwd_index].vertex_id:
                break

        expected_forward_propagation_sequence = list(route)[first_change_fwd_index - 1:first_change_bwd_index + 2]
        assert_forward_propagations(captured_evaluation_operations,
                                    to_propagated_arcs(expected_forward_propagation_sequence))
        assert_concatenations(captured_evaluation_operations, [expected_forward_propagation_sequence[-1].vertex_id])


@dataclass(frozen=True)
class RecordedMove:
    resulting_raw_routes: Tuple[Tuple[int]]
    move_origin: routingblocks.NodeLocation
    move_target: routingblocks.NodeLocation
    evaluation_operations: List

    def __repr__(self):
        return f'{self.move_origin} {self.move_target}: {self.resulting_raw_routes}'


@pytest.mark.parametrize("raw_routes", [
    [[1, 2, 3, 4, 5, 6, 7, 8]],
    [[1, 2, 3, 4, 5, 6, 7, 8], [9, 10, 11, 12, 13, 14]],
    [[], [1, 7, 2, 6, 3, 8]]
])
@pytest.mark.parametrize("move_type,origin_segment_length,target_segment_length", [
    (routingblocks.SwapOperatorMove_0_1, 0, 1),
    (routingblocks.SwapOperatorMove_0_2, 0, 2),
    (routingblocks.SwapOperatorMove_1_1, 1, 1),
    (routingblocks.SwapOperatorMove_1_2, 1, 2),
    (routingblocks.SwapOperatorMove_2_1, 2, 1),
])
def test_swap_symmetry(mock_evaluation, large_adptw_instance: evrptw.Instance, move_type, origin_segment_length,
                       target_segment_length, raw_routes):
    instance: evrptw.Instance = large_adptw_instance

    assert len([x for route in raw_routes for x in route]) == len(
        set([x for route in raw_routes for x in route])), "Test does not work with duplicate vertex ids"

    def _recreate_solution():
        return build_solution(mock_evaluation, instance, raw_routes)

    symmetric_moves = defaultdict(list)

    # Collect all moves
    for origin_route_index, target_route_index, origin_node_index, target_node_index \
            in generate_neigborhood(_recreate_solution(),
                                    origin_segment_length=origin_segment_length,
                                    target_segment_length=target_segment_length,
                                    interroute=True, intraroute=True):
        if target_route_index == origin_route_index:
            # Ensure that there is no overlap between the segments
            if not (origin_node_index + 1 + origin_segment_length < target_node_index
                    or origin_node_index + origin_segment_length + 1 < target_node_index):
                continue
        # Start with a clean solution
        solution = _recreate_solution()

        # Create the move
        swap_operator_move = move_type(
            evrptw.NodeLocation(origin_route_index, origin_node_index),
            evrptw.NodeLocation(target_route_index, target_node_index))

        # Evaluate it
        mock_evaluation.reset()
        swap_operator_move.get_cost_delta(mock_evaluation, instance, solution)
        evaluation_operations = list(mock_evaluation.ops)
        # Apply it
        swap_operator_move.apply(instance, solution)

        # Store the move with the resulting solution
        raw_solution = tuple(tuple(x.vertex_strid for x in route) for route in solution)
        recorded_move = RecordedMove(resulting_raw_routes=raw_solution,
                                     move_origin=evrptw.NodeLocation(origin_route_index, origin_node_index),
                                     move_target=evrptw.NodeLocation(target_route_index, target_node_index),
                                     evaluation_operations=evaluation_operations)
        symmetric_moves[raw_solution].append(recorded_move)

    for recorded_moves in symmetric_moves.values():
        if sum(1 for x in recorded_moves if len(x.evaluation_operations) > 0) != 1:
            msg = f"Detected two evaluations that resulted in same solution {recorded_moves[0].resulting_raw_routes}!\n" \
                  f"Generator arcs:"
            for move in recorded_moves:
                msg += f"\n\t({move.move_origin}, {move.move_target})"
            msg += "\nOriginal solution:\n" + "\n".join(map(str, list(map(str, _recreate_solution().routes))))
            pytest.fail(msg)
