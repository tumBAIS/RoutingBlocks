.. _alns:

ALNS
====

Solver
------

RoutingBlocks provides a generic ALNS solver that can be extended with arbitrary destroy and repair operators.
The solver manages operator selection, operator weights, and solution generation:

.. autoapiclass:: routingblocks.AdaptiveLargeNeighborhood
    :members:
    :undoc-members:

Operators
---------

The routingblocks package provides a set of destroy and repair operators.

.. _alns_operators:

.. autoapiclass:: routingblocks.operators.RandomRemovalOperator
    :members:
    :undoc-members:

.. autoapiclass:: routingblocks.operators.RandomInsertionOperator
    :members:
    :undoc-members:

Custom operators
----------------

Custom ALNS operators can be implemented by inheriting from the abstract base classes :py:class:`routingblocks.DestroyOperator` and :py:class:`routingblocks.RepairOperator` for destroy and repair operators, respectively.
The interfaces are as follows:

.. autoapiclass:: routingblocks.DestroyOperator
    :members:
    :undoc-members:

.. autoapiclass:: routingblocks.RepairOperator
    :members:
    :undoc-members:
