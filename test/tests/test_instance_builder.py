import pytest
from typing import Dict, Optional, Tuple
from routingblocks import Vertex, Arc, Instance
from routingblocks.utility import InstanceBuilder


def ilen(iterable):
    return sum(1 for x in iterable)


# Sample vertex and arc factories for testing
def sample_vertex_factory(id: int, str_id: str, is_depot: bool, is_station: bool, data: Dict) -> Vertex:
    return Vertex(id, str_id, is_depot, is_station, data)


def sample_arc_factory(data: Dict) -> Arc:
    return Arc(data)


# Helper function to complete the arc matrix for a successful instance building test
def add_complete_arcs(builder: InstanceBuilder, vertices: Dict[str, Dict], arc_data: Dict):
    for v1 in vertices:
        for v2 in vertices:
            builder.add_arc(v1, v2, arc_data)


# Revised test for successful instance building
def test_successful_instance_building():
    builder = InstanceBuilder(sample_vertex_factory, sample_arc_factory)

    vertices_data = {
        "depot": {"capacity": 100},
        "customer1": {"demand": 20},
        "station1": {"capacity": 50}
    }

    builder.set_depot("depot", vertices_data["depot"])
    builder.add_customer("customer1", vertices_data["customer1"])
    builder.add_station("station1", vertices_data["station1"])

    arc_data = {"distance": 10}
    add_complete_arcs(builder, vertices_data, arc_data)

    instance = builder.build()
    assert isinstance(instance, Instance)
    assert instance.depot.data == vertices_data["depot"]
    assert ilen(instance.customers) == 1
    assert next(instance.customers).data == vertices_data["customer1"]
    assert ilen(instance.stations) == 1
    assert next(instance.stations).data == vertices_data["station1"]

    for i in instance:
        for j in instance:
            assert instance.get_arc(i.vertex_id, j.vertex_id).data == arc_data


# Parameterized test for missing arcs
@pytest.mark.parametrize("arcs_to_add", [
    0,  # No arcs added
    4  # Too few arcs added
])
def test_building_with_missing_arcs(arcs_to_add: int):
    builder = InstanceBuilder(sample_vertex_factory, sample_arc_factory)
    builder.set_depot("depot", {"capacity": 100})
    builder.add_customer("customer1", {"demand": 20})

    arc_data = {"distance": 10}
    vertices = ["depot", "customer1"]

    # Adding some (or none) of the required arcs
    for i in range(arcs_to_add):
        src = vertices[i % 2]
        dst = vertices[(i + 1) % 2]
        builder.add_arc(src, dst, arc_data)

    with pytest.raises(ValueError, match="Instance requires arcs between all vertices"):
        builder.build()


# Test case for instance building without a depot
def test_building_without_depot():
    builder = InstanceBuilder(sample_vertex_factory, sample_arc_factory)
    builder.add_customer("customer1", {"demand": 20})

    with pytest.raises(ValueError, match="Instance requires depot"):
        builder.build()


# Test case for instance building without customers
def test_building_without_customers():
    builder = InstanceBuilder(sample_vertex_factory, sample_arc_factory)
    builder.set_depot("depot", {"capacity": 100})

    with pytest.raises(ValueError, match="Instance requires at least one customer"):
        builder.build()


# Test case for resetting the builder
def test_resetting_builder():
    builder = InstanceBuilder(sample_vertex_factory, sample_arc_factory)
    builder.set_depot("depot", {"capacity": 100})
    builder.add_customer("customer1", {"demand": 20})
    builder.reset()

    with pytest.raises(ValueError, match="Instance requires depot"):
        builder.build()


# Test adding both depot and station to a vertex (should raise an assertion error)
def test_adding_both_depot_and_station():
    builder = InstanceBuilder(sample_vertex_factory, sample_arc_factory)

    with pytest.raises(AssertionError, match="Vertex cannot be both depot and station"):
        builder._add_vertex("depot_station", True, True, {"capacity": 100})


# Test adding multiple depots (should raise a value error)
def test_adding_multiple_depots():
    builder = InstanceBuilder(sample_vertex_factory, sample_arc_factory)
    builder.set_depot("depot1", {"capacity": 100})

    with pytest.raises(ValueError, match="Instance already has a depot"):
        builder.set_depot("depot2", {"capacity": 150})
