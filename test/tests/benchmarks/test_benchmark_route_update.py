from __future__ import annotations

from dataclasses import dataclass
from typing import List

import pytest

import helpers

from fixtures import *

import routingblocks as alns
from routingblocks import niftw


@dataclass
class Label:
    cum_load: float = 0.
    cum_distance: float = 0.
    earliest_arrival: float = 0.
    shifted_earliest_arrival: float = 0.
    latest_time_of_arrival: float = 0.
    residual_charge_in_time: float = 0.
    cum_time_shift: float = 0.
    cum_overcharge: float = 0.
    prev_overcharge: float = 0.
    prev_time_shift: float = 0.


@dataclass
class Vertex:
    earliest_arrival_time: float
    latest_arrival_time: float
    service_time: float
    demand: int
    is_station: bool


@dataclass
class Arc:
    duration: float
    consumption: float
    cost: float


class PyEvaluation:
    def __init__(self, _battery_capacity, _storage_capacity, _replenishment_time):
        self.battery_capacity = _battery_capacity
        self.storage_capacity = _storage_capacity
        self.replenishment_time = _replenishment_time

    def propagate_forward(self, pred_label: Label, pred_vertex: Vertex, vertex: Vertex, arc: Arc) -> Label:
        t_ij = arc.duration + pred_vertex.service_time
        q_ij = arc.consumption
        c_ij = arc.cost

        propagated_label = Label()

        propagated_label.cum_distance = pred_label.cum_distance + c_ij
        propagated_label.cum_load = pred_label.cum_load + vertex.demand

        if pred_vertex.is_station:
            propagated_label.earliest_arrival = max(vertex.earliest_arrival_time,
                                                    pred_label.shifted_earliest_arrival + t_ij) + self.replenishment_time
            propagated_label.residual_charge_in_time = q_ij
        else:
            propagated_label.earliest_arrival = max(vertex.earliest_arrival_time,
                                                    pred_label.shifted_earliest_arrival + t_ij)
            propagated_label.residual_charge_in_time = min(pred_label.residual_charge_in_time,
                                                           self.battery_capacity) + q_ij

        propagated_label.shifted_earliest_arrival = min(propagated_label.earliest_arrival,
                                                        vertex.latest_arrival_time)

        propagated_label.cum_time_shift = pred_label.cum_time_shift + max(
            propagated_label.earliest_arrival - vertex.latest_arrival_time, 0)
        propagated_label.cum_overcharge = pred_label.cum_overcharge + max(
            propagated_label.residual_charge_in_time - self.battery_capacity, 0)

        propagated_label.prev_time_shift = pred_label.cum_time_shift
        propagated_label.prev_overcharge = pred_label.cum_overcharge

        return propagated_label

    def propagate_backward(self, succ_label: Label, succ_vertex: Vertex, vertex: Vertex, arc: Arc) -> Label:
        propagated_label = Label()

        t_ij = arc.duration + vertex.service_time
        q_ij = arc.consumption
        c_ij = arc.cost

        propagated_label.cum_distance = succ_label.cum_distance + c_ij
        propagated_label.cum_load = succ_label.cum_load + vertex.demand

        if succ_vertex.is_station:
            propagated_label.earliest_arrival = min(vertex.latest_arrival_time,
                                                    succ_label.shifted_earliest_arrival - t_ij - self.replenishment_time)
            propagated_label.residual_charge_in_time = q_ij
        else:
            propagated_label.earliest_arrival = min(vertex.latest_arrival_time,
                                                    succ_label.shifted_earliest_arrival - t_ij)
            propagated_label.residual_charge_in_time = min(self.battery_capacity,
                                                           succ_label.residual_charge_in_time) + q_ij

        propagated_label.shifted_earliest_arrival = max(propagated_label.earliest_arrival,
                                                        vertex.earliest_arrival_time)

        propagated_label.cum_time_shift = succ_label.cum_time_shift + max(0,
                                                                          vertex.earliest_arrival_time - propagated_label.earliest_arrival)
        propagated_label.cum_overcharge = succ_label.cum_overcharge + max(0,
                                                                          propagated_label.residual_charge_in_time - self.battery_capacity)

        return propagated_label

    def create_forward_label(self, vertex: Vertex) -> Label:
        l = Label()
        return l

    def create_backward_label(self, vertex: Vertex) -> Label:
        l = Label()
        return l

    def concatenate(self, fwd: Label, bwd: Label, vertex: Vertex) -> float:
        distance = fwd.cum_distance + bwd.cum_distance
        overload = max(fwd.cum_load + bwd.cum_load - vertex.demand - self.storage_capacity, 0)

        additional_time_shift = max(0, fwd.shifted_earliest_arrival - bwd.shifted_earliest_arrival)

        if vertex.is_station:
            additional_overcharge = max(fwd.residual_charge_in_time - self.battery_capacity, 0)
        else:
            additional_overcharge = max(0, fwd.residual_charge_in_time + min(self.battery_capacity,
                                                                             bwd.residual_charge_in_time) - self.battery_capacity)

        time_shift = fwd.cum_time_shift + bwd.cum_time_shift + additional_time_shift
        overcharge = fwd.prev_overcharge + bwd.cum_overcharge + additional_overcharge
        return distance + overload + time_shift + overcharge

    def compute_cost(self, fwd_label: Label) -> float:
        return fwd_label.cum_distance + fwd_label.cum_load + fwd_label.cum_overcharge + fwd_label.cum_time_shift

    def is_feasible(self, fwd_label: Label) -> bool:
        return self.compute_cost(fwd_label) == fwd_label.cum_distance


