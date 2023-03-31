import routingblocks
from sys import float_info
from evrptw.instance import Instance


class ShawRelatedness:
    def __init__(self, py_instance: Instance, cpp_instance: routingblocks.Instance, distance_weight: float,
                 demand_weight: float,
                 time_weight: float):
        self._py_instance = py_instance
        self._cpp_id_to_vertex = [self._py_instance.vertices[x.str_id] for x in cpp_instance]

        self._distance_weight = distance_weight
        self._demand_weight = demand_weight
        self._time_weight = time_weight

        self._max_distance = max(x.cost for x in self._py_instance.arcs.values())
        self._max_demand = max(x.demand for x in self._py_instance.vertices.values())
        self._min_demand = min(x.demand for x in self._py_instance.vertices.values())
        self._max_earliest_arrival_time = max(x.ready_time for x in self._py_instance.vertices.values())
        self._min_earliest_arrival_time = min(x.ready_time for x in self._py_instance.vertices.values())

    def __call__(self, i: int, j: int) -> float:
        vertex_i, vertex_j = self._cpp_id_to_vertex[i], self._cpp_id_to_vertex[j]
        d_ij = self._py_instance.arcs[vertex_i.vertex_id, vertex_j.vertex_id].cost
        relatedness = (
                self._distance_weight * (d_ij / self._max_distance) +
                self._demand_weight * (abs(vertex_i.demand - vertex_j.demand) / (self._max_demand - self._min_demand)) +
                self._time_weight * (abs(vertex_i.ready_time - vertex_j.ready_time)
                                     / (self._max_earliest_arrival_time - self._min_earliest_arrival_time))
        )
        return 1.0 / relatedness if relatedness != 0. else float_info.max
