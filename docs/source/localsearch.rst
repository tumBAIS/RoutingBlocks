Local search
============

Local search is a core component of almost every state-of-the-art metaheuristic designed for vehicle routing problems.
RoutingBlocks provides a :ref:`local search solver <local_search_solver>`, a set of local search operators, and a set of :ref:`pivoting rules <pivoting_rules>`
that can be used to implement customized local search procedures. It is possible to implement custom local search operators and pivoting rules.

.. _local_search_solver:

Local search solver
-------------------

.. autoapiclass:: routingblocks.LocalSearch
   :members:
   :undoc-members:

Operators
---------

The routingblocks package provides a set of local search operators:

.. autoclass:: routingblocks.operators.InsertStationOperator

.. autoclass:: routingblocks.operators.RemoveStationOperator

.. autoclass:: routingblocks.operators.InterRouteTwoOptOperator

RoutingBlocks provies a set of generic swap operators that implement relocate and exchange moves.
The operators follow the following naming convention:
``SwapOperator_<i>_<j>`` generates all moves that swap segments of length ``i`` with segments of length ``j``.
The operator considers Inter- and Intra-route moves. Each operator can be configured to consider only a subset of arcs by passing a :py:class:`routingblocks.ArcSet` to the constructor.

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

Custom Local Search Operators
-----------------------------

To implement a custom local search operator, you need to implement the
:py:class:`routingblocks.LocalSearchOperator` interface:

.. autoapiclass:: routingblocks.LocalSearchOperator
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

.. autoapiclass:: routingblocks.PivotingRule
    :members:
    :undoc-members:

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




Helpers
-------

.. autoapiclass:: routingblocks.QuadraticNeighborhoodIterator
   :members:
   :undoc-members:

.. autoapifunction:: routingblocks.iter_neighborhood

