from typing import Optional


class ArcSet:
    def __init__(self, number_of_vertices: int) -> None: ...

    def forbid_arc(self, origin_vertex_id: VertexID, target_vertex_id: VertexID) -> None: ...

    def include_arc(self, origin_vertex_id: VertexID, target_vertex_id: VertexID) -> None: ...

    def includes_arc(self, origin_vertex_id: VertexID, target_vertex_id: VertexID) -> bool: ...


class GeneratorArc:
    @overload
    def __init__(self, solution: Solution, origin_route_index: int, origin_node_position: int, target_route_index: int,
                 target_node_position: int) -> None: ...

    @overload
    def __init__(self, solution: Solution, origin_location: NodeLocation, target_location: NodeLocation) -> None: ...

    @property
    def origin_node(self) -> Node: ...

    @property
    def origin_route(self) -> Route: ...

    @property
    def target_node(self) -> Node: ...

    @property
    def target_route(self) -> Route: ...


class Move:
    def __init__(self) -> None: ...

    def apply(self, instance: Instance, solution: Solution) -> None: ...

    def get_cost_delta(self, evaluation: Evaluation, instance: Instance, solution: Solution) -> float: ...


class LocalSearchOperator:
    def __init__(self) -> None: ...

    def finalize_search(self) -> None: ...

    def find_next_improving_move(self, evaluation: Evaluation, solution: Solution,
                                 last_evaluated_move: Move) -> Move: ...

    def prepare_search(self, solution: Solution) -> None: ...


class PivotingRule:
    """
    The pivoting rule interface.
    """

    def __init__(self) -> None:
        ...

    def select_move(self, solution: Solution) -> Optional[Move]:
        """
        Return the move to be applied to the solution. Returns none if no move is found.

        :param Solution solution: The solution to be improved.
        :return: The move to be applied to the solution.
        :rtype: Optional[Move]
        """
        ...

    def continue_search(self, found_improving_move: Move, delta_cost: float, solution: Solution) -> bool:
        """
        Determine if the search should continue or terminate.

        :param Move found_improving_move: The move found to be improving.
        :param float delta_cost: The (exact) cost difference between the current solution and the solution after applying the move.
        :param Solution solution: The solution the move should be applied to.
        :return: True if the search should continue, False otherwise.
        """
        ...


class BestImprovementPivotingRule(PivotingRule):
    """
    The best improvement pivoting rule selects the best improving move found during the search for improving moves.
    It never terminates the search prematurely.
    """
    ...


class KBestImprovementPivotingRule(PivotingRule):
    """
    The k-best improvement pivoting rule selects best out of the first k improving moves found during the search for improving moves.
    It terminates the search as soon as the k-th improving move is found.
    """

    def __init__(self, k: int) -> None:
        """
        Configures the number of improving moves to consider.

        :param int k: The number of improving moves to consider.
        """
        ...

    ...


class FirstImprovementPivotingRule(PivotingRule):
    """
    The first improvement pivoting rule selects the first improving move found during the search for improving moves.
    It terminates the search as soon as the first improving move is found.
    """
    ...


class LocalSearch:
    """
    This class implements a customizable local search algorithm.


    """

    def __init__(self, instance: Instance, evaluation: Evaluation, exact_evaluation: Optional[Evaluation],
                 pivoting_rule: PivotingRule) -> None: ...

    def optimize(self, solution: Solution, operator: List[LocalSearchOperator]) -> None: ...


class QuadraticNeighborhoodIterator:
    def __init__(self) -> None: ...


def iter_neighborhood(solution: Solution) -> Iterator: ...
