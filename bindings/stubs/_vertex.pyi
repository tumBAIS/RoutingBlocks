from typing import Any


class Vertex:
    """
    A simple vertex object that represents a location vehicles can visit. Vertices can be stations, depots or customers.
    Each vertex has a unique id and a human-readable string identifier. The vertex also stores additional data transparent
    to the RoutingBlocks package. This data can be used to store additional information about the vertex, such as time windows,
    demand, prizes, or any other attribute that is relevant to the problem.
    """
    vertex_id: int  # The unique identifier of the vertex.
    str_id: str
    is_station: bool
    is_depot: bool

    def __init__(self, vertex_id: int, str_id: str, is_station: bool, is_depot: bool, data: Any) -> None:
        """
        :param vertex_id: The unique identifier of the vertex.
        :param str_id: A human-readable string identifier for the vertex.
        :param is_station: Whether the vertex is a station.
        :param is_depot: Whether the vertex is a depot.
        :param data: Additional data associated with the vertex.
        """
        ...

    @property
    def is_customer(self) -> bool:
        """
        Determines if the vertex is a customer.

        :return: True if the vertex is a customer, False otherwise.
        :rtype: bool
        """
        ...

    @property
    def data(self) -> Any:
        """
        Retrieves the vertex data.

        :return: The data associated with the vertex.
        :rtype: Any
        """

    def __str__(self) -> str:
        """
        Returns a human-readable string representation of the vertex based on the arc's :ivar str_id.

        :return: A string representation of the vertex.
        :rtype: str
        """
        ...
