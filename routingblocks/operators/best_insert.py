from typing import List, Iterable, Callable, TypeVar

import routingblocks

from .move_selectors import MoveSelector


class BestInsertionOperator(routingblocks.RepairOperator):
    def __init__(self, instance: routingblocks.Instance, move_selector: MoveSelector[routingblocks.InsertionMove]):
        routingblocks.RepairOperator.__init__(self)
        self._instance = instance
        self._move_cache = routingblocks.InsertionCache(self._instance)
        # Exposed
        self.move_selector = move_selector

    def apply(self, evaluation: routingblocks.Evaluation, solution: routingblocks.Solution,
              vertex_ids: Iterable[int]) -> None:
        vertex_ids = [x for x in vertex_ids if not self._instance.get_vertex(x).is_station]
        self._move_cache.rebuild(evaluation, solution, vertex_ids)
        for vertex_id in vertex_ids:
            best_insertion = self.move_selector(self._move_cache.get_best_insertions_for_vertex(vertex_id))
            best_route = best_insertion.after_node.route
            # Stop tracking
            self._move_cache.stop_tracking(vertex_id)
            # Insert the vertex
            solution.insert_vertex_after(best_insertion.after_node, vertex_id)
            # Update the cache
            self._move_cache.invalidate_route(solution[best_route],
                                              best_route)

    def name(self) -> str:
        return "BestInsertionOperator"

    def can_apply_to(self, solution: routingblocks.Solution) -> bool:
        return True
