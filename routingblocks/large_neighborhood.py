from ._routingblocks import AdaptiveLargeNeighborhood, Random, DestroyOperator, RepairOperator, Evaluation, Solution
from typing import Tuple, Iterator


class LargeNeighborhood:
    """
    LNS solver.

    The solver uses a destroy and repair operator to iteratively generate new solutions. The destroy operator is used to
    remove a number of vertices from the solution, while the repair operator is used to re-insert the removed vertices
    into the solution.

    Destroy and repair operators are selected uniformly from a pool of registered operators.
    """

    def __init__(self, randgen: Random) -> None:
        """
        :param randgen: Random number generator.
        """
        self._alns = AdaptiveLargeNeighborhood(randgen, 0.)

    def add_destroy_operator(self, destroy_operator: DestroyOperator) -> DestroyOperator:
        """
        Register a new destroy operator. The weight of this operator is initialized to the average weight of all other
        destroy operators.

        :param destroy_operator: The operator to add.
        """
        return self._alns.add_destroy_operator(destroy_operator)

    def add_repair_operator(self, repair_operator: RepairOperator) -> RepairOperator:
        """
        Register a new repair operator. The weight of this operator is initialized to the average weight of all other
        repair operators.

        :param repair_operator: The operator to add.
        """
        return self._alns.add_repair_operator(repair_operator)

    def generate(self, evaluation: Evaluation, solution: Solution, number_of_vertices_to_remove: int) -> Tuple[
        DestroyOperator, RepairOperator]:
        """
        Generate a new solution by applying a destroy and repair operator selected randomly.
        Modifies the solution in-place.

        :param evaluation: The evaluation function to use.
        :param solution: The solution to generate a new solution from.
        :param number_of_vertices_to_remove: The number of vertices to remove from the solution.
        :return: A tuple containing the destroy and repair operator used to generate the solution.
        """
        return self._alns.generate(evaluation, solution, number_of_vertices_to_remove)

    def remove_destroy_operator(self, destroy_operator: DestroyOperator) -> None:
        """
        Remove a destroy operator.

        :param destroy_operator: The operator to remove.
        """
        return self._alns.remove_destroy_operator(destroy_operator)

    def remove_repair_operator(self, repair_operator: RepairOperator) -> None:
        """
        Remove a repair operator.

        :param repair_operator: The operator to remove.
        """
        return self._alns.remove_repair_operator(repair_operator)

    @property
    def destroy_operators(self) -> Iterator:
        """
        Get an iterator over all registered destroy operators.
        """
        return self._alns.destroy_operators

    @property
    def repair_operators(self) -> Iterator:
        """
        Get an iterator over all registered repair operators.
        """
        return self._alns.repair_operators
