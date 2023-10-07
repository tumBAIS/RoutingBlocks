.. _custom_problem_settings:

Custom problem settings
=====================

Prototyping in Python
--------------------------

So far, the example developed in the previous sections is limited to the EVRP-TW-PR. However, the library is designed to be easily extensible to other problem settings. This requires implementing five interfaces:

* VertexData: Holds the data associated with a vertex
* ArcData: Holds the data associated with an arc
* ForwardLabel: Holds the forward label of a vertex
* BackwardLabel: Holds the backward label of a vertex
* Evaluation: Implements the main labeling and evaluation logic

Label and data classes
~~~~~~~~~~~~~~~~~~~~~~

Implementing VertexData, ArcData, and Label classes is straightforward:

.. code-block:: python

    class CVRPVertexData:
        def __init__(self, demand: int):
            self.demand = demand


    class CVRPArcData:
        def __init__(self, distance: float):
            self.distance = distance


    class CVRPForwardLabel:
        def __init__(self, distance: float, load: float):
            self.distance = distance
            self.load = load

    class CVRPBackwardLabel:
        def __init__(self, distance: float, load: float):
            self.distance = distance
            self.load = load

.. note::

    The example above effectively duplicates the code for CVRPForwardLabel and CVRPBackwardLabel. There is no requirement for distinct ForwardLabel and BackwardLabel classes, i.e., we could replace these with a single CVRPLabel class. For the sake of clarity, we keep them separate in this example.

The evaluation class
~~~~~~~~~~~~~~~~~~~~

Having defined the data and label classes holding problem-specific data, we can now implement the evaluation class.
RoutingBlocks provides two interfaces for this purpose (cf. :ref:`Evaluation`):

* :py:class:`routingblocks.PyEvaluation`: General evaluation class. Receives the full route, i.e., all concatenated route segments, for cost evaluation. This is the most general interface
* :py:class:`routingblocks.PyConcatenationBasedEvaluation`: Evaluation class for problems with constant time, i.e., concatenation-based, evaluation. Provided for convenience, i.e., to provide a simple, more efficient interface for these special cases

As constant-time evaluation is possible for the CVRP, we implement the :py:class:`routingblocks.PyConcatenationBasedEvaluation` interface in the following:

.. code-block:: python

    # Add a type alias for conciseness. Note: This is not required.
    CVRPLabel = Union[CVRPForwardLabel, CVRPBackwardLabel]

    class CVRPEvaluation(rb.PyConcatenationBasedEvaluation):
        def __init__(self, storage_capacity: float):
            rb.PyConcatenationBasedEvaluation.__init__(self)
            self._storage_capacity = storage_capacity
            self.load_penalty = 1.

        def _compute_cost(self, distance: float, load: float) -> float:
            # Helper function to compute the cost of a label.
            return distance + self.load_penalty * max(0., load - self._storage_capacity)

        def compute_cost(self, label: CVRPLabel) -> float:
            return self._compute_cost(label.distance, label.load)

        def get_cost_components(self, label: CVRPLabel) -> List[float]:
            return [label.distance, label.load]

        def concatenate(self, fwd: CVRPForwardLabel, bwd: CVRPBackwardLabel, vertex: rb.Vertex) -> float:
            return self._compute_cost(fwd.distance + bwd.distance, fwd.load + bwd.load)

        def create_backward_label(self, vertex: rb.Vertex) -> CVRPBackwardLabel:
            return CVRPBackwardLabel(0., 0.)

        def create_forward_label(self, vertex: rb.Vertex) -> CVRPForwardLabel:
            return CVRPForwardLabel(0., vertex.data.demand)

        def is_feasible(self, label: CVRPLabel) -> bool:
            return label.load <= self._storage_capacity

        def propagate_backward(self, succ_label: CVRPBackwardLabel, succ_vertex: rb.Vertex,
                               vertex: rb.Vertex, arc: rb.Arc) -> CVRPBackwardLabel:
            return CVRPBackwardLabel(succ_label.distance + arc.data.distance, succ_label.load + succ_vertex.data.demand)

        def propagate_forward(self, pred_label: CVRPForwardLabel, pred_vertex: rb.Vertex,
                              vertex: rb.Vertex, arc: rb.Arc) -> CVRPForwardLabel:
            return CVRPForwardLabel(pred_label.distance + arc.data.distance, pred_label.load + vertex.data.demand)

.. warning::

    Calls to vertex.data and arc.data are not type-safe: they work only if the vertex and arc data types have been defined in python. This is a tradeoff between performance and safety.

Theses classes can now be used in place of the ones provided by out of the box. In fact, using the solver developed in the :ref:`previous sections <alns_extension>` (`source code <https://github.com/tumBAIS/RoutingBlocks/tree/main/examples/alns>`_),
we can solve the CVRP by simply swapping the evaluation class and creating the corresponding CVRPData classes:

.. code-block:: python
    :linenos:
    :emphasize-lines: 2, 6, 19, 29, 30

    def create_instance(serialized_vertices, serialized_arcs) -> rb.Instance:
        instance_builder = rb.utility.InstanceBuilder()
        # Create and register the vertices
        for vertex in serialized_vertices:
            # Create problem-specific data held by vertices
            vertex_data = CVRPVertexData(vertex['demand'])
            # Register the vertex depending on it's type
            if vertex['Type'] == 'd':
                instance_builder.set_depot(vertex['StringID'], vertex_data)
            elif vertex['Type'] == 'c':
                instance_builder.add_customer(vertex['StringID'], vertex_data)
            else:
                instance_builder.add_station(vertex['StringID'], vertex_data)

        # Create and register the arcs
        for (i, j), arc in serialized_arcs.items():
            # Create problem-specific data held by arcs
            arc_data = CVRPArcData(arc['distance'])
            instance_builder.add_arc(i, j, arc_data)

        # Create instance
        return instance_builder.build()

    # ...

    def alns(instance: rb.Instance, vehicle_storage_capacity: float,
             number_of_iterations: int = 100, min_vertex_removal_factor: float = 0.2,
             max_vertex_removal_factor: float = 0.4):
        evaluation = CVRPEvaluation(vehicle_storage_capacity)
        evaluation.load_penalty = 1000.0

.. warning::

    We recommend implementing a custom Evaluation class by extending the native RoutingBlocks library instead of providing a Python implementation for code used beyond prototyping.

Writing a native extension
--------------------------

Pure Python-based implementations of :py:class:`routingblocks.PyEvaluation`, :py:class:`routingblocks.PyConcatenationBasedEvaluation`, and :py:class:`routingblocks.Propagator` classes suffer from a significant performance penalty. This is due to the fact that parts of the library provided in native code need to return control to python interpreter for every evaluation.
To avoid this, the library provides native extension interfaces for all of it's runtime critical components.
We provide an example of how to port the CVRP example to native code `here <https://github.com/tumBAIS/routingblocks-native-extension-example>`_.
Specifically, the repository provides the necessary boilerplate code for building, dependency management, packaging, publishing, and installation of custom native extensions.
We ask users to consider publishing their native extensions on PyPI to make them available to the community.

The source code of :py:class:`routingblocks.adptw.Evaluation` (`native/src/ADPTWEvaluation.cpp <https://github.com/tumBAIS/RoutingBlocks/blob/develop/native/src/ADPTWEvaluation.cpp>`_), :py:class:`routingblocks.niftw.Evaluation` (`native/src/NIFTWEvaluation.cpp <https://github.com/tumBAIS/RoutingBlocks/blob/develop/native/src/NIFTWEvaluation.cpp>`_), :py:class:`routingblocks.adptw.FRVCP` (`native/include/routingblocks/ADPTWEvaluation.h <https://github.com/tumBAIS/RoutingBlocks/blob/develop/native/include/routingblocks/ADPTWEvaluation.h>`_), and :py:class:`routingblocks.niftw.FRVCP` (`native/include/routingblocks/NIFTWEvaluation.h <https://github.com/tumBAIS/RoutingBlocks/blob/develop/native/include/routingblocks/NIFTWEvaluation.h>`_) provides further examples.