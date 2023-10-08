.. _custom_operators:

Custom operators
=============================

.. _custom_destroy_operator:

The example developed in the previous section so far uses only the standard operators RoutingBlocks provides out of the box. However, it is also possible to implement custom local search, destroy, and repair operators. We'll implement a simple RouteRemoval destroy operator as an example:

.. code-block:: python

    # Custom destory, repair, and local serach operators inherit from the DestroyOperator, RepairOperator, and Operator base classe, respectively, respectively.
    class RouteRemoveOperator(routingblocks.DestroyOperator):
        def __init__(self, rng: routingblocks.Random):
            # Important: Do not use super()!
            routingblocks.DestroyOperator.__init__(self)
            self._rng = rng

        # Returns true if the operator can be applied to the current solution
        def can_apply_to(self, _solution: routingblocks.Solution) -> bool:
            return len(_solution) > 0

        # Applies the operator to the current solution
        def apply(self, evaluation: routingblocks.Evaluation, solution: routingblocks.Solution, number_of_removed_vertices: int) -> List[
            int]:
            # Try to remove random routes
            removed_customers = []
            while len(solution) > 0 and len(removed_customers) < number_of_removed_vertices:
                random_route_index = self._rng.randint(0, len(solution) - 1)
                removed_customers.extend(x.vertex_id for x in solution[random_route_index] if not x.vertex.is_depot)
                del solution[random_route_index]
            return removed_customers

        # Returns the operator's name
        def name(self) -> str:
            return "RouteRemoveOperator"

The operator removes random routes from the solution until the desired number of vertices has been removed. Destroy operators implement the DestroyOperator interface. The interface requires implementation of the following methods:

* can_apply_to: Returns true if the operator can be applied to the current solution
* apply: Applies the operator to the current solution, returning the ids of the removed vertices
* name: Returns the operator's name

The operator can be registered with the ALNS solver in the same way as the standard operators:

.. code-block:: python

        alns.add_destroy_operator(RouteRemoveOperator(randgen))

The same approach can be used to implement custom repair and local search operators. See :ref:`here <alns_custom_operators>` for further details on implementing destroy and repair operators, and :ref:`here <local_search_custom_operators>` for a comprehensive example of a custom local search operator.