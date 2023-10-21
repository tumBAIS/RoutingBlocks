Local search
============

Local search is a core component of almost every state-of-the-art metaheuristic designed for vehicle routing problems.
RoutingBlocks provides a :ref:`local search solver <local_search_solver>`, a set of local search operators, and a set of :ref:`pivoting rules <pivoting_rules>`
that can be used to implement customized local search procedures. It is possible to implement custom local search operators and pivoting rules.

.. _local_search_solver:

Local search solver
-------------------

The local search solver optimizes a given solution by applying a set of local search operators until no further
improvement is possible. Operators model iterators over the neighborhood of a solution. Specifically, on each invocation, an operator generates the (next) improving :py:class:`routingblocks.Move`. Moves provide apply and evaluation interfaces, that allow to apply the move to a solution and to evaluate the resulting solution, respectively.

The solver can be configured with a pivoting rule (e.g. :py:class:`routingblocks.BestImprovementPivotingRule`, :py:class:`routingblocks.KBestImprovementPivotingRule`, or :py:class:`routingblocks.FirstImprovementPivotingRule`) to select the next move to be applied to the solution. The following flowchart illustrates the control flow of the local search solver:

.. mermaid::
    :caption: Control flow of the local search solver
    :align: center

        flowchart TD
            A["optimize(..., operators)"] --> MAINLOOP["Find improving moves"]

            MAINLOOP --> ITER["Explore neighborhoods"]

            ITER --> |"operator in operators"| PREPARE["operator.prepare_search(...)"]
            PREPARE --> FIND_IMPROV["operator.find_next_improving_move(...)"]

            FIND_IMPROV --> |Move is not None| VALIDATE["validate move"]
            FIND_IMPROV -->|Move is None| FINALIZE["operator.finalize_search(...)"]

            VALIDATE -->|Move is None| FIND_IMPROV
            VALIDATE --> |Move is not None| PIVOT["pivoting_rule.continue_search(Move, ...)"]

            PIVOT --> |false| FINALIZE
            PIVOT --> |true| FIND_IMPROV

            FINALIZE --> |"operators remain and search was not aborted"| ITER
            FINALIZE --> |"otherweise"| SELECT_MOVE["pivoting_rule.select_move(...)"]
            SELECT_MOVE --> |"Move is not None"| APPLY["Apply move"]

            APPLY --> |Look for next improving move| MAINLOOP
            SELECT_MOVE --> |"Move is None"| ABORT["Terminate local search"]

The primary loop of the local search (*Find improving moves*) algorithm can be found in :py:meth:`routingblocks.LocalSearch.optimize`. In each iteration, the loop identifies an improvement by investigating the neighboring solutions of the current one, and applies the selected move. The loop concludes when no further improvements are discovered.

A nested loop is utilized to explore neighborhoods (*Explore neighborhoods*), iterating over all operators provided in routingblocks.LocalSearch.optimize to examine the current solution's neighboring solutions for each operator. At this point, the :py:meth:`routingblocks.LocalSearchOperator.prepare_search` method initializes the operator. Following that, the :py:meth:`routingblocks.LocalSearchOperator.find_next_improving_move` method is invoked to locate the subsequent improvement. If no improvement is found, the :py:meth:`routingblocks.LocalSearchOperator.finalize_search` method finalizes the operator. If an improvement is discovered, it is then validated using the exact evaluation given to the solver. The search for improvements continues if the move is deemed invalid. If valid, the pivoting rule determines whether the search should proceed or end.


After the nested loop concludes — either due to all operators being exhausted or the pivoting rule deciding to terminate the search — the :py:meth:`routingblocks.PivotingRule.select_move` method selects the following move to be implemented on the solution. If no move is chosen, the local search ends. If a move is selected, it is applied to the solution, and the primary loop restarts.

The interface of the local search solver is as follows:

.. autoapiclass:: routingblocks.LocalSearch
   :members:
   :undoc-members:

Operators
---------

Operators generate moves that improve the passed solution. Moves (cf. :py:class:`routingblocks.Move`) provide apply and evaluation interfaces, that allow to apply the move to a solution and to evaluate the resulting solution, respectively.

