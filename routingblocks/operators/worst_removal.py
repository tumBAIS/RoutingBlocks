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

import routingblocks
from typing import Iterable, List
from .move_selectors import MoveSelector


class WorstRemovalOperator(routingblocks.DestroyOperator):
    """
    Iteratively (one at a time) removes vertices according to the benefit yielded by removing them, i.e., the change in
    the solution's cost with and without the vertex.

    The operator uses a :class:`routingblocks.operators.MoveSelector[routingblocks.RemovalMove]` to choose the
    next vertex to remove. This selector receives as argument a list of :class:`routingblocks.RemovalMove` objects,
    each one representing a possible removal of a vertex from the solution, ordered by cost improvement.

    This allows to customize the operator to different removal strategies, such as removing the vertex with the
    highest cost improvement, removing the vertex with the lowest cost improvement, or introducing randomness in the
    selection process.
    """

    def __init__(self, instance: routingblocks.Instance, move_selector: MoveSelector[routingblocks.RemovalMove]):
        """
        :param instance: The problem instance
        :param routingblocks.operators.MoveSelector[routingblocks.RemovalMove] move_selector: The move selector used to choose the next vertex to remove
        """
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
