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

class RemovalMove:
    vertex_id: VertexID
    node_location: NodeLocation
    delta_cost: float

    def __init__(self, vertex_id: VertexID, node_location: NodeLocation, delta_cost: float) -> None:
        """
        :param vertex_id: The vertex ID of the node to be removed.
        :param node_location: The location of the node to be removed.
        :param delta_cost: The change in cost of the solution if the node is removed.
        """
        ...

    def __eq__(self, other: RemovalMove) -> bool: ...


class RemovalCache:
    def __init__(self, instance: Instance) -> None: ...

    def clear(self) -> None: ...

    def invalidate_route(self, route: Route, route_index: int) -> None: ...

    def rebuild(self, evaluation: Evaluation, solution: Solution) -> None: ...

    @property
    def moves_in_order(self) -> List[RemovalMove]: ...
