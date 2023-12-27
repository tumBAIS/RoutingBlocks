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

from itertools import islice
from typing import Iterable, List

import routingblocks
from dataclasses import dataclass


@dataclass
class RemovalMove:
    vertex_id: int
    node_location: routingblocks.NodeLocation
    delta_cost: float


class RemovalCache:
    def __init__(self, instance: routingblocks.Instance):
        self._instance = instance
        self._worst_removals: List[RemovalMove] = []
        self._evaluation = None

    def _add_moves_of_route(self, route: routingblocks.Route, route_index: int):
        for pred, (removed, pos), succ in zip(route, zip(islice(route, 1, None), range(1, len(route) - 1)),
                                              islice(route, 2, None)):
            cost = routingblocks.evaluate_splice(self._evaluation, self._instance, route, pos - 1, pos + 1)
            delta_cost = cost - route.cost
            self._worst_removals.append(
                RemovalMove(removed.vertex_id, routingblocks.NodeLocation(route_index, pos), delta_cost))

    def _remove_moves_of_route(self, route_index: int):
        self._worst_removals[:] = [move for move in self._worst_removals if
                                   move.node_location.route != route_index]

    def clear(self):
        self._worst_removals.clear()
        self._evaluation = None

    def rebuild(self, evaluation: routingblocks.Evaluation, solution: routingblocks.Solution):
        self.clear()
        self._evaluation = evaluation
        for route_index, route in enumerate(solution):
            self._add_moves_of_route(route, route_index)
        self._worst_removals.sort(key=lambda move: move.delta_cost)

    @property
    def moves_in_order(self) -> Iterable[RemovalMove]:
        return self._worst_removals

    def invalidate_route(self, route: routingblocks.Route, route_index: int):
        self._remove_moves_of_route(route_index)
        self._add_moves_of_route(route, route_index)
        self._worst_removals.sort(key=lambda move: move.delta_cost)
