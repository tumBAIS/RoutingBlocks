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

import time
from typing import Tuple, Callable, Dict, List, Iterable, Optional

import pytest

import helpers
import routingblocks

from fixtures import *

import sys

try:
    import routingblocks as evrptw
    from routingblocks import adptw


    class MockLabel:
        def __init__(self, msg: str):
            self.msg = msg


    class MockPropagator(evrptw.Propagator):
        def __init__(self):
            evrptw.Propagator.__init__(self)
            self._ops = []

        def prepare(self, arg0: List[int]) -> None:
            pass

        def propagate(self, arg0: MockLabel, arg1: adptw.Vertex, arg2: adptw.Vertex, arg3: adptw.Arc) -> Optional[
            MockLabel]:
            self._ops.append('propagate')
            return None

        def cheaper_than(self, arg0: MockLabel, arg1: MockLabel) -> bool:
            self._ops.append('cheaper_than')
            return False

        def order_before(self, arg0: MockLabel, arg1: MockLabel) -> bool:
            self._ops.append('order_before')
            return False

        def is_final_label(self, arg0: MockLabel) -> bool:
            self._ops.append('is_final_label')
            return False

        def extract_path(self, arg0: MockLabel) -> List[int]:
            self._ops.append('extract_path')
            return []

        def dominates(self, arg0: MockLabel, arg1: MockLabel) -> bool:
            self._ops.append('dominates')
            return False

        def create_root_label(self) -> MockLabel:
            return MockLabel("root")
except ModuleNotFoundError:
    pass


def create_facility_placement_optimizer(instance: Tuple):
    py_instance, instance = instance
    return adptw.FacilityPlacementOptimizer(instance, py_instance.parameters.battery_capacity_time)


def test_facility_placement_optimizer_route_with_existing_stations(instance):
    facility_placement_optimizer = create_facility_placement_optimizer(instance)
    _, adptw_instance = instance

    stations = list(adptw_instance.stations)
    unoptimized_vertex_ids = [0, stations[0].vertex_id, 0]
    optimized_vertex_ids = facility_placement_optimizer.optimize(unoptimized_vertex_ids)
    assert optimized_vertex_ids == [adptw_instance.depot.vertex_id, adptw_instance.depot.vertex_id]


def test_facility_placement_optimizer_empty_route(instance):
    facility_placement_optimizer = create_facility_placement_optimizer(instance)

    unoptimized_vertex_ids = [0, 0]
    optimized_vertex_ids = facility_placement_optimizer.optimize(unoptimized_vertex_ids)
    assert optimized_vertex_ids == unoptimized_vertex_ids


def test_facility_placement_optimizer_feasible_route(instance):
    facility_placement_optimizer = create_facility_placement_optimizer(instance)
    _, adptw_instance = instance
    customers = list(adptw_instance.customers)

    unoptimized_vertex_ids = [0, customers[0].vertex_id, 0]
    optimized_vertex_ids = facility_placement_optimizer.optimize(unoptimized_vertex_ids)
    assert optimized_vertex_ids == unoptimized_vertex_ids


def test_facility_placement_optimizer_routes(large_instance):
    py_instance: helpers.Instance
    py_instance, instance = large_instance
    customers = list(instance.customers)
    stations = list(instance.stations)

    evaluation = adptw.Evaluation(py_instance.parameters.battery_capacity_time,
                                  py_instance.parameters.capacity)

    # Generate test routes
    total_routes_considered = 0
    start_time = time.process_time()
    timeout = lambda: (time.process_time() - start_time) >= 30
    while True:
        picked_customers = random.choices([x.vertex_id for x in customers], k=random.randint(1, 10))

        local_search = evrptw.LocalSearch(instance, evaluation, None, evrptw.FirstImprovementPivotingRule())
        sol = evrptw.Solution(evaluation, instance,
                              [evrptw.create_route(evaluation, instance, picked_customers),
                               *(evrptw.Route(evaluation, instance) for _ in range(instance.fleet_size - 1))])
        local_search.optimize(sol, [routingblocks.operators.SwapOperator_0_1(instance, None)])
        for r in sol:
            if r.empty:
                continue
            facility_placement_optimizer = adptw.FacilityPlacementOptimizer(instance,
                                                                            py_instance.parameters.battery_capacity_time)
            labelled_route = facility_placement_optimizer.optimize([x.vertex_id for x in r])
            labelled_route = evrptw.create_route(evaluation, instance, labelled_route[1:-1])
            assert labelled_route.cost <= r.cost
            total_routes_considered += 1
            if timeout() or total_routes_considered >= 100:
                return


def test_facility_placement_optimizer_custom_propagator(adptw_instance):
    facility_placement_optimizer = evrptw.FacilityPlacementOptimizer(adptw_instance, MockPropagator())
    route = [0, 1, 0]
    facility_placement_optimizer.optimize(route)
