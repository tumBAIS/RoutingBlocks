class ADPTWVertexData:
    def __init__(self, x: float, y: float, demand: resource_t, earliest_time_of_arrival: resource_t,
                 latest_time_of_arrival: resource_t, service_time: resource_t) -> ADPTWVertexData:
        ...


class ADPTWArcData:
    def __init__(self, distance: resource_t, travel_time: resource_t, consumption: resource_t) -> ADPTWArcData: ...


# adptw submodule
def create_adptw_vertex(vertex_id: int, str_id: str, is_station: bool, is_depot: bool,
                        data: ADPTWVertexData) -> Vertex: ...


def create_adptw_arc(data: ADPTWArcData) -> Arc: ...
