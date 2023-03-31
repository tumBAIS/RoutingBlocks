import routingblocks
from sys import float_info
from evrptw.instance import Instance


class SpatioTemporalRelatedness:
    def __init__(self, py_instance: Instance, cpp_instance: routingblocks.Instance, slack_weight: float,
                 tw_shift_weight: float):
        self._py_instance = py_instance
        self._cpp_id_to_vertex = [self._py_instance.vertices[x.str_id] for x in cpp_instance]
        self._slack_weight = slack_weight
        self._tw_shift_weight = tw_shift_weight

    def __call__(self, i: int, j: int) -> float:
        vertex_i, vertex_j = self._cpp_id_to_vertex[i], self._cpp_id_to_vertex[j]
        t_ij = self._py_instance.arcs[vertex_i.vertex_id, vertex_j.vertex_id].travel_time
        inverse_relatedness = (t_ij \
                               + max(0,
                                     vertex_j.ready_time - vertex_i.service_time - t_ij - vertex_i.ready_time) * self._slack_weight \
                               + max(0,
                                     vertex_i.ready_time + vertex_i.service_time + t_ij - vertex_j.due_time) * self._tw_shift_weight)
        return 1.0 / inverse_relatedness if inverse_relatedness != 0. else float_info.max
