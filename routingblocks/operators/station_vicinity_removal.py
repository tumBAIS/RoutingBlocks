from __future__ import annotations
from typing import List, Callable, Iterable, Optional, Tuple, Union

import routingblocks
from .cluster_removal import ClusterRemovalOperator, DistanceBasedClusterMemberSelector


class StationSeedSelector:
    def __init__(self, stations: List[routingblocks.Vertex], randgen: Optional[routingblocks.Randgen] = None):
        self._stations = stations
        self._randgen = randgen

    def _get_station_locations(self, solution: routingblocks.Solution):
        return [
            y for x in self._stations for y in solution.find(x.id)
        ]

    def __call__(self, evaluation: routingblocks.Evaluation, solution: routingblocks.Solution,
                 removed_vertices: List[int]) -> routingblocks.Vertex:
        # Get all stations in the solution not removed yet
        stations = [x for x in self._get_station_locations(solution) if
                    solution.lookup(x).vertex_id not in removed_vertices]
        if len(stations) == 0:
            raise StopIteration
        # Return a random one
        picked_station_location = stations[self._randgen.randint(0, len(stations) - 1)]
        return solution.lookup(picked_station_location).vertex


class StationVicinityRemovalOperator(routingblocks.DestroyOperator):
    def __init__(self, instance: routingblocks.Instance,
                 get_distance: Callable[[routingblocks.Vertex, routingblocks.Vertex], float],
                 min_radius_factor: float, max_radius_factor: float,
                 randgen: Optional[routingblocks.Randgen]):
        routingblocks.DestroyOperator.__init__(self)
        self._cluster_removal_operator = ClusterRemovalOperator(
            seed_selector=StationSeedSelector(list(instance.stations), randgen),
            cluster_member_selector=DistanceBasedClusterMemberSelector(
                vertices=[*instance.stations, *instance.customers],
                get_distance=get_distance,
                min_radius_factor=min_radius_factor,
                max_radius_factor=max_radius_factor,
                randgen=randgen)
        )

    def can_apply_to(self, _solution: routingblocks.Solution) -> bool:
        return any(node.vertex.is_station
                   for route in _solution.routes for node in route)

    def name(self) -> str:
        return "StationVicinityRemovalOperator"

    def apply(self, evaluation: routingblocks.Evaluation, solution: routingblocks.Solution,
              number_of_removed_vertices: int) -> List[int]:
        return self._cluster_removal_operator.apply(evaluation, solution, number_of_removed_vertices)
