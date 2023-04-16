class RemovalMove:
    vertex_id: VertexID
    node_location: NodeLocation
    delta_cost: float

    def __init__(self, vertex_id: VertexID, node_location: NodeLocation, delta_cost: float) -> None: ...

    def __eq__(self, other: RemovalMove) -> bool: ...


class RemovalCache:
    def __init__(self, instance: Instance) -> None: ...

    def clear(self) -> None: ...

    def invalidate_route(self, route: Route, route_index: int) -> None: ...

    def rebuild(self, evaluation: Evaluation, solution: Solution) -> None: ...

    @property
    def moves_in_order(self) -> List[RemovalMove]: ...
