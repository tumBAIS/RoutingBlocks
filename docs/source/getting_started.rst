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
            arc_data = rb.adptw.ADPTWArcData(arc['distance'], arc['consumption'], arc['travel_time'])
            instance_builder.add_arc(i, j, arc_data)

        # Create instance
        return instance_builder.build()


.. note::

    RoutingBlocks does not store parameters in the Instance object.

We use the InstanceBuilder class, which provides a convenient way to build a RoutingBlocks Instance from a set of vertices and arcs. It takes two functions as arguments: a vertex and an arc factory. These create a vertex or an arc object from the data provided by the user. The InstanceBuilder class then takes care of registering the vertices and arcs in the Instance object.


Having created the instance, we can now implement the ILS algorithm. We start by creating an Evaluation object, which will be responsible for cost calculation and efficient move evaluation. RoutingBlocks already provides a Evaluation class for the EVRP-TW-PR, so we can simply use it:

.. note::

        It is possible to implement a custom Evaluation class for custom problem settings (See `Custom problem settings<_custom_problem_settings>`_)

.. code-block:: python

    vehicle_storage_capacity = params['C']
    # Vehicle battery capacity in units of time:
    # battery capacity * inverse refueling rate = battery capacity / refueling rate
    vehicle_battery_capacity_time = params['Q'] * params['g']
    # Create an evaluation object
    evaluation = rb.adptw.Evaluation(vehicle_battery_capacity_time, vehicle_storage_capacity)

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
        rb.SwapOperator_0_1(instance, arc_set),
        rb.SwapOperator_1_1(instance, arc_set),
        rb.InsertStationOperator(instance),
        rb.RemoveStationOperator(instance),
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
        # Create a new solution by copying the current solution
        new_solution = copy.copy(solution)

        # Exchange random sequences between routes
        num_exchanges = random.randint(0, max_exchanges)
        for _ in range(num_exchanges):
            # Select two random routes
            route_1 = random.choice(new_solution)
            route_2 = random.choice(new_solution)
            if route_1 is route_2:
                continue
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
        return new_solution


We can now implement the main loop of the ILS algorithm:

.. code-block:: python

    best_solution = create_random_solution(evaluation, instance)
    current_solution = copy.copy(best_solution)
    for i in range(10):
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

        local_search = rb.LocalSearch(instance, evaluation, None)
        # Configure the local search to use a best-improvement pivoting rule
        local_search.set_use_best_improvement(True)
        # Create a set of allowed arcs
        arc_set = rb.ArcSet(instance.number_of_vertices)

        # Create a set of operators that will be used later when calling the local search
        operators = [
            rb.SwapOperator_0_1(instance, arc_set),
            rb.SwapOperator_1_1(instance, arc_set),
            rb.InsertStationOperator(instance),
            rb.RemoveStationOperator(instance),
        ]

        best_solution = create_random_solution(evaluation, instance)
        current_solution = copy.copy(best_solution)
        for i in range(10):
            # Search the neighborhood of the current solution. This modifies the solution in-place.
            local_search.optimize(current_solution, operators)
            if current_solution.cost < best_solution.cost:
                best_solution = current_solution
                print(f"New best solution found: {best_solution.cost}")

            # Perturb the current solution
            current_solution = perturb(current_solution, len(current_solution) // 2)

        print("Best solution:")
        print(solution)

The full source code can be found in the main `github repository <https://github.com/tumBAIS/RoutingBlocks/tree/develop/examples/ils>`_ .

Extending the algorithm to an ALNS
------------------------------------
.. _alns_extension:

The ILS does not perform well on the EVRP-TW-PR, as it is not able to find good solutions in a reasonable amount of time. To improve the performance, we can extend the algorithm to an `ALNS <https://en.wikipedia.org/wiki/Adaptive_large_neighborhood_search>`_.

Adapting to custom problem settings
------------------------------------
.. _custom_problem_settings:

So far, the example is limited to the EVRP-TW-PR. However, the library is designed to be easily extensible to other problem settings. To do so, we need to implement five interfaces:

* VertexData: Holds the data associated with a vertex
* ArcData: Holds the data associated with an arc
* ForwardLabel: Holds the forward label of a vertex
* BackwardLabel: Holds the backward label of a vertex
* Evaluation: Implements the main labeling and evaluation logic

.. code-block:: python

    class Evaluation(rb.Evaluation):
        def __init__(self, instance):
            super().__init__(instance)

        def cost(self, solution):
            pass

        def evaluate_move(self, solution, move):
            pass

.. warning::

    We recommend implementing a custom Evaluation class by extending the native RoutingBlocks library instead of providing a python implementation for code used beyond prototyping. See `<extension>`_ for more information