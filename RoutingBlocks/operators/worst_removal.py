import vrpis
from typing import Iterable, List
from . import MoveSelector
from vrpis import RemovalCache, RemovalMove


class WorstRemovalOperator(vrpis.DestroyOperator):
    def __init__(self, instance: vrpis.Instance, move_selector: MoveSelector[RemovalMove]):
        vrpis.DestroyOperator.__init__(self)
        self._instance = instance
        self._move_cache = RemovalCache(self._instance)
        # Exposed
        self.move_selector = move_selector

    def name(self) -> str:
        return "WorstRemovalOperator"

    def can_apply_to(self, _solution: vrpis.Solution) -> bool:
        return len(_solution) > 0

    def apply(self, evaluation: vrpis.Evaluation, _solution: vrpis.Solution, number_of_removed_vertices: int) -> List[
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
