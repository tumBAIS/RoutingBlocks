from typing import Callable, TypeVar

from .. import Instance, Vertex, Arc

T = TypeVar('T')
VertexID = int
vertex_factory = Callable[[VertexID, str, bool, bool, T], Vertex]
arc_factory = Callable[[T], Arc]


class InstanceBuilder:
    def __init__(self, create_vertex: vertex_factory = Vertex, create_arc: arc_factory = Arc):
        self._depot = None
        self._customers = {}
        self._stations = {}

        self._create_vertex = create_vertex
        self._create_arc = create_arc

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
        self._add_vertex(str_id, True, False, vertex_data)

    def add_customer(self, str_id: str, vertex_data):
        self._add_vertex(str_id, False, False, vertex_data)

    def add_station(self, str_id: str, vertex_data):
        self._add_vertex(str_id, False, True, vertex_data)

    def add_arc(self, i: str, j: str, arc_data):
        self._arcs[i, j] = arc_data

    @property
    def number_of_vertices(self):
        return (self._depot is not None) + len(self._customers) + len(self._stations)

    def reset(self):
        self._depot = None
        self._customers.clear()
        self._stations.clear()
        self._arcs.clear()

    def build(self) -> Instance:
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
