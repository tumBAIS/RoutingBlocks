from typing import Iterable, Tuple, Dict
from pydantic.dataclasses import dataclass
from pydantic import root_validator, validator
from enum import Enum
from itertools import product

VertexID = str
ArcID = Tuple[str, str]


class VertexType(Enum):
    Depot = 'd'
    Station = 'f'
    Customer = 'c'


@dataclass
class Vertex:
    vertex_id: VertexID
    vertex_type: VertexType
    x_coord: float
    y_coord: float
    demand: float
    ready_time: float
    due_time: float
    service_time: float

    @property
    def is_customer(self) -> bool:
        return self.vertex_type == VertexType.Customer

    @property
    def is_station(self) -> bool:
        return self.vertex_type == VertexType.Station

    @property
    def is_depot(self) -> bool:
        return self.vertex_type == VertexType.Depot

    @root_validator
    def check_depot_sation_demand(cls, values: Dict) -> Dict:
        if values['vertex_type'] in (VertexType.Depot, VertexType.Station):
            if values['demand'] != 0.0:
                raise ValueError(
                    "stations or depots cannot have a non-zero demand")
            if values['service_time'] != 0.0:
                raise ValueError(
                    "stations or depots cannot have a non-zero service_time")
        return values

    @validator('service_time', 'demand', 'ready_time', 'due_time')
    def check_nonzero_members(cls, value):
        if value < 0:
            raise ValueError(
                '_vertex demand, service_time, ready_time, and due_time must be at least 0')
        return value


@dataclass
class Arc:
    consumption: float
    travel_time: float
    distance: float

    @validator('*')
    def check_nonzero_members(cls, value):
        if value < 0:
            raise ValueError('negative arcs are not allowed')
        return value

    @property
    def cost(self) -> float:
        return self.distance


@dataclass
class Parameters:
    battery_capacity: float  # energy
    capacity: float  # demand
    consumption_rate: float  # kwh/distance
    recharging_rate: float  # energy/time
    velocity: float  # distance/time
    fleet_size: int

    @validator('*')
    def check_nonzero_members(cls, value):
        if value <= 0.:
            raise ValueError('parameter values must be greater than 0')
        return value

    @property
    def battery_capacity_time(self) -> float:
        return self.battery_capacity / self.recharging_rate


@dataclass
class Instance:
    parameters: Parameters
    vertices: Dict[VertexID, Vertex]
    arcs: Dict[ArcID, Arc]

    @validator('vertices')
    def check_single_depot(cls, vertices: Dict[VertexID, Vertex]):
        if sum(1 for x in vertices.values() if x.is_depot) != 1:
            raise ValueError('expected exactly one depot')
        return vertices

    @validator('vertices')
    def check_at_least_one_customer(cls, vertices: Dict[VertexID, Vertex]):
        if sum(1 for x in vertices.values() if x.is_customer) == 0:
            raise ValueError('expected at least one customer')
        return vertices

    @validator('vertices')
    def check_vertex_ids_match(cls, vertices: Dict[VertexID, Vertex]):
        for v_id, v in vertices.items():
            if v_id != v.vertex_id:
                raise ValueError(f'Vertex {v} has id {v.vertex_id} but expected {v_id}')
        return vertices

    @root_validator(skip_on_failure=True)
    def check_arc_matrix_complete(cls, members):
        arcs = members['arcs']
        vertices = members['vertices']
        for u, v in product(vertices.values(), repeat=2):
            if (u.vertex_id, v.vertex_id) not in arcs:
                raise ValueError("arc ({(u.vertex_id, v.vertex_id)}) is missing")
        if len(arcs) != len(vertices) ** 2:
            raise ValueError('too many arcs: got {len(arcs)} but expected {len(vertices)**2}')

        return members

    @property
    def depot(self) -> Vertex:
        return self.vertices['D0']

    @property
    def stations(self) -> Iterable[Vertex]:
        return (i for i in self.vertices.values() if i.is_station)

    @property
    def customers(self) -> Iterable[Vertex]:
        return (i for i in self.vertices.values() if i.is_customer)
