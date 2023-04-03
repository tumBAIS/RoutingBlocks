from __future__ import annotations
from bisect import bisect_left
from typing import List, Callable, Iterable, Optional, Tuple, Union

import routingblocks

SeedSelector = Callable[[routingblocks.Evaluation, routingblocks.Solution, List[int]], routingblocks.Vertex]
ClusterMemberSelector = Callable[
    [routingblocks.Evaluation, routingblocks.Solution, routingblocks.Vertex], Iterable[routingblocks.Vertex]]


class ClusterRemovalOperator(routingblocks.DestroyOperator):
    def __init__(self, seed_selector: SeedSelector, cluster_member_selector: ClusterMemberSelector):
        # Important: Do not use super()!
        routingblocks.DestroyOperator.__init__(self)
        self._seed_selector = seed_selector
        self._cluster_member_selector = cluster_member_selector

    def can_apply_to(self, _solution: routingblocks.Solution) -> bool:
        return len(_solution) > 0

    def apply(self, evaluation: routingblocks.Evaluation, solution: routingblocks.Solution,
              number_of_removed_vertices: int) -> List[
        int]:
        removed_customers = []
        while len(solution) > 0 and len(removed_customers) < number_of_removed_vertices:
            # Get next seed
            try:
                next_seed_vertex = self._seed_selector(evaluation, solution, removed_customers)
            except StopIteration:
                # Abort when the cluster member selector throws, i.e., the solution has no further eligible clusters
                break
            # Get members of that seed
            for x in self._cluster_member_selector(evaluation, solution, next_seed_vertex):
                removed_customers.append(x)
                if len(removed_customers) == number_of_removed_vertices:
                    break
        return list(removed_customers)

    def name(self) -> str:
        return "ClusterRemovalOperator"


class DistanceBasedClusterMemberSelector:
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
        self._min_radius_factor = min_radius_factor
        self._max_radius_factor = max_radius_factor

        if (self._min_radius_factor is not None) + (self._max_radius_factor is not None) == 1:
            raise ValueError("Either both or none of min_radius_factor and max_radius_factor must be set.")

        self._randgen = randgen

        # Cache max distance and radii
        self._max_distance = 0.
        self._distance_list: List[List[DistanceBasedClusterMemberSelector.DistanceListItem]] = [
            [] for _ in range(0, max(x.id for x in vertices) + 1)
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

    def __call__(self, evaluation: routingblocks.Evaluation, solution: routingblocks.Solution,
                 seed_vertex: routingblocks.Vertex):
        closest_vertices = self._distance_list[seed_vertex.vertex_id]
        distance_cutoff = self._pick_distance()
        cutoff_idx = bisect_left(closest_vertices, distance_cutoff)
        return [closest_vertices[idx].vertex for idx in range(cutoff_idx)]
