from __future__ import annotations
from .models import Vertex, Instance, Arc
from .parsing import parse_instance
from pathlib import Path
from typing import Callable, Any

try:
    import routingblocks as evrptw
except ModuleNotFoundError:
    pass


def create_cpp_vertex(vertex: Vertex, vertex_id: int, data_factory: Callable[[Vertex], Any]) -> evrptw.Vertex:
    return evrptw.create_adptw_vertex(
        vertex_id, vertex.vertex_id, vertex.is_station, vertex.is_depot, data_factory(vertex))


def create_cpp_arc(arc: Arc, data_factory: Callable[[Arc], Any]) -> evrptw.Arc:
    return evrptw.create_adptw_arc(data_factory(arc))


def adptw_vertex_data_factory(vertex: Vertex) -> evrptw.ADPTWVertexData:
    return evrptw.ADPTWVertexData(vertex.x_coord, vertex.y_coord, vertex.demand, vertex.ready_time, vertex.due_time,
                                  vertex.service_time)


def adptw_arc_data_factory(arc: Arc) -> evrptw.ADPTWArcData:
    return evrptw.ADPTWArcData(arc.distance, arc.consumption, arc.travel_time)


def create_cpp_instance(instance: Instance) -> evrptw.Instance:
    vertex_data_factory = adptw_vertex_data_factory
    arc_data_factory = adptw_arc_data_factory

    # Convert vertices
    sorted_vertices = [instance.depot, *sorted(instance.customers, key=lambda v: v.vertex_id),
                       *sorted(instance.stations, key=lambda v: v.vertex_id)]
    cpp_vertices = [create_cpp_vertex(v, i, data_factory=vertex_data_factory) for i, v in enumerate(sorted_vertices)]
    id_map = {cpp_v.id: v for v, cpp_v in zip(sorted_vertices, cpp_vertices)}

    cpp_arcs = [
        [create_cpp_arc(instance.arcs[id_map[i.id].vertex_id, id_map[j.id].vertex_id], data_factory=arc_data_factory)
         for j in cpp_vertices] for i in cpp_vertices]

    return evrptw.Instance(cpp_vertices, cpp_arcs, instance.parameters.fleet_size)


def parse_evrptw_instance(filename: Path) -> evrptw.Instance:
    # Create cpp instance
    return parse_instance(filename)
