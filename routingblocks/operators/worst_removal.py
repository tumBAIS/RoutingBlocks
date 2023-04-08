import routingblocks
from typing import Iterable, List
from .move_selectors import MoveSelector


class WorstRemovalOperator(routingblocks.DestroyOperator):
    def __init__(self, instance: routingblocks.Instance, move_selector: MoveSelector[routingblocks.RemovalMove]):
        routingblocks.DestroyOperator.__init__(self)
        self._instance = instance
        self._move_cache = routingblocks.RemovalCache(self._instance)
        # Exposed
        self.move_selector = move_selector

    def name(self) -> str:
        return "WorstRemovalOperator"

    def can_apply_to(self, _solution: routingblocks.Solution) -> bool:
        return len(_solution) > 0

    def apply(self, evaluation: routingblocks.Evaluation, _solution: routingblocks.Solution,
              number_of_removed_vertices: int) -> List[
        int]:
        self._move_cache.rebuild(evaluation, _solution)
        removed_vertices = []
        while len(removed_vertices) < number_of_removed_vertices:
            # Choose the next removed vertex
            selected_move = self.move_selector(self._move_cache.moves_in_order)
            # Remove the vertex
            _solution.remove_vertex(selected_move.node_location)
            # Update the cache
            self._move_cache.invalidate_route(_solution[selected_move.node_location.route],
                                              selected_move.node_location.route)
            removed_vertices.append(selected_move.vertex_id)
        return removed_vertices
