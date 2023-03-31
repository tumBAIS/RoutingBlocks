from __future__ import annotations

from dataclasses import dataclass
from typing import List

import pytest

import routingblocks


class MockLabel:
    def __init__(self, state):
        self.state = state

    def __eq__(self, other):
        return self.state == other.state

    def __ne__(self, other):
        return not self.__eq__(other)

    def __repr__(self):
        return str(self.state)

    def __str__(self):
        return str(self.state)


class MockEvaluation(routingblocks.PyConcatenationBasedEvaluation):
    @dataclass
    class ConcatenationCall:
        fwd: MockLabel
        bwd: MockLabel
        vertex: routingblocks.Vertex

        def __str__(self):
            return f"{self.vertex.id}"

    @dataclass
    class ForwardEvaluationCall:
        pred_label: MockLabel
        pred_vertex: routingblocks.Vertex
        vertex: routingblocks.Vertex
        arc: routingblocks.Arc

        def __str__(self):
            return f"{self.pred_vertex.id} -> {self.vertex.id}"

    @dataclass
    class BackwardEvaluationCall:
        succ_label: MockLabel
        succ_vertex: routingblocks.Vertex
        vertex: routingblocks.Vertex
        arc: routingblocks.Arc

        def __str__(self):
            return f"{self.vertex.id} -> {self.succ_vertex.id}"

    @dataclass
    class CreateForwardLabelCall:
        vertex: routingblocks.Vertex

    @dataclass
    class CreateBackwardLabelCall:
        vertex: routingblocks.Vertex

    @dataclass
    class ComputeCostCall:
        fwd: MockLabel

    @dataclass
    class IsFeasibleCall:
        fwd: MockLabel

    @dataclass
    class GetCostComponentsCall:
        fwd: MockLabel

    def __init__(self):
        routingblocks.PyConcatenationBasedEvaluation.__init__(self)
        self.ops = []

    def reset(self):
        self.ops = []

    def propagate_forward(self, pred_label: MockLabel, pred_vertex, vertex, arc) -> MockLabel:
        self.ops.append(MockEvaluation.ForwardEvaluationCall(pred_label, pred_vertex, vertex, arc))
        return MockLabel(hash(('fwd', pred_label.state, pred_vertex.vertex_id, vertex.vertex_id)))

    def propagate_backward(self, succ_label: MockLabel, succ_vertex, vertex, arc) -> MockLabel:
        self.ops.append(MockEvaluation.BackwardEvaluationCall(succ_label, succ_vertex, vertex, arc))
        return MockLabel(hash(('bwd', succ_label.state, vertex.vertex_id, succ_vertex.vertex_id)))

    def create_forward_label(self, vertex) -> MockLabel:
        self.ops.append(MockEvaluation.CreateForwardLabelCall(vertex))
        return MockLabel(hash(('depot_fwd', vertex.vertex_id)))

    def create_backward_label(self, vertex) -> MockLabel:
        self.ops.append(MockEvaluation.CreateBackwardLabelCall(vertex))
        return MockLabel(hash(('depot_bwd', vertex.vertex_id)))

    def concatenate(self, fwd: MockLabel, bwd: MockLabel, vertex: routingblocks.Vertex) -> float:
        self.ops.append(MockEvaluation.ConcatenationCall(fwd, bwd, vertex))
        return 0

    def compute_cost(self, fwd_label: MockLabel) -> float:
        self.ops.append(MockEvaluation.ComputeCostCall(fwd_label))
        return 0

    def is_feasible(self, fwd_label: MockLabel) -> bool:
        self.ops.append(MockEvaluation.IsFeasibleCall(fwd_label))
        return True

    def get_cost_components(self, fwd_label: MockLabel) -> List[float]:
        self.ops.append(MockEvaluation.GetCostComponentsCall(fwd_label))
        return [0]


@pytest.fixture
def mock_evaluation():
    return MockEvaluation()


def assert_forward_propagations(operations, expected_propagations):
    forward_propagations = [x for x in operations if isinstance(x, MockEvaluation.ForwardEvaluationCall)]

    error_message = "Unexpected number of forward propagations:\n" \
                    f"Expected: {expected_propagations}\n" \
                    f"Actual: {[(x.pred_vertex.id, x.vertex.id) for x in forward_propagations]}"

    assert len(forward_propagations) == len(expected_propagations), error_message
    for actual, expected in zip(forward_propagations, expected_propagations):
        assert actual.pred_vertex.id == expected[0], error_message
        assert actual.vertex.id == expected[1], error_message


def assert_concatenations(operations, expected_concatenations):
    concatenations = [x for x in operations if isinstance(x, MockEvaluation.ConcatenationCall)]
    assert len(concatenations) == len(expected_concatenations)
    for actual, expected in zip(concatenations, expected_concatenations):
        assert actual.vertex.id == expected
