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

class ADPTWVertexData:
    """
    Data stored on vertices in an ADPTW problem setting.
    """

    def __init__(self, x: float, y: float, demand: float, earliest_time_of_arrival: float,
                 latest_time_of_arrival: float, service_time: float) -> ADPTWVertexData:
        """
        :param x: The x coordinate of the vertex.
        :param y: The y coordinate of the vertex.
        :param demand: The demand of the vertex. 0 for station and depot vertices.
        :param earliest_time_of_arrival: The earliest time at which service at the vertex can begin.
        :param latest_time_of_arrival: The latest time at which service at the vertex can begin.
        :param service_time: The time needed to serve the vertex.
        """
        ...


class ADPTWArcData:
    """
    Data stored on arcs in an ADPTW problem setting.
    """

    def __init__(self, distance: float, travel_time: float, consumption: float) -> ADPTWArcData:
        """
        :param distance: The distance between the two vertices connected by the arc.
        :param travel_time: The time it takes to travel between the two vertices connected by the arc.
        :param consumption: The time required to replenish the resource consumed when traveling between the two vertices connected by the arc.
        """
        ...


# adptw submodule
def create_adptw_vertex(vertex_id: int, str_id: str, is_facility: bool, is_depot: bool,
                        data: ADPTWVertexData) -> Vertex:
    """
    Creates a vertex for an ADPTW problem setting. Stores :param data directly as a native C++ object.

    .. warning::
        The data member of the created vertex is not accessible from python. Doing so will likely result in a crash.


    :param vertex_id: The unique identifier of the vertex.
    :param str_id: A human-readable string identifier for the vertex.
    :param is_facility: Whether the vertex is a replenishment facility.
    :param is_depot: Whether the vertex is a depot.
    :param data: The data to associate with this vertex.
    :return:
    """
    ...


def create_adptw_arc(data: ADPTWArcData) -> Arc:
    """
    Creates an arc for an ADPTW problem setting. Stores :param data directly as a native C++ object.

    .. warning::
        The data member of the created arc is not accessible from python. Doing so will likely result in a crash.

    :param data: The data to associate with this vertex
    :return:
    """
    ...


class ADPTWEvaluation(PyEvaluation):
    """
    Evaluation for ADPTW problems. Works only with arcs and vertices created using :ref:`create_adptw_arc` and :ref:`create_adptw_vertex`.
    Uses a set of penalty factors to penalize infeasible solutions.

    :var overload_penalty_factor: The penalty factor for overloading the vehicle.
    :var resource_penalty_factor: The penalty factor for consuming more resources than carried the vehicle.
    :var time_shift_penalty_factor: The penalty factor for time shifts.
    """

    overload_penalty_factor: float
    resource_penalty_factor: float
    time_shift_penalty_factor: float

    def __init__(self, vehicle_resource_capacity: float, vehicle_storage_capacity: float) -> None:
        """
        :param float vehicle_resource_capacity: The vehicle's battery capacity expressed in units of time, that is, the time it takes to fully replenish the resource of an empty vehicle.
        :param float vehicle_storage_capacity: The vehicle's storage capacity. Determines how much demand can be served in a single route.
        """
        ...


class ADPTWFacilityPlacementOptimizer:
    """
    ADPTW-specific detour insertion algorithm. Inserts visits to replenishment facilities at optimal locations into a route.
    """

    def __init__(self, instance: Instance, resource_capacity_time: float) -> None:
        """

        :param instance: The instance.
        :param resource_capacity_time: The vehicle's resource capacity expressed in units of time, that is, the time it takes to fully replenish the resource of an empty vehicle.
        """
        ...

    def optimize(self, route_vertex_ids: List[VertexID]) -> List[VertexID]:
        """
        Optimizes the route by inserting visits to replenishment facilities at optimal locations.
        :param route_vertex_ids: The vertex ids of the route to optimize.
        :return: The optimized route as a list of vertex ids.
        """
        ...
