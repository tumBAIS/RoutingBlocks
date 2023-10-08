import random

import pytest
from routingblocks import Instance, Vertex, Arc


@pytest.fixture
def depot():
    return Vertex(0, "depot", False, True, {})


@pytest.fixture
def customers():
    return [
        Vertex(1, "customer1", False, False, {"demand": 20}),
        Vertex(2, "customer2", False, False, {"demand": 30})
    ]


@pytest.fixture
def stations():
    return [
        Vertex(3, "station1", True, False, {"capacity": 50}),
        Vertex(4, "station2", True, False, {"capacity": 60})
    ]


def create_basic_instance(depot, customers, stations, arcs=None):
    if arcs is None:
        arcs = create_arcs_matrix(1 + len(customers) + len(stations))
    return Instance(depot, customers, stations, arcs, 2)


def create_arcs_matrix(num_vertices):
    return [[Arc({"distance": int(f'{i}{j}')}) for j in range(num_vertices)] for i in range(num_vertices)]


def test_create_and_verify_instance(depot, customers, stations):
    arcs = create_arcs_matrix(1 + len(customers) + len(stations))
    instance = create_basic_instance(depot, customers, stations, arcs=arcs)

    assert instance.fleet_size == 2
    assert instance.number_of_customers == 2
    assert instance.number_of_stations == 2
    assert instance.number_of_vertices == 5
    assert len(list(instance.vertices)) == 5
    assert len(list(instance.customers)) == 2
    assert len(list(instance.stations)) == 2

    for tests in (
            (instance.customers, customers),
            (instance.stations, stations),
            (instance.vertices, [depot, *customers, *stations]),
            (instance, [depot, *customers, *stations])):
        for expected, actual in zip(*tests):
            assert id(expected) != id(actual)
            assert expected.vertex_id == actual.vertex_id
            assert expected.str_id == actual.str_id
            assert expected.is_depot == actual.is_depot
            assert expected.is_station == actual.is_station
            assert expected.data == actual.data

    for v, arc_row in zip(instance, arcs):
        for u, arc in zip(instance, arc_row):
            assert id(instance.get_arc(v.vertex_id, u.vertex_id)) != id(arc)
            assert instance.get_arc(v.vertex_id, u.vertex_id).data == arc.data


def test_get_vertex_and_arc(depot, customers, stations):
    instance = create_basic_instance(depot, customers, stations)

    assert instance.get_vertex(1).str_id == "customer1"
    assert instance.get_customer(0).str_id == "customer1"
    assert instance.get_station(0).str_id == "station1"
    assert instance.get_arc(0, 1).data['distance'] == 1
    assert instance.get_arc(1, 0).data['distance'] == 10


def test_empty_instance_creation():
    with pytest.raises(RuntimeError):
        Instance([], [])


@pytest.mark.parametrize("vertex_factory", [
    lambda depot, stations, customers: [depot, *stations, *customers],
    lambda depot, stations, customers: [*stations, depot, *customers],
    lambda depot, stations, customers: [*customers, depot, *stations],
    lambda depot, stations, customers: [*customers, *stations, depot],
])
def test_instance_creation_with_wrong_vertex_order(vertex_factory, depot, stations, customers):
    vertices = vertex_factory(depot, stations, customers)
    with pytest.raises(RuntimeError):
        Instance(vertices, create_arcs_matrix(len(vertices)))


def test_overloaded_constructors(depot, customers, stations):
    vertices = [depot, *customers, *stations]
    arcs = create_arcs_matrix(len(vertices))

    instance1 = Instance(vertices, arcs)
    assert instance1.fleet_size == 2  # Default fleet_size should be inferred correctly

    instance2 = Instance(vertices, arcs, 1)
    assert instance2.fleet_size == 1  # Provided fleet_size should be set correctly