The routingblocks package provides a set of local search operators:

.. autoclass:: routingblocks.operators.InsertStationOperator

.. autoclass:: routingblocks.operators.RemoveStationOperator

.. autoclass:: routingblocks.operators.InterRouteTwoOptOperator

RoutingBlocks provies a set of generic swap operators that implement relocate and exchange moves.
The operators follow the following naming convention:
``SwapOperator_<i>_<j>`` generates all moves that swap segments of length ``i`` with segments of length ``j``.
The operator considers Inter- and Intra-route moves. Each operator can be configured to explore a granular neighborhood,
i.e. to consider only a subset of arcs, by passing a :py:class:`routingblocks.ArcSet` to the constructor.

The following operators are available:

.. autoclass:: routingblocks.operators.SwapOperator_0_1

.. autoclass:: routingblocks.operators.SwapOperator_0_2

.. autoclass:: routingblocks.operators.SwapOperator_0_3

.. autoclass:: routingblocks.operators.SwapOperator_1_1

.. autoclass:: routingblocks.operators.SwapOperator_1_2

.. autoclass:: routingblocks.operators.SwapOperator_1_3

.. autoclass:: routingblocks.operators.SwapOperator_2_1

.. autoclass:: routingblocks.operators.SwapOperator_2_2

.. autoclass:: routingblocks.operators.SwapOperator_2_3

.. autoclass:: routingblocks.operators.SwapOperator_3_1

.. autoclass:: routingblocks.operators.SwapOperator_3_2

.. autoclass:: routingblocks.operators.SwapOperator_3_3

.. _local_search_custom_operators:

Custom Local Search Operators
-----------------------------

Implementing a custom local search operators requires to implement two classes. One that extends the :py:class:`routingblocks.LocalSearchOperator`, and one that extends the :py:class:`routingblocks.Move` interface.
The example below implements a custom operator that inserts depot visits, splitting routes:

.. code-block:: python

    class SplitRouteMove(rb.Move):
        """
        Splits a route into two routes. The passed location will be the first node of the second route.
        """

        def __init__(self, location: rb.NodeLocation):
            rb.Move.__init__(self)
            self.location = location

        def apply(self, instance: rb.Instance, solution: rb.Solution) -> None:
            # Create a new route
            solution.add_route()
            new_route_index = len(solution) - 1

            # Swap the segment [location.position, end] of the route to be split with an empty segment of the new route
            solution.exchange_segment(self.location.route, self.location.position, len(solution[self.location.route]) - 1,
                                      new_route_index, 1, 1)

        def get_cost_delta(self, evaluation: rb.Evaluation, instance: rb.Instance,
                           solution: rb.Solution) -> float:
            split_route = solution[self.location.route]
            cost_of_first_route_after_split = rb.evaluate_splice(evaluation, instance, split_route,
                                                                 self.location.position, len(split_route) - 1)
            cost_of_second_route_after_split = rb.evaluate_splice(evaluation, instance, split_route,
                                                                  1, self.location.position)

            original_route_cost = solution[self.location.route].cost
            return cost_of_first_route_after_split + cost_of_second_route_after_split - original_route_cost


    class SplitRouteOperator(rb.LocalSearchOperator):
        def __init__(self, instance: rb.Instance):
            rb.LocalSearchOperator.__init__(self)
            self.instance = instance

        def _increment_location(self, solution: rb.Solution, location: rb.NodeLocation):
            """
            Increments the given location to the next possible split location. Modifies the passed location in-place.
            Returns None if no further splits are possible.
            :param solution: The solution to be split
            :param location: The location to be incremented
            :return: The incremented location or None if the solution is exhausted
            """
            location.position += 1
            # Move to the next route if the current one is exhausted
            if location.position > len(solution[location.route]) - 1:
                location.route += 1
                location.position = 1
            # No further splits possible
            if location.route >= len(solution):
                return None
            return location

        def _recover_from_move(self, solution: rb.Solution, move: Optional[SplitRouteMove]) -> Optional[rb.NodeLocation]:
            """
            Recovers the state of the search from the given move.
            """
            # If no move was given, start at the beginning
            if move is None:
                return rb.NodeLocation(0, 1)

            # Otherwise continue at the next location
            next_location = self._increment_location(solution, copy.copy(move.location))
            return next_location

        def finalize_search(self) -> None:
            # No cleanup needed
            pass

        def prepare_search(self, solution: rb.Solution) -> None:
            # No preparation needed
            pass

        def find_next_improving_move(self, evaluation: rb.Evaluation, solution: rb.Solution,
                                     last_evaluated_move: rb.Move) -> Optional[rb.Move]:
            assert isinstance(last_evaluated_move, SplitRouteMove) or last_evaluated_move is None
            next_move_location = self._recover_from_move(solution, last_evaluated_move)

            # Iterate over all possible split locations
            while next_move_location is not None:
                next_move = SplitRouteMove(next_move_location)
                # Evaluate the corresponding move
                if next_move.get_cost_delta(evaluation, self.instance, solution) < -1e-2:
                    # If the move is improving, return it
                    return next_move
                # Otherwise continue with the next location
                next_move_location = self._increment_location(solution, next_move_location)
            # Terminate the search if no improving move was found
            return None


