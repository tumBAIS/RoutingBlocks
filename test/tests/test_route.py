from __future__ import annotations

from copy import deepcopy, copy
from typing import Tuple

import pytest

from fixtures import *

try:
    import routingblocks as evrptw
except ModuleNotFoundError:
    pass


def pick_segment(sequence) -> Tuple[int, int]:
    start = random.randint(1, len(sequence) - 2)
    end = random.randint(start + 1, len(sequence) - 1)
    return start, end


def evaluate_forward_sequence(evaluation, instance: evrptw.Instance, nodes: List[evrptw.Node]):
    label = nodes[0].forward_label
    for pred_node, node in zip(nodes, nodes[1:]):
        arc = instance.get_arc(pred_node.vertex_id, node.vertex_id)
        label = evaluation.propagate_forward(label, instance.get_vertex(pred_node.vertex_id),
                                             instance.get_vertex(node.vertex_id), arc)
        yield label


def evaluate_backward_sequence(evaluation, instance: evrptw.Instance, nodes: List[evrptw.Node]):
    label = nodes[-1].backward_label
    for node, succ_node in zip(reversed(nodes[:-1]), reversed(nodes)):
        arc = instance.get_arc(node.vertex_id, succ_node.vertex_id)
        label = evaluation.propagate_backward(label, instance.get_vertex(succ_node.vertex_id),
                                              instance.get_vertex(node.vertex_id), arc)
        yield label


def assert_updated(evaluation, instance: evrptw.Instance, route: evrptw.Route, uid=None):
    # Forward sequence matches
    nodes = list(route)
    assert nodes[0].forward_label == evaluation.create_forward_label(instance.depot)
    for actual_label, expected_label in zip((x.forward_label for x in nodes[1:]),
                                            evaluate_forward_sequence(evaluation=evaluation, instance=instance,
                                                                      nodes=nodes)):
        assert actual_label == expected_label
    expected_depot_label = expected_label

    # Backward sequence matches
    assert nodes[-1].backward_label == evaluation.create_backward_label(instance.depot)
    for actual_label, expected_label in zip(reversed([x.backward_label for x in nodes[:-1]]),
                                            evaluate_backward_sequence(evaluation=evaluation, instance=instance,
                                                                       nodes=nodes)):
        assert actual_label == expected_label

    # Cost correctly cached
    assert route.cost == evaluation.compute_cost(expected_depot_label)
    # Check that the uid is updated
    if uid is not None:
        assert route.modification_timestamp != uid


def test_route_empty_construction(adptw_instance: evrptw.Instance, mock_evaluation):
    instance = adptw_instance
    route = evrptw.Route(mock_evaluation, instance)
    assert route.empty
    assert len(route) == 2
    assert [x.vertex_id for x in route] == [instance.depot.id, instance.depot.id]
    assert route.modification_timestamp == 0
    assert_updated(mock_evaluation, instance, route)


def test_route_get_item(adptw_instance: evrptw.Instance, mock_evaluation):
    instance = adptw_instance
    vertex_ids = [x.id for x in instance.customers]
    route = evrptw.create_route(mock_evaluation, instance, vertex_ids)

    for i, node in enumerate(route):
        # Ensure that get_item does not copy
        assert id(route[i]) == id(route[i])
        assert id(route[i]) == id(node)


def test_route_depot_property(mock_evaluation, adptw_instance):
    instance = adptw_instance
    route = evrptw.Route(mock_evaluation, instance)
    assert id(route.depot) == id(route[0])
    assert id(route.end_depot) == id(route[1])


def test_route_construction_from_route(adptw_instance: evrptw.Instance,
                                       mock_evaluation):
    instance = adptw_instance
    vertex_ids = [x.id for x in instance.customers] + [x.id for x in instance.stations]
    random.shuffle(vertex_ids)

    route = evrptw.create_route(mock_evaluation, instance, vertex_ids)

    assert not route.empty

    expected_vertex_ids = [instance.depot.id, *vertex_ids, instance.depot.id]
    assert len(route) == len(expected_vertex_ids)
    assert [x.vertex_id for x in route] == expected_vertex_ids
    # Is updated?
    assert_updated(mock_evaluation, instance, route)


def test_route_iterator(random_route):
    *_, route = random_route
    for i, node in enumerate(route):
        # Should not copy
        assert id(route[i]) == id(node)


def create_segments(sequence):
    segments = []
    unpicked_vertices = set(sequence)
    while len(unpicked_vertices) > 0:
        segment = random.sample(unpicked_vertices, random.randint(1, len(unpicked_vertices)))
        segments.append(segment)
        unpicked_vertices -= set(segment)
    return segments


def test_route_removal(mock_evaluation, adptw_instance):
    instance: evrptw.Instance = adptw_instance
    vertex_ids = [x.id for x in instance.customers] + [x.id for x in instance.stations]
    random.shuffle(vertex_ids)

    route = evrptw.create_route(mock_evaluation, instance, vertex_ids)

    expected_vertices = [instance.depot.id, *vertex_ids, instance.depot.id]
    while len(expected_vertices) > 2:
        # Choose a subrange
        start, end = pick_segment(route)

        prev_modification_timestamp = route.modification_timestamp
        route.remove_segment(start, end)

        expected_vertices = [*expected_vertices[:start], *expected_vertices[end:]]
        assert [x.vertex_id for x in route] == expected_vertices
        assert_updated(mock_evaluation, instance, route, uid=prev_modification_timestamp)


def assert_exchange_works(evaluation: evrptw.Evaluation, instance: evrptw.Instance, route_a, route_b, segment_a,
                          segment_b):
    # Add the depot
    route_a_vertices = [x.vertex_id for x in route_a]
    route_b_vertices = [x.vertex_id for x in route_b]

    prev_route_a_modification_timestamp = route_a.modification_timestamp
    prev_route_b_modification_timestamp = route_b.modification_timestamp
    # Exchange a random segment
    route_a.exchange_segments(*segment_a, *segment_b, route_b)

    expected_route_a_vertices = [*route_a_vertices[:segment_a[0]], *route_b_vertices[segment_b[0]:segment_b[1]],
                                 *route_a_vertices[segment_a[1]:]]
    expected_route_b_vertices = [*route_b_vertices[:segment_b[0]], *route_a_vertices[segment_a[0]:segment_a[1]],
                                 *route_b_vertices[segment_b[1]:]]

    assert [x.vertex_id for x in route_a] == expected_route_a_vertices
    assert [x.vertex_id for x in route_b] == expected_route_b_vertices
    assert_updated(evaluation, instance, route_a, uid=prev_route_a_modification_timestamp)
    assert_updated(evaluation, instance, route_b, uid=prev_route_b_modification_timestamp)


