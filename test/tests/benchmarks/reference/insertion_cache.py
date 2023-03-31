from dataclasses import dataclass
from typing import List, Iterator, Iterable, Set
from itertools import islice

import routingblocks
import routingblocks as alns


@dataclass
class InsertionMove:
    vertex_id: int
    after_node: routingblocks.NodeLocation
    delta_cost: float

    def __str__(self):
        return f"InsertionMove(vertex_id={self.vertex_id}, after_node={self.after_node}, cost={self.delta_cost})"


class InsertionCache:
    def __init__(self, instance: routingblocks.Instance):
        self._instance = instance
        self._evaluation: routingblocks.Evaluation = None
        self._best_insertions: List[List[InsertionMove]] = [list() for _ in range(instance.number_of_vertices)]
        self._active_vertices: Set[int] = set()

    def _sort_insertions(self):
        for insertion_points in self._best_insertions:
            insertion_points.sort(key=lambda insertion: insertion.delta_cost)

    def _calculate_best_insertions(self, route: routingblocks.Route, route_index: int,
                                   vertex_ids: Iterable[int]):
        for pos, (pred, succ) in enumerate(zip(route, islice(route, 1, None))):
            for vertex_id in vertex_ids:
                cost = routingblocks.evaluate_insertion(self._evaluation, self._instance, route, pos, vertex_id)
                self._best_insertions[vertex_id].append(
                    InsertionMove(vertex_id, routingblocks.NodeLocation(route_index, pos), cost))

    def clear(self):
        for insertion_points in self._best_insertions:
            insertion_points.clear()
        self._active_vertices.clear()
        self._evaluation = None

    def rebuild(self, evaluation: routingblocks.Evaluation, solution: routingblocks.Solution, vertex_ids: Iterable[int]):
        self.clear()
        self._evaluation = evaluation
        for route_index, route in enumerate(solution):
            self._calculate_best_insertions(route, route_index, vertex_ids)

        self._active_vertices = set(vertex_ids)
        self._sort_insertions()

    def _update_route(self, route: routingblocks.Route, route_index: int):
        self._calculate_best_insertions(route, route_index, self._active_vertices)
        self._sort_insertions()

    def stop_tracking(self, vertex_id: int):
        self._active_vertices.remove(vertex_id)
        self._best_insertions[vertex_id].clear()

    def _invalidate_route(self, route_index: int):
        # Remove all insertion points that are not valid anymore
        for i, insertion_points in enumerate(self._best_insertions):
            self._best_insertions[i][:] = [insertion for insertion in insertion_points if
                                           insertion.after_node.route != route_index]

    def invalidate_route(self, route: routingblocks.Route, route_index: int):
        self._invalidate_route(route_index)
        # Recalculate
        self._update_route(route, route_index)

    def get_best_insertions_for_vertex(self, vertex_id: int) -> Iterator[InsertionMove]:
        return iter(self._best_insertions[vertex_id])

    @property
    def best_moves(self) -> Iterator[InsertionMove]:
        return sorted((x for v_id in self._active_vertices for x in self._best_insertions[v_id]),
                      key=lambda x: x.delta_cost)