The signatures of the interfaces to implement are as follows:

.. autoapiclass:: routingblocks.LocalSearchOperator
   :members:
   :undoc-members:

.. autoapiclass:: routingblocks.Move
   :members:
   :undoc-members:

.. _pivoting_rules:

Pivoting rules
--------------

Pivoting rules implement pivoting strategies used by the local search solver.
For this purpose, they provide a method to select the next move to be applied to the solution,
and a method to determine if the search should continue or terminate. The former method is called right after the
search for improving moves terminates, either by exhausting the neighborhood or by the pivoting rule itself.
The latter method is called each time an improving move is found, and can terminate the search prematurely.

Custom pivoting rules can be implemented by subclassing :py:class:`routingblocks.PivotingRule` and overriding the
:py:meth:`routingblocks.PivotingRule.select_move` and
:py:meth:`routingblocks.PivotingRule.continue_search` methods. See :ref:`custom pivoting rules <custom_pivoting_rules>` for more details.

The following pivoting rules are provided by the routingblocks package:

.. autoapiclass:: routingblocks.BestImprovementPivotingRule
    :members:
    :undoc-members:

.. autoapiclass:: routingblocks.KBestImprovementPivotingRule
    :members:
    :undoc-members:

.. autoapiclass:: routingblocks.FirstImprovementPivotingRule
    :members:
    :undoc-members:

.. _custom_pivoting_rules:

Custom pivoting rules
---------------------

Custom pivoting rules can be implemented by subclassing :py:class:`routingblocks.PivotingRule` and overriding the
:py:meth:`routingblocks.PivotingRule.select_move` and
:py:meth:`routingblocks.PivotingRule.continue_search` methods.

Consider the following example, which implements best improvement with blinks:

.. code-block:: python

    class BestImprovementWithBlinksPivotingRule(routingblocks.PivotingRule):
        def __init__(self, blink_probability: float, randgen: routingblocks.Random):
            routingblocks.PivotingRule.__init__(self)
            self._blink_probability = blink_probability
            self._randgen = randgen
            self._best_move = None
            self._best_delta_cost = -1e-2

        def continue_search(self, found_improving_move: routingblocks.Move, delta_cost: float,
                            solution: routingblocks.Solution) -> bool:
            if delta_cost < self._best_delta_cost:
                self._best_move = found_improving_move
                self._best_delta_cost = delta_cost
                # If we do not blink, we can stop the search. Otherwise we continue.
                # This ensures that we always return the best found move, even if only one is found and that one is blinked.
                if self._randgen.uniform(0.0, 1.0) >= self._blink_probability:
                    return False
            return True

        def select_move(self, solution: routingblocks.Solution) -> Optional[routingblocks.Move]:
            move = self._best_move
            self._best_move = None
            self._best_delta_cost = -1e-2
            return move

.. autoapiclass:: routingblocks.PivotingRule
    :members:
    :undoc-members:

