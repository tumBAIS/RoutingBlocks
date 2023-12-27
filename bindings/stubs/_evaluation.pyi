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

from typing import Any, List, Tuple, overload


class Evaluation:
    """
    The evaluation class implements problem-specific cost and move evaluation functions. It's design bases on the
    concepts introduced in :cite:t:`VidalCrainicEtAl2014`.
    Note that this class is an interface: it's not meant to be instantiated or used directly. Please use the concrete
    implementations of this interface and helper functions instead.
    """

    def __init__(self) -> None: ...

    def compute_cost(self, label: AnyForwardLabel) -> float:
        """
        Computes the cost of a given label.

        :param label: The label
        :return: The cost of the label
        :rtype: float
        """
        ...

    def evaluate(self, instance: Instance,
                 segments: List[List[Tuple[Vertex, AnyForwardLabel, AnyBackwardLabel]]]) -> float:
        """
        Evaluates the cost of the route given by concatenating the passed route sub-sequences. Each sub-sequences is
        guaranteed to have valid forward and backward labels.

        Corresponds to ``EVALN`` in :cite:t:`VidalCrainicEtAl2014`.

        :param instance: The instance
        :param segments: A list of route sub-sequences, each given as a list of tuples of (vertex, forward label, backward label)
        :return: The cost of the route
        :rtype: float
        """
        ...

    def create_backward_label(self, vertex: Vertex) -> AnyBackwardLabel:
        """
        Creates and initializes a backward label for the given vertex.

        Corresponds to ``INIT`` in :cite:t:`VidalCrainicEtAl2014`.

        :param vertex: The vertex representing singleton route [vertex]
        :return: The initialized backward label
        :rtype: AnyBackwardLabel
        """
        ...

    def create_forward_label(self, vertex: Vertex) -> AnyForwardLabel:
        """
        Creates and initializes a forward label for the given vertex.

        Corresponds to ``INIT`` in :cite:t:`VidalCrainicEtAl2014`.

        :param vertex: The vertex representing singleton route [vertex]
        :return: The initialized backward label
        :rtype: AnyBackwardLabel
        """
        ...

    def get_cost_components(self, label: AnyForwardLabel) -> List[float]:
        """
        Returns the cost components of a given label.

        :param label: The label
        :return: The cost components of the label
        :rtype: List[float]
        """
        ...

    def is_feasible(self, label: AnyForwardLabel) -> bool:
        """
        Checks whether a given label is feasible.

        :param label: The label
        :return: True if the label is feasible, False otherwise
        :rtype: bool
        """
        ...

    def propagate_forward(self, pred_label: AnyForwardLabel, pred_vertex: Vertex, vertex: Vertex,
                          arc: Arc) -> AnyForwardLabel:
        """
        Extends the partial route represented by the forward label of pred_vertex to the vertex.

        Corresponds to ``FORW([..., pred_vertex]⊕[vertex])`` in :cite:t:`VidalCrainicEtAl2014`.

        :param pred_label: The forward label of the predecessor vertex
        :param pred_vertex: The predecessor vertex
        :param vertex: The vertex
        :param arc: The arc connecting pred_vertex and vertex
        :return: The propagated forward label
        :rtype: AnyForwardLabel
        """
        ...

    def propagate_backward(self, succ_label: AnyBackwardLabel, succ_vertex: Vertex, vertex: Vertex,
                           arc: Arc) -> AnyBackwardLabel:
        """
        Extends the partial route represented by the backward label of succ_vertex to the vertex.

        Corresponds to ``BACK([vertex]⊕[succ_vertex, ...])`` in :cite:t:`VidalCrainicEtAl2014`.

        :param succ_label: The backward label of the successor vertex
        :param succ_vertex: The successor vertex
        :param vertex: The vertex
        :param arc: The arc connecting succ_vertex to vertex. Note that this corresponds to the reverse arc of the forward direction.
        :return: The propagated backward label
        :rtype: AnyBackwardLabel
        """
        ...


class PyEvaluation(Evaluation):
    """
    The PyEvaluation class implements the evaluation interface in pure Python. It's meant to be used as a base class
    for custom python-based evaluation classes.
    """
    ...


