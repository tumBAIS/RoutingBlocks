from typing import Any, List, Tuple

AnyForwardLabel = Any
AnyBackwardLabel = Any

class Evaluation:
    def __init__(self) -> None: ...

    def compute_cost(self, label: AnyForwardLabel) -> cost_t: ...

    def evaluate(self, instance: Instance,
                 segments: List[List[Tuple[Vertex, AnyForwardLabel, AnyBackwardLabel]]]) -> cost_t: ...

    def create_backward_label(self, vertex: Vertex) -> AnyBackwardLabel: ...

    def create_forward_label(self, vertex: Vertex) -> AnyForwardLabel: ...

    def get_cost_components(self, label: AnyForwardLabel) -> List[resource_t]: ...

    def is_feasible(self, label: AnyForwardLabel) -> bool: ...

    def propagate_backward(self, pred_label: AnyForwardLabel, pred_vertex: Vertex, vertex: Vertex,
                           arc: Arc) -> AnyBackwardLabel: ...

    def propagate_forward(self, succ_label: AnyBackwardLabel, succ_vertex: Vertex, vertex: Vertex,
                          arc: Arc) -> AnyForwardLabel: ...


class PyEvaluation(Evaluation):
    ...


class PyConcatenationBasedEvaluation(Evaluation):
    def __init__(self) -> None: ...

    def concatenate(self, fwd: AnyForwardLabel, bwd: AnyBackwardLabel, vertex: Vertex) -> cost_t: ...


@overload
def evaluate_insertion(evaluation: Evaluation, instance: Instance, route: Route, after_position: VertexID,
                       vertex_id: VertexID) -> cost_t: ...


@overload
def evaluate_insertion(evaluation: Evaluation, instance: Instance, route: Route, after_position: VertexID,
                       vertex: Vertex) -> cost_t: ...


@overload
def evaluate_insertion(evaluation: Evaluation, instance: Instance, route: Route, after_position: VertexID,
                       node: Node) -> cost_t: ...


def evaluate_splice(evaluation: Evaluation, instance: Instance, route: Route, forward_segment_end_pos: int,
                    backward_segment_begin_pos: int) -> cost_t: ...
