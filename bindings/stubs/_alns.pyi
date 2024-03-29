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

class RepairOperator:
    def __init__(self) -> None: ...

    def apply(self, evaluation: Evaluation, solution: Solution, removed_vertex_ids: List[VertexID]) -> None:
        """
        Apply this operator to the given solution. The solution is modified in-place.

        :param evaluation: The evaluation function to use.
        :param solution: The solution to apply the operator to.
        :param removed_vertex_ids: The IDs of the vertices that should be inserted into the solution.
        """
        ...

    def can_apply_to(self, solution: Solution) -> bool:
        """
        Check if this operator can be applied to the given solution.

        :param solution: The solution to check.
        """
        ...

    def name(self) -> str:
        """
        Get the name of this operator.
        """
        ...


class DestroyOperator:
    def __init__(self) -> None: ...

    def apply(self, evaluation: Evaluation, solution: Solution, number_of_vertices_to_remove: int) -> List[
        VertexID]:
        """
        Apply this operator to the given solution. The solution is modified in-place.

        :param evaluation: The evaluation function to use.
        :param solution: The solution to apply the operator to.
        :param number_of_vertices_to_remove: The number of vertices to remove from the solution.
        :return: A list containing the IDs of the vertices that were removed from the solution.
        """
        ...

    def can_apply_to(self, solution: Solution) -> bool:
        """
        Check if this operator can be applied to the given solution.

        :param solution: The solution to check.
        """
        ...

    def name(self) -> str:
        """
        Get the name of this operator.
        """
        ...


class AdaptiveLargeNeighborhood:
    """
    ALNS solver.
    Initially, all operators are assigned equal weights of 1. The weights are then adapted based on the performance of the
    operators in the last period. Upon requesting an update, the weights are recalculated as follows:

    .. math::

        w_{op, new} = \\alpha \\cdot \\frac{s_{op}}{\\max(1, n_{op})} + (1 - \\alpha) \\cdot w_{op, old}

    Where

    * :math:`w_{op, new}` is the updated weight of operator :math:`op`
    * :math:`\\alpha` is the smoothing factor, which determines the importance of an operator's historical performance
    * :math:`s_{op}` is the sum of scores achieved by operator :math:`op` in the last period
    * :math:`n_{op}` is the total number of solutions generated using operator :math:`op` in the last period
    * :math:`w_{op, old}` is the old weight of operator :math:`op`
    """

    def __init__(self, randgen: Random, smoothing_factor: float) -> None:
        """
        :param randgen: Random number generator.
        :param smoothing_factor: Smoothing factor for the adaptive weights. Determines the importance of the historical
        performance of the operators.
        """
        ...

    def adapt_operator_weights(self) -> None:
        """
        Calculate the new weights for the operators based on their respective performance in the last period.
        """
        ...

    def add_destroy_operator(self, destroy_operator: DestroyOperator) -> DestroyOperator:
        """
        Register a new destroy operator. The weight of this operator is initialized to the average weight of all other
        destroy operators.

        :param destroy_operator: The operator to add.
        """
        ...

    def add_repair_operator(self, repair_operator: RepairOperator) -> RepairOperator:
        """
        Register a new repair operator. The weight of this operator is initialized to the average weight of all other
        repair operators.

        :param repair_operator: The operator to add.
        """
        ...

    def collect_score(self, destroy_operator: DestroyOperator, repair_operator: RepairOperator,
                      score: float) -> None:
        """
        Collect the score achieved by a solution generated by the given destroy and repair operators.

        :param destroy_operator: The destroy operator used to generate the solution.
        :param repair_operator: The repair operator used to generate the solution.
        :param score: The score achieved by the solution.
        """
        ...

    def generate(self, evaluation: Evaluation, solution: Solution, number_of_vertices_to_remove: int) -> Tuple[
        DestroyOperator, RepairOperator]:
        """
        Generate a new solution by applying a destroy and repair operator selected based the on the operator weights.
        Modifies the solution in-place.

        :param evaluation: The evaluation function to use.
        :param solution: The solution to generate a new solution from.
        :param number_of_vertices_to_remove: The number of vertices to remove from the solution.
        :return: A tuple containing the destroy and repair operator used to generate the solution.
        """
        ...

    def remove_destroy_operator(self, destroy_operator: DestroyOperator) -> None:
        """
        Remove a destroy operator.

        :param destroy_operator: The operator to remove.
        """
        ...

    def remove_repair_operator(self, repair_operator: RepairOperator) -> None:
        """
        Remove a repair operator.

        :param repair_operator: The operator to remove.
        """
        ...

    def reset_operator_weights(self) -> None:
        """
        Reset the weights of all operators to 1.
        """
        ...

    @property
    def destroy_operators(self) -> Iterator:
        """
        Get an iterator over all registered destroy operators.
        """
        ...

    @property
    def repair_operators(self) -> Iterator:
        """
        Get an iterator over all registered repair operators.
        """
        ...
