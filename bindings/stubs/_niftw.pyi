class NIFTWVertexData:
    """
    Data stored on vertices in an NIFTW problem setting.
    """

    def __init__(self, x: float, y: float, demand: resource_t, earliest_time_of_arrival: resource_t,
                 latest_time_of_arrival: resource_t, service_time: resource_t) -> None:
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

    def __init__(self, distance: resource_t, travel_time: resource_t, consumption: resource_t) -> None:
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
    overload_penalty_factor: float
    overcharge_penalty_factor: float
    time_shift_penalty_factor: float

    def __init__(self, vehicle_battery_capacity: resource_t, vehicle_storage_capacity: resource_t,
                 replenishment_time: resource_t) -> None: ...


class NIFTWFRVCP:
    def __init__(self, instance: Instance, battery_capacity_time: resource_t,
                 replenishment_time: resource_t) -> None: ...

    def optimize(self, route_vertex_ids: List[VertexID]) -> List[VertexID]: ...