def test_route_exchange_segment(mock_evaluation, adptw_instance: evrptw.Instance):
    instance: evrptw.Instance = adptw_instance
    vertex_ids = [x.id for x in instance.customers] + [x.id for x in instance.stations]
    random.shuffle(vertex_ids)

    route_a_vertices = vertex_ids[:len(vertex_ids) // 2]
    route_b_vertices = vertex_ids[len(vertex_ids) // 2:]

    create_routes = lambda: (evrptw.create_route(mock_evaluation, instance, route_a_vertices),
                             evrptw.create_route(mock_evaluation, instance, route_b_vertices))

    route_a, route_b = create_routes()

    segment_a = pick_segment(route_a)
    segment_b = pick_segment(route_b)

    assert_exchange_works(evaluation=mock_evaluation, instance=instance,
                          route_a=route_a, route_b=route_b, segment_a=segment_a, segment_b=segment_b)

    route_a, route_b = create_routes()
    assert_exchange_works(evaluation=mock_evaluation, instance=instance,
                          route_a=route_b, route_b=route_a, segment_a=segment_b, segment_b=segment_a)

    route_a, route_b = create_routes()
    segment_a = 1, 2
    assert_exchange_works(evaluation=mock_evaluation, instance=instance,
                          route_a=route_b, route_b=route_a, segment_b=segment_b, segment_a=segment_a)


def assert_intraroute_exchange(evaluation: evrptw.Evaluation, instance: evrptw.Instance, route: evrptw.Route, segment,
                               other_segment):
    vertex_ids = [x.vertex_id for x in route]
    if segment[0] < other_segment[0]:
        expected_vertex_ids = vertex_ids[0:segment[0]] \
                              + vertex_ids[other_segment[0]:other_segment[1]] \
                              + vertex_ids[segment[1]:other_segment[0]] \
                              + vertex_ids[segment[0]:segment[1]] \
                              + vertex_ids[other_segment[1]:]
    else:
        expected_vertex_ids = vertex_ids[0:other_segment[0]] \
                              + vertex_ids[segment[0]:segment[1]] \
                              + vertex_ids[other_segment[1]:segment[0]] \
                              + vertex_ids[other_segment[0]:other_segment[1]] \
                              + vertex_ids[segment[1]:]

    prev_modification_timestamp = route.modification_timestamp
    route.exchange_segments(*segment, *other_segment, route)
    actual_vertices = [x.vertex_id for x in route]

    assert actual_vertices == expected_vertex_ids
    assert_updated(evaluation, instance, route, uid=prev_modification_timestamp)


def test_route_intraroute_exchange_segment(mock_evaluation, adptw_instance: evrptw.Instance):
    instance: evrptw.Instance = adptw_instance
    vertex_ids = [x.id for x in instance.customers] + [x.id for x in instance.stations]
    random.shuffle(vertex_ids)

    def assert_correct(segment, other_segment):
        route = evrptw.create_route(mock_evaluation, instance, vertex_ids)
        assert_intraroute_exchange(mock_evaluation, instance, route, segment, other_segment)

    ########### Moving segments ####################
    # Moving a segment forwards
    segment = 1, 4
    other_segment = 5, 5 + 1
    assert_correct(segment, other_segment)
    # Moving a segment backwards
    segment = 5, 7
    other_segment = 2, 2 + 1
    assert_correct(segment, other_segment)
    ############ Exchanging segment ##################
    # Exchanging forwards, segment with higher position is larger
    segment = 2, 4
    other_segment = 5, 8
    assert_correct(segment, other_segment)
    # Exchanging backwards, segment with higher position is larger
    assert_correct(other_segment, segment)
    # Exchanging forwards, segment with higher position is smaller
    segment = 1, 4
    other_segment = 5, 7
    assert_correct(segment, other_segment)
    # Exchanging backwards, segment with higher position is smaller
    assert_correct(other_segment, segment)


def test_route_copy(random_route):
    *_, route = random_route
    ref_route = route
    assert id(ref_route) == id(route)
    copy_route = copy(route)
    assert id(copy_route) != id(route)
    assert copy_route == route
    assert copy_route.modification_timestamp == route.modification_timestamp

    deepcopy_route = deepcopy(route)
    assert id(deepcopy_route) != id(route)
    assert deepcopy_route == route
    assert deepcopy_route.modification_timestamp == route.modification_timestamp


def test_route_equality(mock_evaluation, adptw_instance: evrptw.Instance):
    instance: evrptw.Instance = adptw_instance
    vertex_ids = [x.id for x in instance.customers] + [x.id for x in instance.stations]
    random.shuffle(vertex_ids)

    # Empty routes are equal
    route_a = evrptw.Route(mock_evaluation, instance)
    route_b = evrptw.Route(mock_evaluation, instance)
    assert route_a == route_b

    route_a = evrptw.create_route(mock_evaluation, instance, vertex_ids)
    assert route_a != route_b

    route_b = evrptw.create_route(mock_evaluation, instance, vertex_ids)
    assert route_a == route_b

    while not route_a.empty:
        position = random.randint(1, len(route_a) - 2)
        route_a.remove_segment(position, position + 1)
        assert route_a != route_b
        route_b.remove_segment(position, position + 1)
        assert route_a == route_b


@pytest.mark.parametrize("sorted", [True, False])
def test_route_remove_vertices(mock_evaluation, sorted: bool, adptw_instance: evrptw.Instance):
    instance: evrptw.Instance = adptw_instance
    customers = list(instance.customers)
    route = evrptw.create_route(mock_evaluation, instance, [x.id for x in customers])
    # [0, 1, 2, 3, 4, 5, 0]
    positions = [evrptw.NodeLocation(0, pos) for pos in range(1, len(route) - 1, 2)]
    if not sorted:
        positions.reverse()

    prev_modification_timestamp = route.modification_timestamp
    route.remove_vertices(positions)
    assert [0, 2, 4, 0] == [x.vertex_id for x in route]
    assert_updated(mock_evaluation, instance, route, uid=prev_modification_timestamp)


@pytest.mark.parametrize("sorted_locations", [True, False])
def test_route_insert_vertices(mock_evaluation, sorted_locations: bool, adptw_instance: evrptw.Instance):
    instance: evrptw.Instance = adptw_instance
    customers = list(instance.customers)
    route = evrptw.create_route(mock_evaluation, instance,
                                [customers[0].id, customers[1].id, customers[2].id])
    # [0, 1, 2, 3, 0]
    to_insert = [customers[3].id, customers[4].id]
    insertion_positions = [evrptw.NodeLocation(0, 0), evrptw.NodeLocation(0, 1)]
    if sorted_locations:
        insertion_positions.reverse()
        to_insert.reverse()

    expected_route = evrptw.create_route(mock_evaluation, instance,
                                         [customers[3].id, customers[0].id, customers[4].id, customers[1].id,
                                          customers[2].id])
    prev_modification_timestamp = route.modification_timestamp
    route.insert_vertices_after(zip(to_insert, insertion_positions))
    assert expected_route == route
    assert_updated(mock_evaluation, instance, route, uid=prev_modification_timestamp)
