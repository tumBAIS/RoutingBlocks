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

from __future__ import annotations
from bisect import bisect_left
from typing import List, Callable, Iterable, Optional, Tuple, Union, Protocol

import routingblocks


class SeedSelector(Protocol):
    """
    Selects a seed vertex from a solution.
    """

    def __call__(self, evaluation: routingblocks.Evaluation, solution: routingblocks.Solution,
                 already_selected_vertices: List[routingblocks.NodeLocation]) -> routingblocks.NodeLocation:
        """
        :param evaluation: The evaluation to use
        :param solution: The solution to select from
        :param already_selected_vertices: A list of vertices that have already been selected, i.e., should not be included in the selection
        :return:
        """
        ...


class ClusterMemberSelector(Protocol):
    """
    Selects a cluster of vertices based on a seed vertex.
    """

    def __call__(self, evaluation: routingblocks.Evaluation, solution: routingblocks.Solution,
                 seed: routingblocks.NodeLocation) -> List[routingblocks.NodeLocation]:
        """
        :param evaluation: The evaluation to use
        :param solution: The solution to select from
        :param seed: The seed vertex
        :return:
        """
        ...


class ClusterRemovalOperator(routingblocks.DestroyOperator):
    """
    The ClusterRemovalOperator is a generic destroy operator that removes clusters of vertices from a solution.
    The operator first selects a seed vertex, and then selects a cluster of vertices around that seed.
    These two steps are repeated until the desired number of vertices has been selected or no seed vertices can be
    identified. The selected vertices are then removed from the solution. Note that seed vertices are not removed unless
    they are also selected by the cluster member selector.

    The seed selection and cluster member selection are delegated to the :py:class:`routingblocks.operators.SeedSelector`
    and :py:class:`routingblocks.operators.ClusterMemberSelector` parameters, respectively.
    This allows to customize the operator for different use cases.
    """

    def __init__(self, seed_selector: SeedSelector, cluster_member_selector: ClusterMemberSelector):
        """
        :param seed_selector: The seed selector
        :param cluster_member_selector: The cluster member selector
        """
        # Important: Do not use super()!
        routingblocks.DestroyOperator.__init__(self)
        self._seed_selector = seed_selector
        self._cluster_member_selector = cluster_member_selector

    def can_apply_to(self, _solution: routingblocks.Solution) -> bool:
        return len(_solution) > 0

    def apply(self, evaluation: routingblocks.Evaluation, solution: routingblocks.Solution,
              number_of_removed_vertices: int) -> List[
        int]:
        removed_vertices: List[routingblocks.NodeLocation] = []
        while len(solution) > 0 and len(removed_vertices) < number_of_removed_vertices:
            # Get next seed
            try:
                next_seed_vertex = self._seed_selector(evaluation, solution, removed_vertices)
            except StopIteration:
                # Abort when the cluster member selector throws, i.e., the solution has no further eligible clusters
                break
            # Get members of that seed
            for x in self._cluster_member_selector(evaluation, solution, next_seed_vertex):
                removed_vertices.append(x)
                if len(removed_vertices) == number_of_removed_vertices:
                    break

        removed_vertex_ids = [solution.lookup(x).vertex_id for x in removed_vertices]

        solution.remove_vertices(removed_vertices)

        return removed_vertex_ids

    def name(self) -> str:
        return "ClusterRemovalOperator"


class DistanceBasedClusterMemberSelector:
    """
    Clusters vertices according to their distance to the seed vertex.
    """

    class DistanceListItem:
        def __init__(self, vertex: routingblocks.Vertex, distance: float):
            self.vertex = vertex
            self.distance = distance

        def __lt__(self, other: Union[DistanceBasedClusterMemberSelector.DistanceListItem, float]):
            if isinstance(other, DistanceBasedClusterMemberSelector.DistanceListItem):
                return self.distance < other.distance
            else:
                return self.distance < other

    def __init__(self, vertices: List[routingblocks.Vertex],
                 get_distance: Callable[[routingblocks.Vertex, routingblocks.Vertex], float],
                 min_radius_factor: float = 1.0, max_radius_factor: float = 1.0,
                 randgen: Optional[routingblocks.Randgen] = None):
        """
        :param vertices: The vertices in the instance
        :param get_distance: A distance function that takes two vertices and returns their distance to each other
        :param min_radius_factor: The minimum radius of the cluster as a factor of the maximum distance between any two vertices
        :param max_radius_factor: The maximum radius of the cluster as a factor of the maximum distance between any two vertices
        :param randgen: A random number generator
        """
        self._min_radius_factor = min_radius_factor
        self._max_radius_factor = max_radius_factor

        if (self._min_radius_factor is not None) + (self._max_radius_factor is not None) == 1:
            raise ValueError("Either both or none of min_radius_factor and max_radius_factor must be set.")

        self._randgen = randgen

        # Cache max distance and radii
        self._max_distance = 0.
        self._distance_list: List[List[DistanceBasedClusterMemberSelector.DistanceListItem]] = [
            [] for _ in range(0, max(x.vertex_id for x in vertices) + 1)
        ]
        for i in vertices:
            for j in vertices:
                distance = get_distance(i, j)
                self._distance_list[i.vertex_id].append(
                    DistanceBasedClusterMemberSelector.DistanceListItem(j, distance))
                self._max_distance = max(distance, self._max_distance)
            self._distance_list[i.vertex_id].sort()

    def _pick_distance(self):
        if self._min_radius_factor == self._max_radius_factor:
            return self._min_radius_factor * self._max_distance
        if self._randgen is None:
            return self._max_distance
        return self._randgen.uniform(self._min_radius_factor, self._max_radius_factor) * self._max_distance

    def _select_vertices(self, seed_vertex: routingblocks.Vertex):
        closest_vertices = self._distance_list[seed_vertex.vertex_id]
        distance_cutoff = self._pick_distance()
        cutoff_idx = bisect_left(closest_vertices, distance_cutoff)
        return [closest_vertices[idx].vertex for idx in range(cutoff_idx)]

    def __call__(self, evaluation: routingblocks.Evaluation, solution: routingblocks.Solution,
                 seed_vertex: routingblocks.NodeLocation) -> List[routingblocks.NodeLocation]:
        selected_vertices = self._select_vertices(solution.lookup(seed_vertex).vertex)
        return [x for vertex in selected_vertices for x in solution.find(vertex.vertex_id)]
