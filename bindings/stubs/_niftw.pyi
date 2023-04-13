class NIFTWVertexData:
    def __init__(self, x: float, y: float, demand: resource_t, earliest_time_of_arrival: resource_t,
                 latest_time_of_arrival: resource_t, service_time: resource_t) -> None:
        ...


class NIFTWArcData:
    def __init__(self, distance: resource_t, travel_time: resource_t, consumption: resource_t) -> None:
        ...


# niftw submodule
def create_niftw_vertex(vertex_id: int, str_id: str, is_station: bool, is_depot: bool, data: NIFTWVertexData) -> Vertex:
    ...


def create_niftw_arc(data: NIFTWArcData) -> Arc: ...
