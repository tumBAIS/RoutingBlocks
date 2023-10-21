from typing import Iterator, overload, List


class Instance:
    """
    Represents an instance of a vehicle routing problem. The instance contains a collection of vertices (depot, stations,
    and customers), a matrix of arcs connecting the vertices, and a fleet size representing the number of vehicles available.
    Provides convenient methods to access and iterate through the various types of vertices.

    .. note::
        It is recommend to use the :ref:`InstanceBuilder <instance-builder>` to create instances.

    """

    @overload
    def __init__(self, depot: Vertex, stations: List[Vertex], customers: List[Vertex], arcs: List[List[Arc]],
                 fleet_size: int) -> None:
        """
        :param Vertex depot: The depot vertex
        :param List[Vertex] stations: A list of station vertices
        :param List[Vertex] customers: A list of customer vertices
        :param List[List[Arc]]] arcs: A matrix of Arc objects representing the connections between vertices
        :param int fleet_size: The number of vehicles in the fleet
        """
        ...

    @overload
    def __init__(self, vertices: List[Vertex], arcs: List[List[Arc]]) -> None:
        """
        :param List[Vertex] vertices: A list of vertices in the order depot, stations, customers
        :param List[List[Arc]] arcs: A list of lists of Arc objects representing the connections between vertices
        """
        ...

    @overload
    def __init__(self, vertices: List[Vertex], arcs: List[List[Arc]], fleet_size: int) -> None:
        """
        :param List[Vertex] vertices: A list of vertices in the order depot, stations, customers
        :param List[List[Arc]] arcs: A list of lists of Arc objects representing the connections between vertices
        :param int fleet_size: The number of vehicles in the fleet
        """
        ...

    @property
    def fleet_size(self) -> int:
        """
        Retrieves the number of vehicles available.

        :return: The fleet size.
        :rtype: int
        """
        ...

    @property
    def number_of_customers(self) -> int:
        """
        Retrieves the number of customers.

        :return: The number of customers.
        :rtype: int
        """
        ...

    @property
    def number_of_stations(self) -> int:
        """
        Retrieves the number of stations.

        :return: The number of stations.
        :rtype: int
        """
        ...

    @property
    def number_of_vertices(self) -> int:
        """
        Retrieves the number of vertices.

        :return: The number of vertices.
        :rtype: int
        """
        ...

    @property
    def depot(self) -> Vertex:
        """
        Retrieves the depot vertex.

        :return: The depot vertex.
        :rtype: Vertex
        """
        ...

    @property
    def vertices(self) -> Iterator[Vertex]:
        """
        Retrieves an iterator over the instance's vertices.

        :return: An iterator over the instance's vertices.
        :rtype: Iterator[Vertex]
        """
        ...

    @property
    def stations(self) -> Iterator[Vertex]:
        """
        Retrieves an iterator over the station vertices.

        :return: An iterator over the station vertices.
        :rtype: Iterator[Vertex]
        """
        ...

    @property
    def customers(self) -> Iterator[Vertex]:
        """
        Retrieves an iterator over the customer vertices.

        :return: An iterator over the customer vertices.
        :rtype: Iterator[Vertex]
        """
        ...

    def __len__(self) -> int:
        """
        Retrieves the number of vertices in the instance.

        :return: The number of vertices.
        :rtype: int
        """
        ...

    def __iter__(self) -> Iterator[Vertex]:
        """
        Retrieves an iterator over the vertices in the instance.

        :return: An iterator over the vertices.
        :rtype: Iterator[Vertex]
        """
        ...

    def get_vertex(self, id: int) -> Vertex:
        """
        Retrieves a vertex by its ID.

        :param int id: The ID of the desired vertex.
        :return: The vertex with the specified ID.
        :rtype: Vertex
        """
        ...

    def get_customer(self, customer_index: int) -> Vertex:
        """
        Retrieves the n-th customer vertex.

        :param int customer_index: The index of the desired customer vertex.
        :return: The customer vertex at the specified index.
        :rtype: Vertex
        """
        ...

    def get_station(self, station_index: int) -> Vertex:
        """
        Retrieves the n-th station vertex.

        :param int station_index: The index of the desired station vertex.
        :return: The station vertex at the specified index.
        :rtype: Vertex
        """
        ...

    def get_arc(self, source_vertex_id: int, target_vertex_id: int) -> Arc:
        """
        Retrieves the arc connecting two vertices, specified by their IDs.

        :param int source_vertex_id: The ID of the source vertex.
        :param int target_vertex_id: The ID of the target vertex.
        :return: The arc connecting the source and target vertices.
        :rtype: Arc
        """
        ...
