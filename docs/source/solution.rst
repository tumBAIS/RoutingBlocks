Solution representation
========================

.. autoapiclass:: routingblocks.Node
    :members:
    :undoc-members:

.. autoapifunction:: routingblocks.create_route

.. autoapiclass:: routingblocks.Route
    :members:
    :undoc-members:

.. autoapiclass:: routingblocks.Solution
    :members:
    :undoc-members:
    :class-doc-from: class
    :special-members: __len__, __iter__, __getitem__, __delitem__

    .. py:method:: __init__(self, evaluation: Evaluation, instance: Instance, number_of_routes: int)

        Creates a new Solution object with the specified number of empty routes.

        :param Evaluation evaluation: The evaluation object for cost and feasibility calculations.
        :param Instance instance: The Instance object representing the problem instance.
        :param int number_of_routes: The number of empty routes the solution should contain.

    .. py:method:: __init__(self, evaluation: Evaluation, instance: Instance, routes: List[Route])

        Creates a new Solution object with the specified list of routes.

        :param Evaluation evaluation: The evaluation object for cost and feasibility calculations.
        :param Instance instance: The Instance object representing the problem instance.
        :param List[Route] routes: The list of routes to include in the solution.

.. autoapiclass:: routingblocks.NodeLocation
    :members:
    :undoc-members:
