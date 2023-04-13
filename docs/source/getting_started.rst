Getting started
===============================

Installing the library
----------------------

The package is available on PyPI and can be installed using pip:

.. code-block:: bash

    pip install routingblocks

To obtain the bleeding-edge development version, run

.. code-block:: bash

    pip install git+https://github.com/tumBAIS/RoutingBlocks

instead.

Implementing a simple ILS
---------------------------

Let's start by implementing a simple `ILS <https://en.wikipedia.org/wiki/Iterated_local_search>`_ algorithm to solve the `EVRP-TW-PR <https://https://research.sabanciuniv.edu/id/eprint/26033/1/WP_EVRPTW-Partial_Recharge_KeskinCatay.pdf>`_.

First, import the library and read the instance

.. code-block:: python

    import routingblocks as rb

    def parse_instance(instance_path: Path):
        str_fields = ['StringID', 'Type']
        with open(instance_path) as instance_stream:
            fields = instance_stream.readline().split()
            # Parse the vertices
            vertices = []
            for line in instance_stream:
                if len(line) == 0:
                    break
                vertex = {key: (x if key in str_fields else float(x)) for key, x in zip(fields, line.split())}
                vertices.append(vertex)
            # Parse the parameters
            parameters = {}
            for line in instance_stream:
                key, *_, value = line.split()
                # Remove surrounding / characters and parse the value
                parameters[key] = float(value[1:-1])

        # Create a matrix of arcs
        arcs = []
        for i in vertices:
            next_row = []
            for j in vertices:
                # Compute distance
                distance = sqrt((i['x'] - j['x']) ** 2 + (i['y'] - j['y']) ** 2)
                # Compute travel time (distance / average velocity)
                travel_time = distance / parameters['v']
                # Compute consumption (consumption rate * travel time / recharging rate)
                consumption = parameters['r'] * travel_time / parameters['g']
                next_row.append(dict(distance=distance, travel_time=travel_time, consumption=consumption))
            arcs.append(next_row)

        return vertices, arcs, parameters

.. note::

    The instance format is described in the `supplemental material <https://data.mendeley.com/datasets/h3mrm5dhxw/1>`_ to `Schneider et. al (2014) <https://pubsonline.informs.org/doi/abs/10.1287/trsc.2013.0490>`_.

Next, we create a RoutingBlocks Instance object from the parsed data:

.. code-block:: python

    def create_instance(serialized_vertices, serialized_arcs) -> rb.Instance:
        instance_builder = rb.utility.InstanceBuilder(create_vertex=rb.adptw.create_adptw_vertex,
                                                      create_arc=rb.adptw.create_adptw_arc)
        # Create and register the vertices
        for vertex in serialized_vertices:
            # Create problem-specific data held by vertices
            vertex_data = rb.adptw.ADPTWVertexData(vertex['x'], vertex['y'], vertex['demand'], vertex['ReadyTime'],
                                                   vertex['DueDate'],
                                                   vertex['ServiceTime'])
            # Register the vertex dependinx for x in self._move_selector(related_vertices)g on it's type
            if vertex['Type'] == 'd':
                instance_builder.set_depot(vertex['StringID'], vertex_data)
            elif vertex['Type'] == 'c':
                instance_builder.add_customer(vertex['StringID'], vertex_data)
            else:
                instance_builder.add_station(vertex['StringID'], vertex_data)

        # Create and register the arcs
        for (i, j), arc in serialized_arcs.items():
            # Create problem-specific data held by arcs
            arc_data = rb.adptw.ADPTWArcData(arc['distance'], arc['consumption'], arc['travel_time'])
            instance_builder.add_arc(i, j, arc_data)

        # Create instance
        return instance_builder.build()


.. note::

    RoutingBlocks does not store parameters in the Instance object.

We use the InstanceBuilder class, which provides a convenient way to build a RoutingBlocks Instance from a set of vertices and arcs. It takes two functions as arguments: a vertex and an arc factory. These create a vertex or an arc object from the data provided by the user. The InstanceBuilder class then takes care of registering the vertices and arcs in the Instance object.


Having created the instance, we can now implement the ILS algorithm. We start by creating an Evaluation object, which will be responsible for cost calculation and efficient move evaluation. RoutingBlocks already provides a Evaluation class for the EVRP-TW-PR, so we can simply use it:

.. note::

        It is possible to implement a custom Evaluation class for custom problem settings (See `Custom problem settings <_custom_problem_settings>`_)

.. code-block:: python

    vehicle_storage_capacity = params['C']
    # Vehicle battery capacity in units of time:
    # battery capacity * inverse refueling rate = battery capacity / refueling rate
    vehicle_battery_capacity_time = params['Q'] * params['g']
    # Create an evaluation object
    evaluation = rb.adptw.Evaluation(vehicle_battery_capacity_time, vehicle_storage_capacity)
    # Set the penalty factors used to penalize violations of the time window, the
    # vehicle capacity, and the charge constraints
    evaluation.overload_penalty_factor = 100.
    evaluation.overcharge_penalty_factor = 100.
    evaluation.time_shift_penalty_factor = 100.

.. note::

    The namespace name ``adptw`` refers to the classification introduced in `Schiffer et al. (2017) <https://www.semanticscholar.org/paper/A-solution-framework-for-the-class-of-vehicle-with-Schiffer-Klein/8eff30dda8ba9faf9aa4d814838fea20d7287203>`_.

Being done with the setup, we can start implementing the main ILS algorithm.
We start by creating a random solution:

.. code-block:: python

    def create_random_solution(evaluation: rb.Evaluation, instance: rb.Instance):
        customer_vertex_ids = [x.vertex_id for x in instance.customers]
        random.shuffle(customer_vertex_ids)

        # Draw a sequence of positions where to split
        number_of_splits = random.randint(1, len(customer_vertex_ids) // 2)
        split_positions = [0, *sorted(random.sample(range(1, len(customer_vertex_ids) - 1), number_of_splits)),
                           len(customer_vertex_ids)]
        # Create routes according to the split positions. Each route is a list of customer vertex ids.
        routes = [[customer_vertex_ids[route_start_index:route_end_index]] for route_start_index, route_end_index in
                  zip(split_positions, split_positions[1:])]
        # Create RoutingBlocks Route objects
        routes = [rb.create_route(evaluation, instance, route) for route in routes]
        # Create RoutingBlocks Solution object
        return rb.Solution(evaluation, instance, routes)


Here, we start by copying all customers into a single list, which is then shuffled and split at random positions to yield a set of routes. We convert these to routingblocks Route objects using the create_route helper function, which takes as arguments the evaluation function, the instance, and a sequence of vertex ids, and creates a Route object, adding start and end depots accordingly. Finally, we create and return a solution from the list of routes.

Next, we create and configure the local search solver:

.. code-block:: python

    local_search = rb.LocalSearch(instance, evaluation, None)
    # Configure the local search to use a best-improvement pivoting rule
    local_search.set_use_best_improvement(True)
    # Create a set of allowed arcs
    arc_set = rb.ArcSet(instance.number_of_vertices)

    # Create a set of operators that will be used later when calling the local search
    operators = [
        rb.operators.SwapOperator_0_1(instance, arc_set),
        rb.operators.SwapOperator_1_1(instance, arc_set),
        rb.operators.InsertStationOperator(instance),
        rb.operators.RemoveStationOperator(instance),
    ]


The local search solver takes three arugments. The instance, the evaluation used, and a second evaluation class that is used to verify moves the first evaluation class deems profitable. This is useful for problems like the EVRP-TW-PR, where exact evaluation is expensive. The default ADPTW Evaluation class implements approximate move evaluation. We could either pass a exact evaluation class here, or we could pass None, which will cause the local search to verify moves by applying them to a copy of the solution, evaluation the cost based on forward labels. This is what we do here.

We also create a set of operators that will be used later when calling the local search. The implementations provided by RoutingBlocks take a set of allowed arcs as an argument. Only arcs within this set will be considered by the operator. By default, all arcs are allowed.

Executing the local search procedure is as simple as calling

.. code-block:: python

    local_search.optimize(solution, operators)

Note that this will modify the solution object in-place.

The final procedure to implement is the perturbation function. This function perturbs the local minimum found by the local search to escape local optima. We implement a simple perturbation function that exchanges a random number of segments between randomly selected routes in the solution:

.. code-block:: python

    def perturb(solution: rb.Solution, max_exchanges: int) -> rb.Solution:
        assert sum(1 for r in solution if not r.empty) > 1, "Cannot perturb a solution with only one route."
        # Create a new solution by copying the current solution
        new_solution = copy.copy(solution)

        # Exchange random sequences between routes
        num_exchanges = random.randint(0, max_exchanges)
        for _ in range(num_exchanges):
            # Select two random routes
            while True:
                route_1 = random.choice(new_solution)
                route_2 = random.choice(new_solution)
                if route_1 is not route_2 and not route_1.empty and not route_2.empty:
                    break
            # Select a random sequence of customers in route 1 that does not include the depot
            start_index_1 = random.randint(1, len(route_1) - 2)
            # end_index is exclusive
            end_index_1 = random.randint(start_index_1, len(route_1) - 1)
            # Do the same for the second route
            # Select a random sequence of customers in route 1 that does not include the depot
            start_index_2 = random.randint(1, len(route_2) - 2)
            # end_index is exclusive
            end_index_2 = random.randint(start_index_2, len(route_2) - 1)
            # Exchange the sequences
            new_solution.exchange_segment(route_1, start_index_1, end_index_1,
                                          route_2, start_index_2, end_index_2)
        return new_solutio

We can now implement the main loop of the ILS algorithm:

.. code-block:: python

    best_solution = create_random_solution(evaluation, instance)
    current_solution = copy.copy(best_solution)
    for i in range(number_of_iterations):
        # Search the neighborhood of the current solution. This modifies the solution in-place.
        local_search.optimize(current_solution, operators)
        if current_solution.cost < best_solution.cost:
            best_solution = current_solution
            print(f"New best solution found: {best_solution.cost}")

        # Perturb the current solution
        current_solution = perturb(current_solution, len(current_solution) // 2)

Putting everything together, we arrive at the following code:

.. code-block:: python

    def solve(instance_path: Path):
        vertices, arcs, params = parse_instance(instance_path)
        instance = create_instance(vertices, arcs)
        vehicle_storage_capacity = params['C']
        # Vehicle battery capacity in units of time:
        # battery capacity * inverse refueling rate = battery capacity / refueling rate
        vehicle_battery_capacity_time = params['Q'] * params['g']

        evaluation = rb.adptw.Evaluation(vehicle_battery_capacity_time, vehicle_storage_capacity)
        # Set the penalty factors used to penalize violations of the time window, the
        # vehicle capacity, and the charge constraints
        evaluation.overload_penalty_factor = 100.
        evaluation.overcharge_penalty_factor = 100.
        evaluation.time_shift_penalty_factor = 100.

        local_search = rb.LocalSearch(instance, evaluation, None)
        # Configure the local search to use a best-improvement pivoting rule
        local_search.set_use_best_improvement(True)
        # Create a set of allowed arcs
        arc_set = rb.ArcSet(instance.number_of_vertices)

        # Create a set of operators that will be used later when calling the local search
        operators = [
            rb.operators.SwapOperator_0_1(instance, arc_set),
            rb.operators.SwapOperator_1_1(instance, arc_set),
            rb.operators.InsertStationOperator(instance),
            rb.operators.RemoveStationOperator(instance),
        ]

        best_solution = create_random_solution(evaluation, instance)
        current_solution = copy.copy(best_solution)
        for i in range(10):
            # Search the neighborhood of the current solution. This modifies the solution in-place.
            local_search.optimize(current_solution, operators)
            if current_solution.cost < best_solution.cost:
                best_solution = current_solution
                print(f"New best solution found: {best_solution.cost} ({best_solution.feasible})")

            # Perturb the current solution
            current_solution = perturb(current_solution, len(current_solution) // 2)

        print("Best solution:")
        print(solution)

The full source code can be found in the main `github repository <https://github.com/tumBAIS/RoutingBlocks/tree/develop/examples/ils>`_ .

Extending the algorithm to an ALNS
------------------------------------
.. _alns_extension:

A simple ILS algorithm is often not sufficient to compete on problem settings such as the EVRP-TW-PR. Here, state of the art algorithm base on `ALNS <https://en.wikipedia.org/wiki/Adaptive_large_neighborhood_search>`_. ALNS utilizes a set of destroy and repair operators to perturb the current solution. The destroy operators remove a part of the solution, while the repair operators try to repair the solution by inserting the removed customers into the solution again. Selecting the operators is done probabilistically, with the probability of selecting an operator being proportional to the operator's performance, which is estimated based the number of times a operator improved the solution.

RoutingBlocks provides a ALNS solver and several destroy and repair operators out of the box. Implementing ALNS is thus straightforward:

.. code-block:: python

    def alns(instance: rb.Instance, vehicle_storage_capacity: float, vehicle_battery_capacity_time: float,
             number_of_iterations: int = 100, min_vertex_removal_factor: float = 0.2,
             max_vertex_removal_factor: float = 0.4):
        evaluation = rb.adptw.Evaluation(vehicle_battery_capacity_time, vehicle_storage_capacity)
        # Set the penalty factors used to penalize violations of the time window, the
        # vehicle capacity, and the charge constraints
        evaluation.overload_penalty_factor = 100.
        evaluation.overcharge_penalty_factor = 100.
        evaluation.time_shift_penalty_factor = 100.

        local_search = rb.LocalSearch(instance, evaluation, None)
        # Configure the local search to use a best-improvement pivoting rule
        local_search.set_use_best_improvement(True)
        # Create a set of allowed arcs
        arc_set = rb.ArcSet(instance.number_of_vertices)

        # Create a set of operators that will be used later when calling the local search
        operators = [
            rb.operators.SwapOperator_0_1(instance, arc_set),
            rb.operators.SwapOperator_1_1(instance, arc_set),
            rb.operators.InsertStationOperator(instance),
            rb.operators.RemoveStationOperator(instance),
        ]
        #############################################################################################
        # End of the code that is identical to the ILS algorithm
        #############################################################################################

        # Create a random engine and seed it with the current time
        randgen = rb.Random(time.time_ns())
        # Create an ALNS solver.
        # Smoothing factor determines the weight of historic performance when selecting an operator.
        smoothing_factor = 0.4
        alns = rb.AdaptiveLargeNeighborhood(randgen, smoothing_factor)

        # Register some operators with the ALNS solver
        alns.add_repair_operator(rb.operators.RandomInsertionOperator(randgen))
        alns.add_repair_operator(rb.operators.BestInsertionOperator(instance,
                                                                    rb.operators.blink_selector_factory(
                                                                        blink_probability=0.1, randgen=randgen)))
        alns.add_destroy_operator(rb.operators.RandomRemovalOperator(randgen))
        alns.add_destroy_operator(rb.operators.WorstRemovalOperator(instance,
                                                                    rb.operators.blink_selector_factory(
                                                                        blink_probability=0.1, randgen=randgen)))

We start from the boilerplate code developed for the ILS and only add a few lines to create and configure the ALNS solver. This class is responsible for operator selection and weight adaption. It takes as arguments a random engine and a smoothing factor. The smoothing factor determines the weight of historic performance when selecting an operator. Next, we create and register destroy and repair operators with the ALNS solver. RoutingBlocks provides a `set of standard operators <alns_operators>`_ out of the box. Here, we use RandomInsertion, BestInsertion, RandomRemoval, and WorstRemoval. We configure BestInsertion and WorstRemoval to select insertion/removal spots using a blink selection criterion.

We can now utilize the ALNS solver to perturb the current solution in the main loop:

.. code-block:: python

        # Generate a random starting solution
        best_solution = create_random_solution(evaluation, instance)
        for i in range(number_of_iterations):
            current_solution = copy.copy(best_solution)
            # Perturb the current solution
            number_of_vertices_to_remove = int(random.uniform(min_vertex_removal_factor, max_vertex_removal_factor) * sum(
                len(route) - 2 for route in current_solution))
            picked_operators = alns.generate(evaluation, current_solution, number_of_vertices_to_remove)

            # Search the neighborhood of the current solution. This modifies the solution in-place.
            local_search.optimize(current_solution, operators)

            if current_solution.cost < best_solution.cost:
                best_solution = current_solution
                print(f"New best solution found: {best_solution.cost} ({best_solution.feasible})")
                # Update the ALNS solver with the performance of the operators used in the last iteration
                # We assign a score of '4' to the operators that were used to improve the solution
                alns.collect_score(*picked_operators, 4)
            else:
                # Update the ALNS solver with the performance of the operators used in the last iteration
                # We assign a score of '0' to the operators that were not used to improve the solution
                alns.collect_score(*picked_operators, 0)

            # Calculate new operator weights based on the last period
            if i % 20 == 0:
                alns.adapt_operator_weights()

        return best_solution

We utilize three fundamental methods of the ALNS solver:
1. alns.generate, which selects and applies a destroy and a repair operator to the current solution, modifying it in-place. The method returns a tuple of the selected operators.
2. alns.collect_score, which collects scores for the passed operators. The method takes as arguments the selected operators and a score.
3. alns.adapt_operator_weights, which adapts the weights of the operators based on the scores collected in the last period.

For more details on the ALNS solver, see the `documentation <alns>`_. The full code of the ALNS algorithm is available `here <alns_code>`_. A more sophisticated ALNS-based algorithm can be found in the `main repository <https://github.com/tumBAIS/RoutingBlocks/tree/main/examples/evrptw>`_.

Implementing custom operators
------------------------------------

The RoutingBlocks library provides a set of standard operators out of the box. However, it is also possible to implement custom local search, destroy, and repair operators. We'll implement a simple RouteRemoval destroy operator as an example:

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

The operator removes random routes from the solution until the desired number of vertices has been removed. Destroy operators implement the DestroyOperator interface. The interface requires the implementation of the following methods:
* can_apply_to: Returns true if the operator can be applied to the current solution
* apply: Applies the operator to the current solution, returning the ids of the removed vertices
* name: Returns the operator's name

The operator can be registered with the ALNS solver in the same way as the standard operators:

.. code-block:: python

        alns.add_destroy_operator(RouteRemoveOperator(randgen))


.. _custom_problem_settings:

Adapting to custom problem settings
------------------------------------

So far, the example is limited to the EVRP-TW-PR. However, the library is designed to be easily extensible to other problem settings. To do so, we need to implement five interfaces:

* VertexData: Holds the data associated with a vertex
* ArcData: Holds the data associated with an arc
* ForwardLabel: Holds the forward label of a vertex
* BackwardLabel: Holds the backward label of a vertex
* Evaluation: Implements the main labeling and evaluation logic

Forward and Backward labels can also be combined into a single label class.

The following example implements a simple CVRP evaluation class:

.. code-block:: python

    class CVRPVertexData:
        def __init__(self, demand: int):
            self.demand = demand


    class CVRPArcData:
        def __init__(self, distance: float):
            self.distance = distance


    class CVRPLabel:
        def __init__(self, distance: float, load: float):
            self.distance = distance
            self.load = load


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

        def concatenate(self, fwd: CVRPLabel, bwd: CVRPLabel, vertex: rb.Vertex) -> float:
            return self._compute_cost(fwd.distance + bwd.distance, fwd.load + bwd.load)

        def create_backward_label(self, vertex: rb.Vertex) -> CVRPLabel:
            return CVRPLabel(0., 0.)

        def create_forward_label(self, vertex: rb.Vertex) -> CVRPLabel:
            return CVRPLabel(0., vertex.data.demand)

        def is_feasible(self, label: CVRPLabel) -> bool:
            return label.load <= self._storage_capacity

        def propagate_backward(self, succ_label: CVRPLabel, succ_vertex: rb.Vertex,
                               vertex: rb.Vertex, arc: rb.Arc) -> CVRPLabel:
            return CVRPLabel(succ_label.distance + arc.data.distance, succ_label.load + succ_vertex.data.demand)

        def propagate_forward(self, pred_label: CVRPLabel, pred_vertex: rb.Vertex,
                              vertex: rb.Vertex, arc: rb.Arc) -> CVRPLabel:
            return CVRPLabel(pred_label.distance + arc.data.distance, pred_label.load + vertex.data.demand)

.. warning::

    Calls to vertex.data and arc.data are not type-safe: they work only if the vertex and arc data types have been defined in python. This is a tradeoff between performance and safety.

To use it, simply create the corresponding CVRPData classes during instance construction and swap the evaluation class:

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

    We recommend implementing a custom Evaluation class by extending the native RoutingBlocks library instead of providing a python implementation for code used beyond prototyping. See `<extension>`_ for more information