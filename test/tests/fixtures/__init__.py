from __future__ import annotations

import random
from typing import Callable, List, TypeVar, Tuple

import pytest
from pathlib import Path

import routingblocks

import helpers as py_evrptw

instance_type = Tuple[py_evrptw.Instance, routingblocks.Instance]

from fixtures.mock_evaluation import *

T = TypeVar('T')


def create_solution(instance, evaluation, raw_routes):
    return routingblocks.Solution(evaluation, instance,
                                  [routingblocks.create_route(evaluation, instance, route) for route in raw_routes])


@pytest.fixture
def instance_parser() -> Callable[[str], InstanceType]:
    def parse_instance(instance_name: str) -> InstanceType:
        py_instance = py_evrptw.parse_evrptw_instance(Path(__file__).parent / "data" / instance_name)
        cpp_instance = py_evrptw.create_cpp_instance(py_instance)
        return py_instance, cpp_instance

    return parse_instance


@pytest.fixture
def instance(instance_parser) -> InstanceType:
    return instance_parser("c101C5.txt")


@pytest.fixture
def large_instance(instance_parser) -> InstanceType:
    return instance_parser("r101_21.txt")


@pytest.fixture
def large_adptw_instance(instance_parser) -> routingblocks.Instance:
    return instance_parser("r101_21.txt")[1]


@pytest.fixture
def adptw_instance(instance_parser) -> routingblocks.Instance:
    return instance_parser("c101C5.txt")[1]


@pytest.fixture
def randgen() -> random.Random:
    return routingblocks.Random(0)


def distribute_randomly(sequence: List[T], num_subsequences: int, randgen=None) -> List[List[T]]:
    subsequences = [[] for _ in range(num_subsequences)]
    if randgen is None:
        randgen = random
    for item in sequence:
        subsequences[randgen.randint(0, len(subsequences) - 1)].append(item)
    return subsequences


def sample_vertices_from_instance(instance, randgen=None):
    if randgen is None:
        randgen = random
    return [*instance.customers] + randgen.choices(list(instance.stations),
                                                   k=int(instance.number_of_stations * 1.5))


@pytest.fixture
def random_routes_factory():
    def build(evaluation, instance: routingblocks.Instance, n_routes: int,
              vertices: List[routingblocks.Vertex] = None, randgen=None) -> List[routingblocks.Route]:
        if vertices is None:
            vertices = sample_vertices_from_instance(instance=instance, randgen=randgen)
        assigned_vertices = distribute_randomly(vertices, n_routes, randgen=randgen)

        def create_route(vertices: List[routingblocks.Vertex]) -> routingblocks.Route:
            if len(vertices) > 0:
                return routingblocks.create_route(evaluation, instance,
                                                  [(x.vertex_id if isinstance(x, routingblocks.Vertex) else x) for x in
                                                   vertices])
            return routingblocks.Route(evaluation, instance)

        routes = [create_route(route_vertices) for route_vertices in assigned_vertices]
        return routes

    return build


@pytest.fixture
def random_raw_route_factory():
    def build(instance, randgen=None, include_depot=False):
        if randgen is None:
            randgen = random
        vertices = sample_vertices_from_instance(instance=instance, randgen=randgen)
        if include_depot:
            vertices = [instance.depot] + vertices + [instance.depot]
        return [(x.vertex_id if isinstance(x, routingblocks.Vertex) else x) for x in vertices]

    return build


@pytest.fixture
def random_route_factory(random_raw_route_factory):
    def build(evaluation, instance, randgen=None):
        if randgen is None:
            randgen = random
        return routingblocks.create_route(evaluation, instance,
                                          random_raw_route_factory(instance, randgen, include_depot=False))

    return build


@pytest.fixture
def random_route(instance, random_route_factory, mock_evaluation):
    py_instance, instance = instance
    return py_instance, instance, random_route_factory(mock_evaluation, instance)


@pytest.fixture
def random_routes(instance, random_routes_factory, evaluation_factory):
    vertices = sample_vertices_from_instance(instance)
    return instance, random_routes_factory(evaluation=evaluation_factory(instance), instance=instance,
                                           vertices=vertices,
                                           n_routes=instance.fleet_size)


@pytest.fixture
def random_solution_factory(random_routes_factory):
    def build(instance, evaluation, vertices: List[routingblocks.Vertex] = None,
              n_routes: int = None, randgen=None) -> routingblocks.Solution:
        if n_routes is None:
            n_routes = instance.fleet_size
        return routingblocks.Solution(evaluation, instance,
                                      random_routes_factory(evaluation=evaluation, instance=instance,
                                                            vertices=vertices, n_routes=n_routes,
                                                            randgen=randgen))

    return build


@pytest.fixture
def random_solution(random_routes_factory, mock_evaluation, instance):
    py_instance, instance = instance
    routes = random_routes_factory(evaluation=mock_evaluation, instance=instance,
                                   vertices=sample_vertices_from_instance(instance), n_routes=instance.fleet_size)
    return py_instance, instance, routingblocks.Solution(mock_evaluation, instance, routes)
