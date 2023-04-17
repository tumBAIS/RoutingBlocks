from __future__ import annotations

from itertools import islice

import pytest
from typing import Optional

import helpers
import routingblocks

from fixtures import *

from routingblocks import adptw


class MockLSOperator(routingblocks.LocalSearchOperator):
    def __init__(self):
        routingblocks.LocalSearchOperator.__init__(self)
        self.ops = []

    def finalize_search(self) -> None:
        self.ops.append("finalize_search")

    def find_next_improving_move(self, arg0: routingblocks.Evaluation, arg2, arg3):
        self.ops.append("find_next_improving_move")
        return None

    def prepare_search(self, arg0) -> None:
        self.ops.append("prepare_search")

    def reset(self) -> None:
        self.ops = []


class MockPivotingRule(routingblocks.PivotingRule):
    def __init__(self, always_abort: bool = True):
        routingblocks.PivotingRule.__init__(self)
        self.always_abort = always_abort
        self.ops = []

    def continue_search(self, found_improving_move: routingblocks.Move, delta_cost: float,
                        solution: routingblocks.Solution) -> bool:
        self.ops.append("continue_search")
        return not self.always_abort

    def select_move(self, solution: routingblocks.Solution) -> Optional[routingblocks.Move]:
        self.ops.append("select_move")
        return None


class BestImprovementWithBlinksPivotingRule(routingblocks.PivotingRule):
    def __init__(self, blink_probability: float, randgen: helpers.RandomGenerator):
        routingblocks.PivotingRule.__init__(self)
        self._blink_probability = blink_probability
        self._randgen = randgen
        self._best_move = None
        self._best_delta_cost = -1e-2

    def continue_search(self, found_improving_move: routingblocks.Move, delta_cost: float,
                        solution: routingblocks.Solution) -> bool:
        return self._randgen.random() < self._blink_probability

    def select_move(self, solution: routingblocks.Solution) -> Optional[routingblocks.Move]:
        move = self._best_move
        self._best_move = None
        self._best_delta_cost = -1e-2
        return move


@pytest.mark.parametrize("always_abort,expected_operations", [
    (True, ["continue_search", "select_move"]),
    (False, ["continue_search", "continue_search", "select_move"])
])
def test_local_search_custom_pivot_rule(instance, always_abort, expected_operations):
    py_instance: helpers.Instance = instance[0]
    instance: routingblocks.Instance = instance[1]
    evaluation = adptw.Evaluation(py_instance.parameters.battery_capacity_time, py_instance.parameters.capacity)

    route = routingblocks.create_route(evaluation, instance,
                                       [station.vertex_id for station in islice(instance.stations, 1, 3)])
    solution = routingblocks.Solution(evaluation, instance, [route])
    assert solution.cost > 0
    # Should find exactly two moves
    operator = routingblocks.operators.RemoveStationOperator(instance)
    pivoting_rule = MockPivotingRule(always_abort=always_abort)

    local_search = routingblocks.LocalSearch(instance, evaluation, None, pivoting_rule)

    local_search.optimize(solution, [operator])
    assert pivoting_rule.ops == expected_operations


@pytest.fixture
def local_search_and_solution(instance, random_solution_factory, randgen):
    py_instance: helpers.Instance = instance[0]
    instance: routingblocks.Instance = instance[1]
    evaluation = adptw.Evaluation(py_instance.parameters.battery_capacity_time, py_instance.parameters.capacity)
    solution = random_solution_factory(instance=instance, evaluation=evaluation)
    pivoting_rule = routingblocks.BestImprovementPivotingRule()
    return routingblocks.LocalSearch(instance, evaluation, None, pivoting_rule), solution


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

    actual_moves = set((x.origin_node, x.target_node) for x in routingblocks.iter_neighborhood(solution))
    assert actual_moves == expected_moves
