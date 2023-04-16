from typing import overload


class Solution:
    """
    The Solution class represents a solution to a VRP problem.
    It maintains a list of Route objects, providing methods for cost calculation, feasibility checking, route manipulation, and finding vertices.
    Solution objects behave like lists of routes, so you can iterate over them, index them, and get their length:

    .. code-block:: python

        solution = Solution(evaluation, instance, 5)

        for route in solution:
            print(route)

        print(solution[0])

        print(len(solution))

        del solution[0]

    Note that any operations that add routes implicitly copy the route objects. For example:

    .. code-block:: python

        solution = Solution(evaluation, instance, 0)
        route = create_route(evaluation, instance, [D, C1, D])
        solution.add_route(route)

        route.insert_vertex_after(1, C2)
        print(route) # [D, C1, C2, D]
        print(solution.routes[0]) # [D, C1, D]

    """

    @overload
    def __init__(self, evaluation: Evaluation, instance: Instance, number_of_routes: int) -> None:
        """
        Creates a new Solution object with the specified number of empty routes.

        :param Evaluation evaluation: The evaluation object for cost and feasibility calculations.
        :param Instance instance: The Instance object representing the problem instance.
        :param int number_of_routes: The number of empty routes the solution should contain.
        """
        ...

    @overload
    def __init__(self, evaluation: Evaluation, instance: Instance, routes: List[Route]) -> None:
        """
        Creates a new Solution object with the specified list of routes.

        :param Evaluation evaluation: The evaluation object for cost and feasibility calculations.
        :param Instance instance: The Instance object representing the problem instance.
        :param List[Route] routes: The list of routes to include in the solution.
        """
        ...

    def add_route(self, route: Optional[Route] = None) -> None:
        """
        Adds a new route to the solution. If no route is provided, an empty route will be added.

        :param Optional[Route] route: The route to add to the solution (optional).
        """
        ...

    @overload
    def exchange_segment(self, first_route: Route, first_route_begin_position: int, first_route_end_position: int,
                         second_route: Route, second_route_begin_position: int,
                         second_route_end_position: int) -> None:
        """
        Exchanges segments between two routes using Route objects.

        :param Route first_route: The first route in the exchange operation.
        :param int first_route_begin_position: The start position of the segment in the first route.
        :param int first_route_end_position: The end position of the segment in the first route.
        :param Route second_route: The second route in the exchange operation.
        :param int second_route_begin_position: The start position of the segment in the second route.
        :param int second_route_end_position: The end position of the segment in the second route.
        """
        ...

    @overload
    def exchange_segment(self, first_route_index: int, first_route_begin_position: int, first_route_end_position: int,
                         second_route_index: int, second_route_begin_position: int,
                         second_route_end_position: int) -> None:
        """
        Exchanges segments between two routes using their indices.

        :param int first_route_index: The index of the first route in the exchange operation.
        :param int first_route_begin_position: The start position of the segment in the first route.
        :param int first_route_end_position: The end position of the segment in the first route.
        :param int second_route_index: The index of the second route in the exchange operation.
        :param int second_route_begin_position: The start position of the segment in the second route.
        :param int second_route_end_position: The end position of the segment in the second route.
        """
        ...

    def find(self, vertex_id: int) -> List[NodeLocation]:
        """
        Finds the locations of a vertex in the solution.

        :param int vertex_id: The vertex ID to search for.
        :return: A list of NodeLocation objects representing the locations of the vertex in the solution.
        :rtype: List[NodeLocation]
        """
        ...

    def insert_vertex_after(self, after_location: NodeLocation, vertex_id: int) -> int:
        """
        Inserts a vertex after the specified location.

        :param NodeLocation after_location: The location after which to insert the vertex.
        :param int vertex_id: The vertex ID to insert.
        :return: The position of the newly inserted vertex.
        :rtype: int
        """
        ...

    def insert_vertices_after(self, vertex_ids_and_positions: Iterable[Tuple[VertexID, NodeLocation]]) -> None:
        """
        Inserts multiple vertices at the specified locations.

        :param Iterable[Tuple[VertexID, NodeLocation]] vertex_ids_and_positions: An iterable of tuples containing the vertex ID and the location after which to insert the vertex.
        """
        ...

    def lookup(self, location: NodeLocation) -> Node:
        """
        Retrieves the node at the specified location.

        :param NodeLocation location: The location of the node to retrieve.
        :return: The node at the specified location.
        :rtype: Node
        """
        ...

    def remove_route(self, route: Route) -> None:
        """
        Removes the specified route from the solution.

        :param Route route: The route to remove from the solution.
        """
        ...

    def remove_vertex(self, location: NodeLocation) -> None:
        """
        Removes the vertex at the specified location.

        :param NodeLocation location: The location of the vertex to remove.
        """
        ...

    def remove_vertices(self, locations: List[NodeLocation]) -> None:
        """
        Removes multiple vertices at the specified locations.
        This is more efficient than calling remove_vertex() multiple times.

        :param List[NodeLocation] locations: A list of locations of the vertices to remove.
        """
        ...

    def copy(self) -> Solution:
        """
        Creates a copy of the solution. This copies routes and nodes.

        :return: A copy of the solution.
        :rtype: Solution
        """
        ...

    def __copy__(self) -> Solution:
        """
        Creates a copy of the solution. This copies routes and nodes.

        :return: A copy of the solution.
        :rtype: Solution
        """
        ...

    def __deepcopy__(self, memodict: dict = {}) -> Solution:
        """
        Creates a copy of the solution. This copies routes and nodes.

        :param dict memodict: A memoization dictionary to store copies of objects (Optional).
        :return: A copy of the solution.
        :rtype: Solution
        """
        ...

    def __delitem__(self, route_index: int) -> None:
        """
        Removes the route at the specified index.

        :param int route_index: The index of the route to remove.
        """
        ...

    def __eq__(self, other: Solution) -> bool:
        """
        Determines if the solution is equal to another solution.

        :param Solution other: The other solution to compare.
        :return: True if the solutions are equal, False otherwise.
        :rtype: bool
        """
        ...

    def __getitem__(self, route_index: int) -> Route:
        """
        Retrieves the route at the specified index.

        :param int route_index: The index of the route to retrieve.
        :return: The route at the specified index.
        :rtype: Route
        """
        ...

    def __iter__(self) -> Iterator:
        """
        Returns an iterator over the routes in the solution.

        :return: An iterator over the routes.
        :rtype: Iterator
        """
        ...

    def __len__(self) -> int:
        """
        Returns the number of routes in the solution.

        :return: The number of routes in the solution.
        :rtype: int
        """
        ...

    def __ne__(self, other: Solution) -> bool:
        """
        Determines if the solution is not equal to another solution.

        :param Solution other: The other solution to compare.
        :return: True if the solutions are not equal, False otherwise.
        :rtype: bool
        """
        ...

    @property
    def cost(self) -> float:
        """
        Gets the total cost of the solution, i.e., the sum of costs of all routes.

        :return: The total cost of the solution.
        :rtype: float
        """
        ...

    @property
    def cost_components(self) -> List[float]:
        """
        Gets the cost components of the solution, i.e., the sum of costs components of all routes.

        :return: A list of cost components for the solution.
        :rtype: List[float]
        """
        ...

    @property
    def feasible(self) -> bool:
        """
        Determines if the solution is feasible.

        :return: True if the solution is feasible, False otherwise.
        :rtype: bool
        """
        ...

    @property
    def insertion_points(self) -> List[NodeLocation]:
        """
        Gets all possible insertion points in the solution. That is, for each route r, positions [0, len(r) - 2].

        :return: A list of NodeLocation objects representing the insertion points in the solution.
        :rtype: List[NodeLocation]
        """
        ...

    @property
    def non_depot_nodes(self) -> List[NodeLocation]:
        """
        Gets the non-depot nodes in the solution. Useful for removing nodes from the solution.

        :return: A list of NodeLocation objects representing the non-depot nodes in the solution.
        :rtype: List[NodeLocation]
        """
        ...

    @property
    def number_of_insertion_points(self) -> int:
        """
        Gets the number of insertion points in the solution.

        :return: The number of insertion points in the solution.
        :rtype: int
        """
        ...

    @property
    def number_of_non_depot_nodes(self) -> int:
        """
        Gets the number of non-depot nodes in the solution.

        :return: The number of non-depot nodes in the solution.
        :rtype: int
        """
        ...

    @property
    def routes(self) -> Iterator:
        """
        Returns an iterator over the routes in the solution.

        :return: An iterator over the routes.
        :rtype: Iterator
        """
        ...
