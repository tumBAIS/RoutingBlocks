# Copyright (c) 2023 Patrick S. Klein (@libklein)
#
# Permission is hereby granted, free of charge, to any person obtaining a copy of
# this software and associated documentation files (the "Software"), to deal in
# the Software without restriction, including without limitation the rights to
# use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
# the Software, and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
# FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
# COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
# IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

from typing import Iterable, Tuple


class Route:
    """
    Routes represent a sequence of visits to vertices, represented by Node objects.
    A route starts and ends at the depot.
    The route class ensures that the information stored in each node's label is consistent with the forward and
    backward sequences.

    Example:
        Route [D, a, b, c, d, e, D], where capital D represents a visit to the depot. Then the route class ensures
        that the forward labels stored on node 'c' represent the sequence [D, a, b, c], that is, are calculated by propagating the forward label at D across arcs (D, a), (a, b), (b, c).
        Similarly, the backward labels stored on node 'c' represent the sequence [c, d, e, D].

    Routes behave like lists of nodes, that is, they can e.g. be indexed and iterated over.

    Routes carry a globally unique modification timestamp which can be used to efficiently test two routes for equality:
    On each modification of the route, the modification timestamp is incremented, while copying a route preserves it's timestamp.
    Hence, two routes with the same modification timestamp are guaranteed to be equal, although the converse does not necessarily apply.
    """

    def __init__(self, evaluation: Evaluation, instance: Instance) -> None:
        """
        The route constructor creates an empty route, that is, a route that contains only start and end depots.
        Refer to create_route for a method to create a route from a sequence of vertex ids.

        :param Evaluation evaluation: The Evaluation object used for cost and feasibility calculations.
        :param Instance instance: The Instance object representing the problem instance.
        """
        ...

    @property
    def cost(self) -> float:
        """
        Calculates the route's cost.

        :return: The cost of the route.
        :rtype: float
        """
        ...

    @property
    def cost_components(self) -> List[float]:
        """
        Calculates the route's cost components.

        :return: A list of cost components .
        :rtype: List[float]
        """
        ...

    @property
    def depot(self) -> Node:
        """
        Retrieves the depot node of the route.

        :return: The start depot node.
        :rtype: Node
        """
        ...

    @property
    def empty(self) -> bool:
        """
        Determines if the route is empty.

        :return: True if the route is empty, False otherwise.
        :rtype: bool
        """
        ...

    @property
    def end_depot(self) -> Node:
        """
        Retrieves the end depot node of the route.

        :return: The end depot node.
        :rtype: Node
        """
        ...

    @property
    def feasible(self) -> bool:
        """
        Determines if the route is feasible.

        :return: True if the route is feasible, False otherwise.
        :rtype: bool
        """
        ...

    @property
    def modification_timestamp(self) -> int:
        """
        Retrieves the modification timestamp of the route.

        :return: The modification timestamp.
        :rtype: int
        """
        ...

    def __iter__(self) -> Iterator:
        """
        Returns an iterator over the nodes in the route.

        :return: An iterator over the nodes in the route.
        :rtype: Iterator
        """
        ...

    def __len__(self) -> int:
        """
        Returns the number of nodes in the route, including the depot nodes.

        :return: The number of nodes in the route, including depot nodes.
        :rtype: int
        """
        ...

    def __eq__(self, arg0: Route) -> bool:
        """
        Compares this route to another route for equality.
        Note that this does not use the route's modification timestamp, but rather compares the nodes in the route.

        :param Route arg0: The route to compare with.
        :return: True if the routes are equal, False otherwise.
        :rtype: bool
        """
        ...

    def __ne__(self, arg0: Route) -> bool:
        """
        Compares this route to another route for inequality.
        Note that this does not use the route's modification timestamp, but rather compares the nodes in the route.

        :param Route arg0: The route to compare with.
        :return: True if the routes are not equal, False otherwise.
        :rtype: bool
        """
        ...

    def exchange_segments(self, segment_begin_position: int, segment_end_position: int,
                          other_segment_begin_position: int,
                          other_segment_end_position: int, other_route: Route) -> None:
        """
        Exchanges the segment [segment_begin_position, segment_end_position) of this route with the segment
        [other_segment_begin_position, other_segment_end_position) of the other route. The swapped segments do not
        include the respective segment end nodes. This method if well defined even if other_route is this route as
        long as the segments do not overlap.

        :param int segment_begin_position: The start position of the segment in this route.
        :param int segment_end_position: The end position of the segment in this route. Not included.
        :param int other_segment_begin_position: The start position of the segment in the other route.
        :param int other_segment_end_position: The end position of the segment in the other route. Not included.
        :param Route other_route: The other route to exchange segments with.
        """
        ...

    def insert_segment_after(self, position: int, node_segment: List[Node]) -> int:
        """
        Inserts a sequence of nodes after the given position in the route.

        :param int position: The position after which to insert the segment.
        :param List[Node] node_segment: The sequence of nodes to insert.
        :return: The new position after the insertion.
        :rtype: int
        """
        ...

    def insert_vertices_after(self, vertices: Iterable[Tuple[VertexID, int]]) -> None:
        """
        Inserts a batch of vertices at the given positions. This method is more efficient than calling insert_segment_after
        multiple times.


        :param Iterable vertices: The iterable containing tuples of vertices with the positions to insert after.
        """
        ...

    def remove_segment(self, begin_position: int, end_position: int) -> int:
        """
        Removes a segment of nodes from the route. Example:

        .. code-block:: python

            route = routingblocks.create_route(evaluation, instance, [D, C1, C2, C3, D])
            new_position_of_end_position = route.remove_segment(1, 3)
            print(route) # [D, C3, D]
            print(new_position_of_end_position) # 1

        :param int begin_position: The start position of the segment to remove.
        :param int end_position: The end position of the segment to remove. Not inclusive.
        :return: The new position of end_position after removal.
        :rtype: int
        """
        ...

    def remove_vertices(self, vertex_positions: List[int]) -> None:
        """
        Removes vertices at the specified positions from the route.

        :param List[int] vertex_positions: The list of vertex positions to remove.
        """
        ...

    def update(self) -> None:
        """
        Updates the route, recalculating the forward and backward labels, costs, and feasibility.
        """
        ...

    def __copy__(self) -> Route:
        """
        Creates a copy of the route. Copies Node objects and labels, but not the underlying Vertex and Instance objects.

        :return: A deep copy of the route.
        :rtype: Route
        """
        ...

    def copy(self) -> Route:
        """
        Creates a copy of the route. Copies Node objects and labels, but not the underlying Vertex and Instance objects.

        :return: A deep copy of the route.
        :rtype: Route
        """
        ...

    def __deepcopy__(self, memodict: Dict = None) -> Route:
        """
        Creates a copy of the route. Copies Node objects and labels, but not the underlying Vertex and Instance objects.
        Same as __copy__.

        :param Dict memodict: A dictionary for memoization (optional).
        :return: A deep copy of the route.
        :rtype: Route
        """
        ...

    def __getitem__(self, position: int) -> Node:
        """
        Retrieves the node at the specified position in the route.

        :param int position: The position of the node.
        :return: The node at the specified position.
        :rtype: Node
        """
        ...


def create_route(evaluation: Evaluation, instance: Instance, vertex_ids: List[int]) -> Route: ...
