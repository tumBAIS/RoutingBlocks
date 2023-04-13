class RepairOperator:
    def __init__(self) -> None: ...

    def apply(self, evaluation: Evaluation, solution: Solution, removed_vertex_ids: List[VertexID]) -> None: ...

    def can_apply_to(self, solution: Solution) -> bool: ...

    def name(self) -> str: ...


class DestroyOperator:
    def __init__(self) -> None: ...

    def apply(self, evaluation: Evaluation, solution: Solution, number_of_vertices_to_remove: int) -> List[
        VertexID]: ...

    def can_apply_to(self, solution: Solution) -> bool: ...

    def name(self) -> str: ...


class AdaptiveLargeNeighborhood:
    def __init__(self, randgen: Random, smoothing_factor: float) -> None: ...

    def adapt_operator_weights(self) -> None: ...

    def add_destroy_operator(self, destroy_operator: DestroyOperator) -> DestroyOperator: ...

    def add_repair_operator(self, repair_operator: RepairOperator) -> RepairOperator: ...

    def collect_score(self, destroy_operator: DestroyOperator, repair_operator: RepairOperator,
                      score: float) -> None: ...

    def generate(self, evaluation: Evaluation, solution: Solution, number_of_vertices_to_remove: int) -> Tuple[
        DestroyOperator, RepairOperator]: ...

    def remove_destroy_operator(self, destroy_operator: DestroyOperator) -> None: ...

    def remove_repair_operator(self, repair_operator: RepairOperator) -> None: ...

    def reset_operator_weights(self) -> None: ...

    @property
    def destroy_operators(self) -> Iterator: ...

    @property
    def repair_operators(self) -> Iterator: ...