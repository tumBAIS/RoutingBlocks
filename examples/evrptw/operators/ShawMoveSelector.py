from typing import Iterable

import routingblocks
from routingblocks.operators.related_removal import RelatedVertexRemovalMove
from evrptw.instance import Instance


class ShawMoveSelector:
    def __init__(self, instance: Instance, cpp_instance: routingblocks.Instance, randgen: routingblocks.Random, shaw_exponent: float):
        self._instance = instance
        self._cpp_id_to_vertex = [self._instance.vertices[x.str_id] for x in cpp_instance]
        self._shaw_exponent = shaw_exponent
        self._randgen = randgen

    def __call__(self, move_iter: Iterable[RelatedVertexRemovalMove]) -> RelatedVertexRemovalMove:
        # Ignore stations
        non_station_nodes = [x for x in move_iter if not self._cpp_id_to_vertex[x.vertex_id].is_station]
        # Pick nth
        pos = int(len(non_station_nodes) * (self._randgen.uniform(0., 1.) ** self._shaw_exponent))
        return non_station_nodes[pos]
