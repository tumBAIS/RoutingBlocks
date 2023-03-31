from __future__ import annotations

import itertools
import random
from collections import defaultdict
from pathlib import Path
from typing import Tuple, Callable, Dict, List, Iterable, Union

import pytest

import helpers

from fixtures import *

import sys

try:
    import routingblocks as evrptw
except ModuleNotFoundError:
    pass


def create_segments(sequence):
    segments = []
    prev_segment_end = 1
    while True:
        next_segment_end = prev_segment_end + random.randint(1, len(sequence) - prev_segment_end - 1)

        segments.append((prev_segment_end, next_segment_end))

        prev_segment_end = next_segment_end
        if prev_segment_end == len(sequence) - 1:
            break
    return segments


try:
    class MockLabel:
        def __init__(self, msg):
            self.msg = msg

        def __repr__(self):
            return self.msg

        def __str__(self):
            return self.msg


    class MockEvaluation(evrptw.PyConcatenationBasedEvaluation):
        def __init__(self):
            evrptw.PyConcatenationBasedEvaluation.__init__(self)

        def propagate_forward(self, *args) -> MockLabel:
            return MockLabel("forward")

        def propagate_backward(self, *args) -> MockLabel:
            return MockLabel("backward")

        def create_forward_label(self, *args) -> MockLabel:
            return MockLabel("forward_created")

        def create_backward_label(self, *args) -> MockLabel:
            return MockLabel("backward_created")

        def concatenate(self, *args) -> float:
            return 0

        def compute_cost(self, arg0) -> float:
            return 0
except:
    pass


@pytest.mark.skip
def test_evaluation_lifetime(instance):
    _, instance = instance
    eval = MockEvaluation()
    route = evrptw.Route(eval, instance)
    # Evaluation should still work even after freeing the object on the python side
    route.update()
    del eval
    route.update()


@pytest.mark.parametrize('execution_number', range(10))
@pytest.mark.skip
def test_evaluation_concatenation(instance: evrptw.Instance,
                                  execution_number):
    vertex_ids = [x.id for x in [*instance.customers, *instance.stations]]
    random.shuffle(vertex_ids)
    route = evrptw.create_route(instance, vertex_ids)
    vertex_ids = [instance.depot, *vertex_ids, instance.depot]

    print()
    print([x.vertex_id for x in route])

    eval = LoggingEvaluation(
        evrptw.create_adptw_evaluation(instance.vehicle_battery_capacity, instance.vehicle_storage_capacity))
    evrptw.set_evaluation(eval)

    segments = create_segments(vertex_ids)
    route_segments = [evrptw.RouteSegment(route, seg_beg, seg_end) for seg_beg, seg_end in segments]
    for i, ((fwd_valid_seg_beg, _), fwd_valid_route_segment) in enumerate(zip(segments, route_segments)):
        for j, ((bwd_valid_seg_beg, _), bwd_valid_route_segment) in enumerate(zip(segments, route_segments)):
            if j <= i:
                continue
            segments_between = segments[i:j]
            route_segments_between = route_segments[i:j]

            print(segments[i], segments_between, segments[j])

            # Expected cost is just the route's cost
            expected_cost = route.cost

            print(f"Concatenating ..., {route[fwd_valid_seg_beg - 1].vertex_id}] - ", end="")
            for seg in segments_between:
                print("[", end="")
                print([route[index].vertex_id for index in range(seg[0], seg[1])], end="")
                print("] - ", end="")
            print(f"[{route[bwd_valid_seg_beg].vertex_id}, ...")
            actual_cost = evrptw.concatenate(instance, route, fwd_valid_seg_beg - 1, route_segments_between, route,
                                             bwd_valid_seg_beg)
            assert actual_cost == expected_cost
