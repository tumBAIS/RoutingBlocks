.. _alns:

Metaheuristic components
=========================

Operators
---------

The routingblocks package provides a set of destroy and repair operators that remove and insert vertices into a
solution, respectively.

.. _alns_operators:

Destroy operators
^^^^^^^^^^^^^^^^^^

.. autoapiclass:: routingblocks.operators.WorstRemovalOperator

.. autoapiclass:: routingblocks.operators.ClusterRemovalOperator

.. autoapiclass:: routingblocks.operators.StationVicinityRemovalOperator

.. autoapiclass:: routingblocks.operators.RelatedRemovalOperator

.. autoapiclass:: routingblocks.operators.RelatedVertexRemovalMove
    :members:

.. autoapiclass:: routingblocks.operators.RouteRemovalOperator

.. autoapiclass:: routingblocks.operators.RandomRemovalOperator

Insertion operators
^^^^^^^^^^^^^^^^^^^^

.. autoapiclass:: routingblocks.operators.BestInsertionOperator

.. autoapiclass:: routingblocks.operators.RandomInsertionOperator

Operator customization
^^^^^^^^^^^^^^^^^^^^^^

Move Selectors
***********************

RoutingBlocks uses the concept of move selectors to customize destroy and repair operator behavior.
These select a move from a sequence of possible moves.
Their interface can often be implemented as a lambda function, but it is possible to implement stateful move selectors as well.
The interface is as follows:

.. autoapiclass:: routingblocks.operators.MoveSelector
    :special-members: __call__

The following example illustrates two simple move selectors. The first one always selects the first move, the second one
chooses a move at random:

.. code-block:: python

    # Selects the first move.
    def my_first_move_selector(moves):
        return next(moves)

    # Selects a move at random.
    class my_random_move_selector:

        def __init__(self, random_generator):
            self.rng = random_generator

        def __call__(self, moves):
            return random.choice(list(moves))

RoutingBlocks provides a set of pre-defined move selectors:

.. autoapifunction:: routingblocks.operators.first_move_selector

.. autoapifunction:: routingblocks.operators.last_move_selector

.. autoapifunction:: routingblocks.operators.nth_move_selector_factory

.. autoapifunction:: routingblocks.operators.blink_selector_factory

.. autoapifunction:: routingblocks.operators.random_selector_factory

Other
***********************

.. autoapiclass:: routingblocks.operators.SeedSelector
    :special-members: __call__

.. autoapiclass:: routingblocks.operators.ClusterMemberSelector
    :special-members: __call__

.. autoapifunction:: routingblocks.operators.build_relatedness_matrix

.. _alns_custom_operators:

Custom operators
----------------

Custom operators can be implemented by inheriting from the abstract base classes :py:class:`routingblocks.DestroyOperator` and :py:class:`routingblocks.RepairOperator` for destroy and repair operators, respectively. See :ref:`here <custom_destroy_operator>` for an example.
The interfaces are as follows:

.. autoapiclass:: routingblocks.DestroyOperator
    :members:
    :undoc-members:

.. autoapiclass:: routingblocks.RepairOperator
    :members:
    :undoc-members:

Solvers
-------

RoutingBlocks provides (A)LNS solvers that can be extended with arbitrary destroy and repair operators.
The solvers manage operator selection, operator weights, and solution generation. We note that this solver can also be
used to implement other perturbation-based algorithms. Their interfaces are as follows:

.. autoapiclass:: routingblocks.LargeNeighborhood
    :members:
    :undoc-members:

.. autoapiclass:: routingblocks.AdaptiveLargeNeighborhood
    :members:
    :undoc-members:
