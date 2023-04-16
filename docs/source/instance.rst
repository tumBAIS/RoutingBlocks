Problem instance
================

Problem instance classes represent the problem to be solved. They model the underlying graph and store problem-specific
data. The following classes are available:

.. autoapiclass:: routingblocks.Vertex
    :members:
    :undoc-members:

.. autoapiclass:: routingblocks.Arc
    :members:
    :undoc-members:

.. autoapiclass:: routingblocks.Instance
    :members:
    :undoc-members:
    :special-members: __len__, __iter__

    .. py:method:: __init__(self, depot: Vertex, stations: List[Vertex], customers: List[Vertex], arcs: List[List[Arc]], fleet_size: int) -> None

          Initialize an Instance with a depot, lists of stations and customers, a list of arcs, and a fleet size.

          :param Vertex depot: The depot vertex
          :param List[Vertex] stations: A list of station vertices
          :param List[Vertex] customers: A list of customer vertices
          :param List[List[Arc]]] arcs: A matrix of Arc objects representing the connections between vertices
          :param int fleet_size: The number of vehicles in the fleet

    .. py:method:: __init__(self, vertices: List[Vertex], arcs: List[List[Arc]]) -> None

          Initialize an Instance with a list of vertices and a list of arcs. Sets the fleet size to the number of customers. Expects vertices to be in the order depot, stations, customers.

          :param List[Vertex] vertices: A list of vertices in the order depot, stations, customers
          :param List[List[Arc]] arcs: A list of lists of Arc objects representing the connections between vertices

    .. py:method:: __init__(self, vertices: List[Vertex], arcs: List[List[Arc]], fleet_size: int) -> None

          Initialize an Instance with a list of vertices, a list of arcs, and a fleet size. Expects vertices to be in the order depot, stations, customers.

          :param List[Vertex] vertices: A list of vertices in the order depot, stations, customers
          :param List[List[Arc]] arcs: A list of lists of Arc objects representing the connections between vertices
          :param int fleet_size: The number of vehicles in the fleet


The :ref:`instance builder <instance-builder>` is a utility class that can be used to create instances. It takes
care of ordering, ensures unique vertex IDs, and validates consistency. The following example illustrates it's usage:

.. code-block:: python

    from routingblocks import InstanceBuilder, Vertex, Arc

    class MyVertexData:
        pass
        
    class MyArcData:
        pass

    # Default factories use the constructors of Vertex and Arc. Use custom factories to create Vertex/Arc representations.
    # optimized for specific problem settings (e.g., `create_adtpw_vertex` and `create_adtpw_arc` for ADTPW)
    builder = InstanceBuilder()
    builder.fleet_size = 3

    builder.add_depot("D", MyVertexData())

    builder.add_customer("C1", MyVertexData())
    builder.add_customer("C2", MyVertexData())

    builder.add_station("S1", MyVertexData())

    for i, j in product(["D", "S1", "C1", "C2"], ["D", "S1", "C1", "C2"]):
        builder.add_arc(i, j, MyArcData())

    instance = builder.build()



.. _instance-builder:

.. autoapiclass:: routingblocks.utility.InstanceBuilder
    :members:
    :undoc-members:

.. py:function:: vertex_factory(id: VertexID, str_id: str, is_depot: bool, is_station: bool, data: T) -> Vertex
    :noindex:

    Function signature for a vertex_factory. Creates a new vertex using the given parameters.

    :param id: Unique ID of the vertex
    :param str_id: Name of the vertex
    :param is_depot: Whether the vertex is a depot
    :param is_station: Whether the vertex is a station
    :param data: User-defined data to be stored in the vertex
    :return: A new Vertex object

.. py:function:: arc_factory(data: T) -> Arc
    :noindex:

    Function signature for a arc_factory. Creates a new arc with the given data.

    :param data: User-defined data to be stored in the arc
    :return: A new Arc object
