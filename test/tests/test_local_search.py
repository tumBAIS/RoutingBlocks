from __future__ import annotations

import itertools
import math
import random
import time
from collections import defaultdict
from pathlib import Path
from typing import Tuple, Callable, Dict, List, Iterable

import pytest

import helpers

from fixtures import *

try:
    import routingblocks as evrptw
    from routingblocks import adptw


    class MockLSOperator(evrptw.Operator):
        def __init__(self):
            evrptw.Operator.__init__(self)
            self.ops = []

        def finalize_search(self) -> None:
            self.ops.append("finalize_search")

        def find_next_improving_move(self, arg0: evrptw.Evaluation, arg2, arg3):
            self.ops.append("find_next_improving_move")
            return None

        def prepare_search(self, arg0) -> None:
            self.ops.append("prepare_search")

        def reset(self) -> None:
            self.ops = []

except ModuleNotFoundError:
    pass


@pytest.fixture
def local_search_and_solution(instance, random_solution_factory, randgen):
    py_instance: helpers.Instance = instance[0]
    instance: evrptw.Instance = instance[1]
    evaluation = adptw.Evaluation(py_instance.parameters.battery_capacity_time, py_instance.parameters.capacity)
    solution = random_solution_factory(instance=instance, evaluation=evaluation)
    return evrptw.LocalSearch(instance, evaluation, None), solution


def test_local_search_optimize_inplace(local_search_and_solution):
    local_search, solution = local_search_and_solution
    route = solution[0]
    local_search.optimize(solution, [])
    assert solution[0] is route


@pytest.mark.skip
def test_local_search_custom_operator(local_search_and_solution):
    local_search, solution = local_search_and_solution
    operator = MockLSOperator()

    local_search.optimize(solution, [operator])
    assert operator.ops == ["prepare_search", "find_next_improving_move", "finalize_search"]


@pytest.mark.skip
def test_local_search_custom_operators_lifetime(local_search_and_solution):
    local_search, solution = local_search_and_solution
    local_search.optimize(solution, [MockLSOperator()])


def test_neighborhood_iterator(random_solution):
    *_, solution = random_solution
    expected_moves = set()
    for i, route_i in enumerate(solution):
        for u, node_u in enumerate(route_i):
            for j, route_j in enumerate(solution):
                for v, node_v in enumerate(route_j):
                    expected_moves.add((node_u, node_v))

    actual_moves = set((x.origin_node, x.target_node) for x in evrptw.iter_neighborhood(solution))
    assert actual_moves == expected_moves