class PyConcatenationBasedEvaluation(Evaluation):
    def __init__(self) -> None: ...

    def concatenate(self, fwd: AnyForwardLabel, bwd: AnyBackwardLabel, vertex: Vertex) -> float:
        """
        Specialization of the general evaluation member of :ref:`PyEvaluation` for cases where two route subsequences can
        be concatenated efficiently. Classes who extend this class do not need to implement :ref:`PyEvaluation.evaluate`.

        The concatenation function works as follows:
        Assume that we have two route subsequences ``[v_1, ..., v_n]`` and ``[v_{n+1}, ..., v_m]``. The specialized evaluation
        function first propagates v_n to v_{n+1}, and the calls ``concatenate(fwd_{n+1}, bwd_{n+1}, v_{n+1})`` where ``fwd_{n+1}`` is the
        forward label of ``v_{n+1}`` and ``bwd_{n+1}`` is the backward label of ``v_{n+1}``.

        If the specialized evaluation function is called with several route sub-sequences
        ``[..., v_n], [v_n+1, ... v_{n+m}], [v_{n+m+1}, ...]`` then the first sequence is extended to v_{n+m} and the
        operation reduces to the first case:
        ``concatenate(fwd_{n+m+1}, bwd_{n+m+1}, v_{n+m+1})`` where ``fwd_{n+m+1}`` is the forward label of ``v_{n+m+1}`` and ``bwd_{n+m+1}`` is the backward label of ``v_{n+m+1}``.

        :param fwd: The forward label representing the route sub-sequence ``[v_1, ..., v_n, v_{n+1}]``
        :param bwd: The backward label representing the route sub-sequence ``[v_{n+1}, ..., v_m]``
        :param vertex: The vertex ``v_{n+1}``
        :return: The cost of the route ``[v_1, ..., v_n, v_{n+1}, ..., v_m]``
        :rtype: float
        """
        ...


@overload
def evaluate_insertion(evaluation: Evaluation, instance: Instance, route: Route, after_position: int,
                       vertex_id: VertexID) -> float:
    """
    Evaluates inserting a vertex into a route after the specified position.

    :param evaluation: The evaluation function
    :param instance: The instance
    :param route: The route
    :param after_position: The position after which the vertex is inserted
    :param vertex_id: The id of the vertex to insert
    :return: The cost of the route with the vertex inserted
    :rtype: float
    """
    ...


@overload
def evaluate_insertion(evaluation: Evaluation, instance: Instance, route: Route, after_position: int,
                       vertex: Vertex) -> float:
    """
    Evaluates inserting a vertex into a route after the specified position.

    :param evaluation: The evaluation function
    :param instance: The instance
    :param route: The route
    :param after_position: The position after which the vertex is inserted
    :param vertex: The vertex to insert
    :return: The cost of the route with the vertex inserted
    :rtype: float
    """
    ...


@overload
def evaluate_insertion(evaluation: Evaluation, instance: Instance, route: Route, after_position: int,
                       node: Node) -> float:
    """
    Evaluates inserting a node into a route after the specified position.

    :param evaluation: The evaluation function
    :param instance: The instance
    :param route: The route
    :param after_position: The position after which the vertex is inserted
    :param node: The node to insert
    :return: The cost of the route with the vertex inserted
    :rtype: float
    """
    ...


def evaluate_splice(evaluation: Evaluation, instance: Instance, route: Route, forward_segment_end_pos: int,
                    backward_segment_begin_pos: int) -> float:
    """
    Evaluates splicing two sub-sequences of a route together. The first sub-seuence ends (non-inclusive) at
    ``forward_segment_end_pos`` and the second sub-sequence starts (including) at ``backward_segment_begin_pos``.

    Example:
    Let ``route = [D, C1, C2, C3, C4, C5, C6, D]`` and ``forward_segment_end_pos = 2`` and ``backward_segment_begin_pos = 5``.
    Then the sub-sequences are ``[D, C1]`` and ``[C5, C6, D]``. The splice operation thus evaluates the cost of the route
    ``[D, C1, C5, C6, D]``.

    :param evaluation: The evaluation function
    :param instance: The instance
    :param route: The route
    :param forward_segment_end_pos: The end position of the first sub-sequence (non-inclusive)
    :param backward_segment_begin_pos: The start position of the second sub-sequence
    :return: The cost of the spliced route
    :rtype: float
    """
    ...
