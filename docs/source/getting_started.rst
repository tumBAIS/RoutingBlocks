.. _getting_started:

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
                tokens = line.split()
                if len(tokens) == 0:
                    break
                # Read columns into a dictionary
                vertex = {key: (x if key in str_fields else float(x)) for key, x in zip(fields, tokens)}
                vertices.append(vertex)
            # Parse the parameters
            parameters = {}
            for line in instance_stream:
                key, *_, value = line.split()
                # Remove surrounding / characters and parse the value
                parameters[key] = float(value[1:-1])

        # Create a mapping from pairs of vertices to arcs
        arcs = {}
        for i in vertices:
            for j in vertices:
                # Compute distance
                distance = sqrt((i['x'] - j['x']) ** 2 + (i['y'] - j['y']) ** 2)
                # Compute travel time (distance / average velocity)
                travel_time = distance / parameters['v']
                # Compute consumption (consumption rate * travel time / recharging rate)
                consumption = parameters['r'] * travel_time / parameters['g']
                arcs[i['StringID'], j['StringID']] = dict(distance=distance, travel_time=travel_time,
                                                          consumption=consumption)

        return vertices, arcs, parameters

.. note::

    The instance format is described in the `supplemental material <https://data.mendeley.com/datasets/h3mrm5dhxw/1>`_ to :cite:t:`SchneiderStengerEtAl2014`.

Next, we create a RoutingBlocks Instance object from the parsed data:

