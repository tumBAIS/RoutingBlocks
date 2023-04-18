class NodeLocation:
    """
    A class representing the location of a node within a solution. Stores the route index and the position of the node within that route.
    """
    route: int
    position: int

    def __init__(self, route: int, position: int) -> None:
        """
        :param int route: The index of the route in which the node is located.
        :param int position: The position of the node within the route.
        """
        ...

    def __eq__(self, other: NodeLocation) -> bool: ...

    def __getitem__(self, i: int) -> int: ...

    def __len__(self) -> int: ...

    def __lt__(self, other: NodeLocation) -> bool: ...

    def __ne__(self, other: NodeLocation) -> bool: ...
