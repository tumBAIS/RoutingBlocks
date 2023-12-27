# Copyright (c) 2023 Patrick S. Klein (@libklein)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

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
    # Set resource penalty sufficiently high to ensure that the operator inserts stations until (charge) feasible.
    evaluation.resource_penalty_factor = 1000.
    evaluation.overload_penalty_factor = 0.
    evaluation.time_shift_penalty_factor = 0.

    solution = recreate_solution(evaluation=evaluation, instance=instance)
    route = solution[0]
    assert not route.feasible
    assert route.cost_components[2] > 0
    station_insertion_operator = evrptw.operators.InsertStationOperator(instance)
    ls = evrptw.LocalSearch(instance, evaluation, None, evrptw.FirstImprovementPivotingRule())
    ls.optimize(solution, [station_insertion_operator])
    # Charge penalty should be 0.
    assert route.cost_components[2] == 0.
