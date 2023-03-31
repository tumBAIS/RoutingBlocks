from itertools import islice
from typing import Iterable, List

import vrpis
from dataclasses import dataclass


@dataclass
class RemovalMove:
    vertex_id: int
    node_location: vrpis.NodeLocation
    delta_cost: float


class RemovalCache:
    def __init__(self, instance: vrpis.Instance):
        self._instance = instance
        self._worst_removals: List[RemovalMove] = []
        self._evaluation = None

    def _add_moves_of_route(self, route: vrpis.Route, route_index: int):
        for pred, (removed, pos), succ in zip(route, zip(islice(route, 1, None), range(1, len(route) - 1)),
                                              islice(route, 2, None)):
            cost = vrpis.evaluate_splice(self._evaluation, self._instance, route, pos - 1, pos + 1)
            delta_cost = cost - route.cost
            self._worst_removals.append(
                RemovalMove(removed.vertex_id, vrpis.NodeLocation(route_index, pos), delta_cost))

    def _remove_moves_of_route(self, route_index: int):
        self._worst_removals[:] = [move for move in self._worst_removals if
                                   move.node_location.route != route_index]

    def clear(self):
        self._worst_removals.clear()
        self._evaluation = None

    def rebuild(self, evaluation: vrpis.Evaluation, solution: vrpis.Solution):
        self.clear()
        self._evaluation = evaluation
        for route_index, route in enumerate(solution):
            self._add_moves_of_route(route, route_index)
        self._worst_removals.sort(key=lambda move: move.delta_cost)

    @property
    def moves_in_order(self) -> Iterable[RemovalMove]:
        return self._worst_removals

    def invalidate_route(self, route: vrpis.Route, route_index: int):
        self._remove_moves_of_route(route_index)
        self._add_moves_of_route(route, route_index)
        self._worst_removals.sort(key=lambda move: move.delta_cost)
