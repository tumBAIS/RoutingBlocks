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
    return evrptw.adptw.create_adptw_vertex(
        vertex_id, vertex.vertex_id, vertex.is_station, vertex.is_depot, data_factory(vertex))


def create_cpp_arc(arc: Arc, data_factory: Callable[[Arc], Any]) -> evrptw.Arc:
    return evrptw.adptw.create_adptw_arc(data_factory(arc))


def adptw_vertex_data_factory(vertex: Vertex) -> evrptw.adptw.VertexData:
    return evrptw.adptw.VertexData(vertex.x_coord, vertex.y_coord, vertex.demand, vertex.ready_time,
                                   vertex.due_date,
                                   vertex.service_time)


def adptw_arc_data_factory(arc: Arc) -> evrptw.adptw.ArcData:
    return evrptw.adptw.ArcData(arc.distance, arc.consumption, arc.travel_time)


def create_cpp_instance(instance: Instance) -> evrptw.Instance:
    vertex_data_factory = adptw_vertex_data_factory
    arc_data_factory = adptw_arc_data_factory

    # Convert vertices
    sorted_vertices = [instance.depot, *sorted(instance.customers, key=lambda v: v.vertex_id),
                       *sorted(instance.stations, key=lambda v: v.vertex_id)]
    cpp_vertices = [create_cpp_vertex(v, i, data_factory=vertex_data_factory) for i, v in enumerate(sorted_vertices)]
    id_map = {cpp_v.vertex_id: v for v, cpp_v in zip(sorted_vertices, cpp_vertices)}

    cpp_arcs = [
        [create_cpp_arc(instance.arcs[id_map[i.vertex_id].vertex_id, id_map[j.vertex_id].vertex_id],
                        data_factory=arc_data_factory)
         for j in cpp_vertices] for i in cpp_vertices]

    return evrptw.Instance(cpp_vertices, cpp_arcs, instance.parameters.fleet_size)


def parse_evrptw_instance(filename: Path) -> evrptw.Instance:
    # Create cpp instance
    return parse_instance(filename)
