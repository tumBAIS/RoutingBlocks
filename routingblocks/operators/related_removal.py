from __future__ import annotations
from typing import List, Callable, Set, Tuple

from .move_selectors import MoveSelector
from dataclasses import dataclass

import routingblocks


@dataclass(frozen=True)
class RelatedVertexRemovalMove:
    """
    :ivar vertex_id: The id of the corresponding vertex.
    :ivar relatedness: The relatedness of the vertex to the seed vertex.
    :ivar location: The location of the vertex in the solution.
    """
    vertex_id: int
    relatedness: float
    location: routingblocks.NodeLocation

    def __hash__(self):
        return hash((self.vertex_id, self.location.route, self.location.position))

    def __eq__(self, other):
        return self.vertex_id == other.vertex_id and \
            self.location.route == other.location.route and \
            self.location.position == other.location.position


def build_relatedness_matrix(instance: routingblocks.Instance, relatedness_computer: Callable[[int, int], float]) -> \
        List[
            List[float]]:
    """
    Builds a relatedness matrix for the given instance and relatedness function.

    :param instance: The instance.
    :param relatedness_computer: A function that computes the relatedness between two vertices. Takes as input the ids of the two vertices and returns a number that measures the degree of relatedness.
    :return: A matrix of relatedness values.
    """
    matrix: List[List[float]] = []
    n = instance.number_of_vertices
    for i in range(n):
        matrix.append([0] * n)
        for j in range(n):
            if i != j:
                matrix[i][j] = relatedness_computer(i, j)
    return matrix


class RelatedRemovalOperator(routingblocks.DestroyOperator):
    """
    Removes related vertices from the solution. The operator first selects an initial seed vertex.
    Then, it selects the n most related vertices to the current seed vertex and adds them to the list of removed vertices.
    It then selects the next seed vertex from the list of removed vertices and repeats the process until the desired number of vertices has been selected.
    Finally, the operator removes the selected vertices from the solution.

    The operator determines related vertices by using a relatedness matrix passed to the constructor.
    This matrix contains a number that measures the degree of the relatedness between each pair of vertices.
    The higher the number, the more related the vertices are.

    (Initial) seed and related vertex selection is done using move selectors.
    """

    def __init__(self, relatedness_matrix: List[List[float]],
                 move_selector: MoveSelector[RelatedVertexRemovalMove],
                 seed_selector: MoveSelector[RelatedVertexRemovalMove],
                 initial_seed_selector: MoveSelector[routingblocks.Node],
                 cluster_size: int = 1):
        """

        :param relatedness_matrix: The relatedness matrix. See :py:func:`build_relatedness_matrix` for a way to build such a matrix.
        :param move_selector: The move selector to use for selecting the vertex to remove. Receives a list of related vertices, ordered by the degree of relatedness in descending order.
        :param seed_selector: The move selector to use for selecting the seed vertex.
        :param initial_seed_selector: The move selector to use for selecting the initial seed vertex.
        :param cluster_size: The number of related vertices to remove for each seed.
        """
        # Important: Do not use super()!
        routingblocks.DestroyOperator.__init__(self)
        self._relatedness_matrix = relatedness_matrix
        self._move_selector = move_selector
        self._seed_selector = seed_selector
        self._initial_seed_selector = initial_seed_selector
        self._cluster_size = cluster_size

        self._nodes_in_solution: List[Tuple[routingblocks.NodeLocation, routingblocks.Node]] = []

    def can_apply_to(self, _solution: routingblocks.Solution) -> bool:
        return len(_solution) > 0

    def _get_sorted_related_vertices(self, related_vertices: List[float],
                                     removed_vertices: Set[RelatedVertexRemovalMove]):
        related_vertices_in_solution = []
        # Iterate over solution and add entry for each node
        for node_location, node in self._nodes_in_solution:
            vertex_id = node.vertex_id
            candidate = RelatedVertexRemovalMove(vertex_id, related_vertices[vertex_id], node_location)
            if candidate not in removed_vertices:
                related_vertices_in_solution.append(candidate)

        # Sort by relatedness
        related_vertices_in_solution.sort(key=lambda x: x.relatedness, reverse=True)
        return related_vertices_in_solution

    def _remove_seed_and_related(self, solution: routingblocks.Solution,
                                 removed_vertices: List[RelatedVertexRemovalMove], num_vertices_to_remove: int):
        # Pick random node from already removed vertices
        seed_move = self._seed_selector(removed_vertices)
        seed_move_id = seed_move.vertex_id
        # seed_node_id = next(itertools.islice(removed_vertices, self._randgen.randint(0, len(removed_vertices) - 1), None)).vertex_id
        # Get related vertices
        related_vertices = self._get_sorted_related_vertices(self._relatedness_matrix[seed_move_id], removed_vertices)
        # Select related vertices to remove
        for _ in range(num_vertices_to_remove):
            next_vertex = self._move_selector(related_vertices)
            removed_vertices.append(next_vertex)
            related_vertices.remove(next_vertex)
        # Convenience return, actually modifies in-place
        return removed_vertices

    def _select_initial_seed(self, _solution: routingblocks.Solution) -> routingblocks.NodeLocation:
        return self._initial_seed_selector(x[0] for x in self._nodes_in_solution)

    def _cache_nodes_in_solution(self, solution: routingblocks.Solution):
        for node_location in solution.non_depot_nodes:
            self._nodes_in_solution.append((node_location, solution[node_location.route][node_location.position]))

    def apply(self, evaluation: routingblocks.Evaluation, _solution: routingblocks.Solution,
              number_of_removed_vertices: int) -> List[
        int]:
        # Cache nodes in the solution with their locations
        self._cache_nodes_in_solution(_solution)
        # Select seed node
        initial_seed_location = self._select_initial_seed(_solution)
        # seed_node_location = routingblocks.sample_locations(_solution, self._randgen, 1, False)[0]
        seed_node = _solution[initial_seed_location.route][initial_seed_location.position]
        # Initialize removed vertices
        removed_vertices = [RelatedVertexRemovalMove(seed_node.vertex_id, 1.0, initial_seed_location)]
        # Remove related
        while len(removed_vertices) < number_of_removed_vertices:
            num_vertices_to_remove = min(number_of_removed_vertices - len(removed_vertices), self._cluster_size)
            removed_vertices = self._remove_seed_and_related(_solution, removed_vertices,
                                                             num_vertices_to_remove)
        # Remove vertices
        _solution.remove_vertices([move.location for move in removed_vertices])
        # Clear the cache
        self._nodes_in_solution.clear()
        return [move.vertex_id for move in removed_vertices]

    def name(self) -> str:
        return self.__class__.__name__
