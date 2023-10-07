.. _Evaluation:

Evaluation
==========

The evaluation class implements problem-specific cost and move evaluation functions. It's design bases on the
concepts introduced in :cite:t:`VidalCrainicEtAl2014`.

Note that this class is an interface: it's not meant to be instantiated or used directly. Please use the concrete
implementations of this interface and helper functions instead. See e.g. :ref:`ADPTW` or :ref:`NIFTW` for examples of
concrete implementations.

.. warning::

    We recommend implementing a custom Evaluation class by extending the native RoutingBlocks library instead of providing a python implementation for code used beyond prototyping. See `native extensions <https://github.com/tumBAIS/routingblocks-native-extension-example>`_ for an example.

.. autoapiclass:: routingblocks.PyConcatenationBasedEvaluation
    :members:
    :undoc-members:
    :inherited-members:

.. autoapiclass:: routingblocks.PyEvaluation
    :members:
    :undoc-members:
    :inherited-members:


.. py:function:: evaluate_insertion(evaluation: Evaluation, instance: Instance, route: Route, after_position: int, vertex_id: VertexID) -> float

   Evaluates inserting a vertex into a route after the specified position.

   :param Evaluation evaluation: The evaluation function
   :param Instance instance: The instance
   :param Route route: The route
   :param int after_position: The position after which the vertex is inserted
   :param VertexID vertex_id: The id of the vertex to insert
   :return: The cost of the route with the vertex inserted
   :rtype: float

.. py:function:: evaluate_insertion(evaluation: Evaluation, instance: Instance, route: Route, after_position: int, vertex: Vertex) -> float

   Evaluates inserting a vertex into a route after the specified position.

   :param Evaluation evaluation: The evaluation function
   :param Instance instance: The instance
   :param Route route: The route
   :param int after_position: The position after which the vertex is inserted
   :param Vertex vertex: The vertex to insert
   :return: The cost of the route with the vertex inserted
   :rtype: float

.. py:function:: evaluate_insertion(evaluation: Evaluation, instance: Instance, route: Route, after_position: int, node: Node) -> float

   Evaluates inserting a node into a route after the specified position.

   :param Evaluation evaluation: The evaluation function
   :param Instance instance: The instance
   :param Route route: The route
   :param int after_position: The position after which the vertex is inserted
   :param Node node: The node to insert
   :return: The cost of the route with the vertex inserted
   :rtype: float


.. autoapifunction:: routingblocks.evaluate_splice
