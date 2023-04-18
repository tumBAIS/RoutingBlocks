from typing import Any


class Arc:
    """
    A simple arc object that represents a connection between two vertices in a graph. The arc stores additional data
    transparent to the RoutingBlocks package. This data can be used to store additional information about the arc, such as
    distances, durations, or any other attributes relevant to the problem being modeled.
    """

    def __init__(self, data: Any) -> None:
        """
        :param data: Additional data associated with the arc.
        """
        ...

    @property
    def data(self) -> Any:
        """
        Retrieves the arc data.

        :return: The data associated with the arc.
        :rtype: Any
        """

    def __str__(self) -> str:
        """
        Generates a human-readable string representation of the arc.

        :return: A string representation of the arc.
        :rtype: str
        """
        ...
