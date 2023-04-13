from typing import Any


class Arc:
    def __init__(self, data: Any) -> None:
        """Creates a new arc."""
        ...

    @property
    def data(self) -> Any:
        """The arc data. Only well-defined for ArcData classes defined in Python."""
        ...