def run_native_evaluation(route: alns.Route):
    route.update()


def run_python_evaluation(fwd_label: Label, bwd_label: Label, route: List[Vertex], arcs: List[Arc],
                          evaluation: PyEvaluation):
    prev_label = fwd_label
    prev_vertex = route[0]
    for i in range(1, len(route)):
        vertex = route[i]
        arc = arcs[i - 1]
        fwd_label = evaluation.propagate_forward(prev_label, prev_vertex,
                                                 vertex, arc)
        prev_label = fwd_label
        prev_vertex = vertex

    succ_label = bwd_label
    succ_vertex = route[-1]
    for i in reversed(range(0, len(route) - 1)):
        vertex = route[i]
        arc = arcs[i]
        bwd_label = evaluation.propagate_forward(succ_label, succ_vertex,
                                                 vertex, arc)
        succ_label = bwd_label
        succ_vertex = vertex


def convert_route(route: alns.Route, py_instance: helpers.Instance) -> Tuple[List[Vertex], List[Arc]]:
    converted_route: List[helpers.Vertex] = [py_instance.vertices[node.vertex.str_id] for node in route]
    vertices = [Vertex(earliest_arrival_time=v.ready_time, latest_arrival_time=v.due_date,
                       service_time=v.service_time,
                       demand=int(v.demand), is_station=v.is_station) for v in converted_route]
    arcs = []
    for i, j in zip(converted_route, converted_route[1:]):
        py_arc: helpers.Arc = py_instance.arcs[i.vertex_id, j.vertex_id]
        arcs.append(Arc(duration=py_arc.travel_time, consumption=py_arc.consumption, cost=py_arc.cost))

    return vertices, arcs


@pytest.mark.skip
def test_evaluation_benchmark_update_route_r(instance_parser, random_route_factory):
    replenishment_time = 0.
    storage_capacity = 170.
    battery_capacity = 170.

    py_instance, instance = instance_parser('r101_21.txt')

    #### Native evaluation ####
    native_eval = niftw.Evaluation(battery_capacity, storage_capacity, replenishment_time)

    # Create routes for native evaluation
    randgen = random.Random(0)
    route = random_route_factory(native_eval, instance, randgen)
    #### Python evaluation ####
    # Parse route
    py_route_vertices, py_route_arcs = convert_route(route, py_instance)
    py_eval = PyEvaluation(battery_capacity, storage_capacity, replenishment_time)
    initial_fwd_label = py_eval.create_forward_label(py_route_vertices[0])
    initial_bwd_label = py_eval.create_backward_label(py_route_vertices[-1])
    run_python_evaluation(initial_fwd_label, initial_bwd_label, py_route_vertices, py_route_arcs, py_eval)


@pytest.mark.benchmark(group="evaluation")
@pytest.mark.parametrize("evaluation", ['native', 'python'])
def test_evaluation_benchmark_update_route(instance_parser,
                                           random_route_factory, benchmark, evaluation):
    replenishment_time = 0.
    storage_capacity = 170.
    battery_capacity = 170.

    py_instance, instance = instance_parser('r101_21.txt')

    #### Native evaluation ####
    native_eval = niftw.Evaluation(battery_capacity, storage_capacity, replenishment_time)

    # Create routes for native evaluation
    randgen = random.Random(0)
    route = random_route_factory(native_eval, instance, randgen)

    if evaluation == 'native':
        benchmark(run_native_evaluation, route=route)

    #### Python evaluation ####
    # Parse route
    py_route_vertices, py_route_arcs = convert_route(route, py_instance)
    py_eval = PyEvaluation(battery_capacity, storage_capacity, replenishment_time)
    initial_fwd_label = py_eval.create_forward_label(py_route_vertices[0])
    initial_bwd_label = py_eval.create_backward_label(py_route_vertices[-1])
    if evaluation == 'python':
        benchmark(run_python_evaluation, fwd_label=initial_fwd_label, bwd_label=initial_bwd_label,
                  arcs=py_route_arcs, route=py_route_vertices, evaluation=py_eval)