.. code-block:: python

    def create_instance(serialized_vertices, serialized_arcs) -> rb.Instance:
        instance_builder = rb.utility.InstanceBuilder(create_vertex=rb.adptw.create_adptw_vertex,
                                                      create_arc=rb.adptw.create_adptw_arc)
        # Create and register the vertices
        for vertex in serialized_vertices:
            # Create problem-specific data held by vertices
            vertex_data = rb.adptw.VertexData(vertex['x'], vertex['y'], vertex['demand'], vertex['ReadyTime'],
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
            arc_data = rb.adptw.ArcData(arc['distance'], arc['consumption'], arc['travel_time'])
            instance_builder.add_arc(i, j, arc_data)

        # Create instance
        return instance_builder.build()


.. note::

    RoutingBlocks does not store parameters in the Instance object.

We utilize the InstanceBuilder class, offering a convenient way to construct a RoutingBlocks Instance from a set of vertices and arcs. It requires two functions as arguments: a vertex and an arc factory. These functions create a vertex or an arc object based on the data provided by the user. The InstanceBuilder class then handles the registration of vertices and arcs within the Instance object.

Once the instance is created, we can proceed to implement the ILS algorithm. We initiate by creating an Evaluation object, which is responsible for cost calculation and efficient move evaluation. RoutingBlocks already includes an Evaluation class for the EVRP-TW-PR, allowing us to easily use it:

.. note::

        It is possible to implement a custom Evaluation class for custom problem settings (See :ref:`Custom problem settings <custom_problem_settings>`).

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

    The module's name ``adptw`` refers to the classification introduced in :cite:t:`vrpis`.

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


Here, we begin by copying all customers into a single list, which is then shuffled and randomly split at various positions to generate a set of routes. We convert these into RoutingBlocks Route objects using the create_route helper function. This function takes the evaluation function, the instance, and a sequence of vertex IDs as arguments and creates a Route object, adding start and end depots as needed. Finally, we create and return a solution using the list of routes.

Subsequently, we create and configure the local search solver:

.. code-block:: python

    # Create a best-improvement pivoting rule
    pivoting_rule = rb.BestImprovementPivotingRule()
    # Configure the local search - use the best-improvement pivoting rule
    local_search = rb.LocalSearch(instance, evaluation, None, pivoting_rule)
    # Create a set of allowed arcs
    arc_set = rb.ArcSet(instance.number_of_vertices)

    # Create a set of operators that will be used later when calling the local search
    operators = [
        rb.operators.SwapOperator_0_1(instance, arc_set),
        rb.operators.SwapOperator_1_1(instance, arc_set),
        rb.operators.InsertStationOperator(instance),
        rb.operators.RemoveStationOperator(instance),
    ]


The local search solver accepts four arguments: the instance, the evaluation used, a second evaluation object that verifies moves deemed profitable by the first evaluation class, and a pivoting rule.
Passing a second evaluation object for verification is beneficial for problems like EVRP-TW-PR, where exact evaluation is costly.
By default, the ADPTW Evaluation class implements approximate move evaluation. We can either pass an exact evaluation class here,
or we can pass None, which prompts the local search to validate moves by applying them to a solution copy and evaluating the cost based on forward labels.

The pivoting rule implements the pivoting strategy used by the local search. RoutingBlocks provides three pivoting rules:
:ref:`best improvement <best_improvement_pivoting_rule>`, :ref:`k-best improvement <k_best_improvement_pivoting_rule>`, and :ref:`first improvement <first_improvement_pivoting_rule>`.
It is also possible to implement custom pivoting rules (See :ref:`custom pivoting rules <custom_pivoting_rules>`).
The former is the default and is the one we use here. The latter stops the local search as soon as a profitable move is found.

Additionally, we create a set of operators to be used later when invoking the local search. The implementations provided by RoutingBlocks require a set of allowed arcs as an argument. The operator will only consider arcs within this set. By default, all arcs are allowed.
Executing the local search procedure is as simple as calling

.. code-block:: python

    local_search.optimize(solution, operators)

Be aware that this process will modify the solution object in-place.

The last procedure to implement is the perturbation function. This function disturbs the local minimum identified by the local search in order to escape local optima. We implement a straightforward perturbation function that swaps a random number of segments between randomly chosen routes within the solution:

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

        pivoting_rule = rb.BestImprovementPivotingRule()
        local_search = rb.LocalSearch(instance, evaluation, None, pivoting_rule)
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

A simple ILS algorithm often falls short in competitive problem settings such as the EVRP-TW-PR. In these cases, state-of-the-art algorithms rely on ALNS. ALNS employs a set of destroy and repair operators to perturb the current solution. Destroy operators remove a portion of the solution, while repair operators attempt to fix the solution by reinserting the removed customers. Operator selection is done probabilistically, with the probability of selecting an operator being proportional to its performance, which is estimated based on the number of times an operator has improved the solution.

RoutingBlocks offers an ALNS solver and several destroy and repair operators out of the box, making the implementation of ALNS fairly straightforward:

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

        pivoting_rule = rb.BestImprovementPivotingRule()
        local_search = rb.LocalSearch(instance, evaluation, None, pivoting_rule)
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


We begin with the boilerplate code established for the ILS and add just a few lines to create and configure the ALNS solver. This class is responsible for operator selection and weight adaptation. It takes a random engine and a smoothing factor as arguments. The smoothing factor determines the weight of historical performance when selecting an operator. Next, we create and register destroy and repair operators with the ALNS solver. RoutingBlocks provides a :ref:`set of standard operators <alns_operators>` out of the box. In this case, we use RandomInsertion, BestInsertion, RandomRemoval, and WorstRemoval. We configure BestInsertion and WorstRemoval to select insertion/removal spots using a blink selection criterion.

We can now employ the ALNS solver to perturb the current solution within the main loop:


.. code-block:: python

        # Generate a random starting solution
        best_solution = create_random_solution(evaluation, instance)
        for i in range(1, number_of_iterations+1):
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

We employ three essential methods of the ALNS solver:

1. alns.generate: This method selects and applies a destroy and a repair operator to the current solution, modifying it in-place. It returns a tuple of the chosen operators.
2. alns.collect_score: This method gathers scores for the provided operators. It requires the selected operators and a score as arguments.
3. alns.adapt_operator_weights: This method adjusts the weights of the operators based on the scores collected during the last period.

For more details on the ALNS solver, see the :ref:`documentation <alns>`. The full code of the ALNS algorithm is available `here <https://github.com/tumBAIS/RoutingBlocks/tree/main/examples/alns>`_. A more sophisticated ALNS-based algorithm can be found in the `main repository <https://github.com/tumBAIS/RoutingBlocks/tree/main/examples/evrptw>`_.