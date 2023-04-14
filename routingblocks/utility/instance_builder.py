from typing import Callable, TypeVar, Protocol, Optional

from .. import Instance, Vertex, Arc

T = TypeVar('T')
VertexID = int


class vertex_factory(Protocol):
    def __call__(self, id: VertexID, str_id: str, is_depot: bool, is_station: bool, data: T) -> Vertex:
        """
        Creates a new vertex using the given parameters.

        :param id: Numeric ID of the vertex
        :param str_id: Name of the vertex
        :param is_depot: Whether the vertex is a depot
        :param is_station: Whether the vertex is a station
        :param data: User-defined data to be stored in the vertex
        """
        ...


class arc_factory(Protocol):
    def __call__(self, data: T) -> Arc:
        """
        Creates a new arc with the given data.

        :param data: User-defined data to be stored in the arc
        :return:
        """
        ...


class InstanceBuilder:
    """
    A class for building an instance of a vehicle routing problem. The builder provides methods to add vertices (depot,
    customers, and stations) and arcs with their associated data. Once all the necessary data has been added, the builder
    can create an instance of the problem.
    """

    def __init__(self, create_vertex: Optional[vertex_factory] = None, create_arc: Optional[arc_factory] = None):
        """
        Initializes a new InstanceBuilder object.

        :param create_vertex: A factory function for creating vertex objects. Defaults to the constructor of the Vertex class.
        :param create_arc: A factory function for creating arc objects. Defaults to the constructor of the Arc class.
        """
        self._depot = None
        self._customers = {}
        self._stations = {}

        self._create_vertex = create_vertex if create_vertex is not None else Vertex
        self._create_arc = create_arc if create_arc is not None else Arc

        self.fleet_size = 0

        self._arcs = {}

    def _add_vertex(self, str_id: str, is_depot: bool, is_station: bool, vertex_data):
        assert is_depot + is_station < 2, "Vertex cannot be both depot and station"
        if is_depot:
            if self._depot is not None:
                raise ValueError(f"Instance already has a depot")
            self._depot = (str_id, False, True, vertex_data)
        elif is_station:
            self._stations[str_id] = (str_id, True, False, vertex_data)
        else:
            self._customers[str_id] = (str_id, False, False, vertex_data)

    def set_depot(self, str_id: str, vertex_data):
        """
        Sets the depot for the instance.

        :param str str_id: The string identifier for the depot.
        :param vertex_data: Additional data associated with the depot vertex.
        """
        self._add_vertex(str_id, True, False, vertex_data)

    def add_customer(self, str_id: str, vertex_data):
        """
        Adds a customer to the instance.

        :param str str_id: The string identifier for the customer.
        :param vertex_data: Additional data associated with the customer vertex.
        """
        self._add_vertex(str_id, False, False, vertex_data)

    def add_station(self, str_id: str, vertex_data):
        """
        Adds a station to the instance.

        :param str str_id: The string identifier for the station.
        :param vertex_data: Additional data associated with the station vertex.
        """
        self._add_vertex(str_id, False, True, vertex_data)

    def add_arc(self, i: str, j: str, arc_data):
        """
        Adds an arc between two vertices.

        :param str i: The string identifier for the source vertex.
        :param str j: The string identifier for the target vertex.
        :param arc_data: Additional data associated with the arc.
        """
        self._arcs[i, j] = arc_data

    @property
    def number_of_vertices(self):
        """
        Retrieves the number of vertices in the instance.

        :return: The number of vertices.
        :rtype: int
        """
        return (self._depot is not None) + len(self._customers) + len(self._stations)

    def reset(self):
        """
        Resets the InstanceBuilder, clearing all stored data.
        """
        self._depot = None
        self._customers.clear()
        self._stations.clear()
        self._arcs.clear()

    def build(self) -> Instance:
        """
        Constructs an Instance object based on the vertices and arcs added to the InstanceBuilder.
        Uses vertex_factory and arc_factory to create the vertices and arcs.

        :return: A new Instance object.
        :rtype: Instance
        :raises ValueError: If the InstanceBuilder does not have a depot or at least one customer.
        :raises ValueError: If not all arcs are defined between vertices.
        """
        if self._depot is None:
            raise ValueError(f"Instance requires depot")
        if len(self._customers) == 0:
            raise ValueError("Instance requires at least one customer")
        if len(self._arcs) != self.number_of_vertices * self.number_of_vertices:
            raise ValueError("Instance requires arcs between all vertices")

        vertices = [self._create_vertex(i, *vertex_data) for i, vertex_data in
                    enumerate((self._depot, *self._customers.values(),
                               *self._stations.values()))]

        arc_matrix = []
        for i, origin in enumerate(vertices):
            arc_matrix.append([])
            for j, destination in enumerate(vertices):
                arc_data = self._arcs.get((origin.str_id, destination.str_id))
                if arc_data is None:
                    raise ValueError(f"Instance requires arc between {origin.str_id} and {destination.str_id}")
                arc_matrix[i].append(self._create_arc(arc_data))

        return Instance(vertices, arc_matrix, self.fleet_size)
