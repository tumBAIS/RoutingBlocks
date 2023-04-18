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
    """
    Interface for implementing custom moves.
    """

    def __init__(self) -> None: ...

    def apply(self, instance: Instance, solution: Solution) -> None:
        """
        Applies the move to the solution.

        :param Instance instance: The instance.
        :param Solution solution: The solution to be improved.
        :return:
        """
        ...

    def get_cost_delta(self, evaluation: Evaluation, instance: Instance, solution: Solution) -> float:
        """
        Returns the cost delta of the move according to the passed evaluation function.

        :param evaluation: The evaluation function to use.
        :param instance: The instance.
        :param solution: The solution.
        :return:
        """
        ...


class LocalSearchOperator:
    """
    Interface for implementing custom local search operators.
    """

    def __init__(self) -> None: ...

    def finalize_search(self) -> None:
        """
        Called after the search for improving moves has been completed.
        """
        ...

    def find_next_improving_move(self, evaluation: Evaluation, solution: Solution,
                                 last_evaluated_move: Move) -> Move:
        """
        Finds the next improving move. Returns None if no improving move is found.
        To avoid looping forever, this method should pick up the search where it left off, i.e., at last_evaluated_move.

        :param Evaluation evaluation: The evaluation function to use.
        :param Solution solution: The solution to be improved.
        :param Move last_evaluated_move: The last move that was evaluated. Note that this corresponds to the last Move this operator returned. None if no move has been evaluated yet.
        :return: The next improving move. None if no improving move is found.
        """
        ...

    def prepare_search(self, solution: Solution) -> None:
        """
        Called before the search for improving moves is started.

        :param Solution solution: The solution to be improved.
        """
        ...


class PivotingRule:
    """
    Interface for implementing custom pivoting rules.
    """

    def __init__(self) -> None:
        ...

    def select_move(self, solution: Solution) -> Optional[Move]:
        """
        Return the move to be applied to the solution.Returns none if no move is found.

        :param Solution solution: The solution to be improved.
        :return: The move to be applied to the solution.
        :rtype: Optional[Move]
        """
        ...

    def continue_search(self, found_improving_move: Move, delta_cost: float, solution: Solution) -> bool:
        """
        Determine if the search should continue or terminate.

        :param Move found_improving_move: The move found to be improving.
        :param float delta_cost: The(exact) cost difference between the current solution and the solution after applying the move.
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
    The k - best improvement pivoting rule selects best out of the first k improving moves found during the search
    for improving moves. It terminates the search as soon as the k - th improving move is found.

    """

    def __init__(self, k: int) -> None:
        """
        :param int k: The number of improving moves to consider.
        """
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

    def optimize(self, solution: Solution, operators: List[LocalSearchOperator]) -> None:
        """
        Searches the neighborhood of the solution for improving moves and applies them until no further improvement is possible.
        The neighborhood is defined by the passed operators. Modifies the passed solution in-place.

        :param Solution solution: The solution to be improved.
        :param List[LocalSearchOperator] operators: The operators to use for searching the neighborhood.
        :return:
        """
        ...


class QuadraticNeighborhoodIterator:

    def __init__(self) -> None: ...

    def iter_neighborhood(solution: Solution) -> Iterator: ...
