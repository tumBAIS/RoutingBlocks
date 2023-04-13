Local search
============

Local search solver
-------------------

.. autoapiclass:: routingblocks.LocalSearch
   :members:
   :undoc-members:

Operators
---------

The routingblocks package provides a set of local search operators:

.. autoclass:: routingblocks.operators.InsertStationOperator
   :members:
   :undoc-members:

.. autoclass:: routingblocks.operators.RemoveStationOperator
   :members:
   :undoc-members:

.. autoclass:: routingblocks.operators.InterRouteTwoOptOperator
   :members:
   :undoc-members:

RoutingBlocks provies a set of generic swap operators that implement relocate and exchange moves.
The operators follow the following naming convention:
``SwapOperator_<i>_<j>`` generates all moves that swap segments of length ``i`` with segments of length ``j``.
The operator considers Inter- and Intra-route moves. Each operator can be configured to consider only a subset of arcs by passing a :py:class:`routingblocks.ArcSet` to the constructor.

The following operators are available:

.. autoclass:: routingblocks.operators.SwapOperator_0_1
    :members:
    :undoc-members:
    :inherited-members:

.. autoclass:: routingblocks.operators.SwapOperator_0_2
   :members:
   :undoc-members:

.. autoclass:: routingblocks.operators.SwapOperator_0_3
   :members:
   :undoc-members:

.. autoclass:: routingblocks.operators.SwapOperator_1_1
   :members:
   :undoc-members:

.. autoclass:: routingblocks.operators.SwapOperator_1_2
   :members:
   :undoc-members:

.. autoclass:: routingblocks.operators.SwapOperator_1_3
   :members:
   :undoc-members:

.. autoclass:: routingblocks.operators.SwapOperator_2_1
   :members:
   :undoc-members:

.. autoclass:: routingblocks.operators.SwapOperator_2_2
   :members:
   :undoc-members:

.. autoclass:: routingblocks.operators.SwapOperator_2_3
   :members:
   :undoc-members:

.. autoclass:: routingblocks.operators.SwapOperator_3_1
   :members:
   :undoc-members:

.. autoclass:: routingblocks.operators.SwapOperator_3_2
   :members:
   :undoc-members:

.. autoclass:: routingblocks.operators.SwapOperator_3_3
   :members:
   :undoc-members:


Custom Local Search Operators
-----------------------------

To implement a custom local search operator, you need to implement the
:py:class:`routingblocks.LocalSearchOperator` interface:

.. autoapiclass:: routingblocks.LocalSearchOperator
   :members:
   :undoc-members:


Helpers
-------

.. autoapiclass:: routingblocks.QuadraticNeighborhoodIterator
   :members:
   :undoc-members:

.. autoapifunction:: routingblocks.iter_neighborhood

