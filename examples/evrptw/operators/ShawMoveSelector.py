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
