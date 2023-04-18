class NIFTWVertexData:
    """
    Data stored on vertices in an NIFTW problem setting.
    """

    def __init__(self, x: float, y: float, demand: float, earliest_time_of_arrival: float,
                 latest_time_of_arrival: float, service_time: float) -> None:
        """
        :param x: The x coordinate of the vertex.
        :param y: The y coordinate of the vertex.
        :param demand: The demand of the vertex. 0 for station and depot vertices.
        :param earliest_time_of_arrival: The earliest time at which service at the vertex can begin.
        :param latest_time_of_arrival: The latest time at which service at the vertex can begin.
        :param service_time: The time needed to serve the vertex.
        """
        ...


class NIFTWArcData:
    """
    Data stored on arcs in an NIFTW problem setting.
    """

    def __init__(self, distance: float, travel_time: float, consumption: float) -> None:
        """
        :param distance: The distance between the two vertices connected by the arc.
        :param travel_time: The time it takes to travel between the two vertices connected by the arc.
        :param consumption: The time required to recharge the energy consumed when traveling between the two vertices connected by the arc.
        """
        ...


# niftw submodule
def create_niftw_vertex(vertex_id: int, str_id: str, is_station: bool, is_depot: bool, data: NIFTWVertexData) -> Vertex:
    """
    Creates a vertex for an NIFTW problem setting. Stores :param data directly as a native C++ object.

    .. warning::
        The data member of the created vertex is not accessible from python. Doing so will likely result in a crash.


    :param vertex_id: The unique identifier of the vertex.
    :param str_id: A human-readable string identifier for the vertex.
    :param is_station: Whether the vertex is a station.
    :param is_depot: Whether the vertex is a depot.
    :param data: The data to associate with this vertex.
    :return:
    """
    ...


def create_niftw_arc(data: NIFTWArcData) -> Arc:
    """
    Creates an arc for an NIFTW problem setting. Stores :param data directly as a native C++ object.

    .. warning::
        The data member of the created arc is not accessible from python. Doing so will likely result in a crash.

    :param data: The data to associate with this vertex
    :return:
    """
    ...


class NIFTWEvaluation(Evaluation):
    """
    Evaluation for NIFTW problems. Works only with arcs and vertices created using :ref:`create_niftw_arc` and :ref:`create_niftw_vertex`.
    Uses a set of penalty factors to penalize infeasible solutions.

    :var overload_penalty_factor: The penalty factor for overloading the vehicle.
    :var overcharge_penalty_factor: The penalty factor for overcharging the vehicle.
    :var time_shift_penalty_factor: The penalty factor for time shifts.
    """
    overload_penalty_factor: float
    overcharge_penalty_factor: float
    time_shift_penalty_factor: float

    def __init__(self, vehicle_battery_capacity: float, vehicle_storage_capacity: float,
                 replenishment_time: float) -> None:
        """
        :param vehicle_battery_capacity: The vehicle's battery capacity expressed in units of time, that is, the time it takes to fully recharge an empty battery.
        :param vehicle_storage_capacity: The vehicle's storage capacity. Determines how much demand can be served in a single route.
        :param replenishment_time: The time penalty incurred to replenish the vehicle's battery.
        """
        ...


class NIFTWFRVCP:
    def __init__(self, instance: Instance, battery_capacity_time: float,
                 replenishment_time: float) -> None: ...

    def optimize(self, route_vertex_ids: List[VertexID]) -> List[VertexID]: ...
