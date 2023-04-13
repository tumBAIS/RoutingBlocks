from __future__ import annotations

from typing import Tuple, Callable, Dict, List, TypeVar, Union

from fixtures import *

import routingblocks as evrptw
from routingblocks import niftw


def create_solution(evaluation, instance: evrptw.Instance, routes: List[evrptw.Route]):
    solution = evrptw.Solution(evaluation, instance,
                               [*routes, *[evrptw.Route(evaluation, instance) for _ in
                                           range(instance.fleet_size - len(routes))]])
    return solution


def recreate_solution(evaluation, instance: evrptw.Instance):
    customers = list(instance.customers)
    route_a = evrptw.create_route(evaluation, instance, [x.vertex_id for x in customers])
    return create_solution(evaluation=evaluation, instance=instance,
                           routes=[route_a])


def test_insert_station_operator_search(large_instance):
    py_instance, instance = large_instance
    evaluation = niftw.Evaluation(py_instance.parameters.battery_capacity_time,
                                  py_instance.parameters.capacity, 0.)
    # Set overcharge penalty sufficiently high to ensure that the operator inserts stations until (charge) feasible.
    evaluation.overcharge_penalty_factor = 1000.
    evaluation.overload_penalty_factor = 0.
    evaluation.time_shift_penalty_factor = 0.

    solution = recreate_solution(evaluation=evaluation, instance=instance)
    route = solution[0]
    assert not route.feasible
    assert route.cost_components[2] > 0
    station_insertion_operator = evrptw.operators.InsertStationOperator(instance)
    ls = evrptw.LocalSearch(instance, evaluation, None)
    ls.optimize(solution, [station_insertion_operator])
    # Charge penalty should be 0.
    assert route.cost_components[2] == 0.
