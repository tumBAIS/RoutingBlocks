from typing import Iterator, overload, List


class Instance:
    @property
    def fleet_size(self) -> int:
        """Return the number of vehicles available."""

    @property
    def number_of_customers(self) -> int:
        """Return the number of customers."""

    @property
    def number_of_stations(self) -> int:
        """Return the number of stations."""

    @property
    def number_of_vertices(self) -> int:
        """Return the number of vertices."""

    @property
    def depot(self) -> Vertex:
        """Return the depot vertex."""

    @property
    def stations(self) -> Iterator[Vertex]:
        """Return an iterator over the station vertices."""

    @property
    def customers(self) -> Iterator[Vertex]:
        """Return an iterator over the customer vertices."""

    @overload
    def __init__(self, vertices: List[Vertex], arcs: List[List[Arc]]) -> None:
        """Initialize an Instance with a list of vertices and a list of arcs. Sets the fleet size to the number of customers.
        Expects vertices to be in the order depot, stations, customers."""

    @overload
    def __init__(self, vertices: List[Vertex], arcs: List[List[Arc]], fleet_size: int) -> None:
        """Initialize an Instance with a list of vertices, a list of arcs, and a fleet size. Expects vertices to be in the
        order depot, stations, customers."""

    @overload
    def __init__(self, depot: Vertex, stations: List[Vertex], customers: List[Vertex], arcs: List[List[Arc]],
                 fleet_size: int) -> None:
        """Initialize an Instance with a depot, lists of stations and customers, a list of arcs, and a fleet size."""

    def __len__(self) -> int:
        """Return the number of vertices in the instance."""

    def __iter__(self) -> Iterator[Vertex]:
        """Return an iterator over the vertices in the instance."""

    def get_vertex(self, id: int) -> Vertex:
        """Get a vertex by its ID."""

    def get_customer(self, customer_index: int) -> Vertex:
        """Get the n-th customer vertex."""

    def get_station(self, station_index: int) -> Vertex:
        """Get the n-th station vertex."""

    def get_arc(self, source_vertex_id: int, target_vertex_id: int) -> Arc: ...
