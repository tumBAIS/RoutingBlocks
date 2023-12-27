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
