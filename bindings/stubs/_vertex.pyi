from typing import Any


class Vertex:
    vertex_id: int
    str_id: str
    is_station: bool
    is_depot: bool

    def __init__(self, vertex_id: int, str_id: str, is_station: bool, is_depot: bool, data: Any) -> None:
        """Creates a new vertex."""
        ...

    @property
    def is_customer(self) -> bool:
        """Whether the vertex is a customer."""
        ...

    @property
    def data(self) -> Any:
        """The vertex data. Only well-defined for VertexData classes defined in Python."""
        ...

    def __str__(self) -> str: ...
