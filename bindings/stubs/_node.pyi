class Node:
    """
    A node represents a visit to a vertex.
    It carries forward and backward labels that are used for cost calculation, constraint checking, and efficient move evaluation.
    The data itself is opaque to the node class, and is only used by the evaluation object.
    """

    def __init__(self, vertex: Vertex, fwd_label: AnyForwardLabel, bwd_label: AnyBackwardLabel) -> None:
        """
        :param Vertex vertex: The associated Vertex object.
        :param AnyForwardLabel fwd_label: The forward label for the node.
        :param AnyBackwardLabel bwd_label: The backward label for the node.
        """
        ...

    def cost(self, evaluation: Evaluation) -> float:
        """
        Computes the cost according to the forward label carried by the node using the given evaluation.
        See the documentation of the evaluation object for more information.

        :param Evaluation evaluation: The evaluation object for computing costs.
        :return: The cost.
        :rtype: float
        """
        ...

    def cost_components(self, evaluation: Evaluation) -> List[float]:
        """
        Computes the individual cost components according to the forward label carried by the node using the given evaluation.
        See the documentation of the evaluation object for more information.

        :param Evaluation evaluation: The evaluation object for computing cost components.
        :return: A list of cost components.
        :rtype: List[float]
        """
        ...

    def feasible(self, evaluation: Evaluation) -> bool:
        """
        Determines if the node is feasible based on the given evaluation.
        See the documentation of the evaluation object for more information.

        :param Evaluation evaluation: The evaluation object for checking feasibility.
        :return: True if the node is feasible, False otherwise.
        :rtype: bool
        """
        ...

    def update_backward(self, evaluation: Evaluation, predecessor: Node, arc: Arc) -> None:
        """
        Updates the backward label of the node based on the given evaluation, predecessor node, and arc.
        See the documentation of the evaluation object for more information.

        :param Evaluation evaluation: The evaluation object for updating the backward label.
        :param Node predecessor: The predecessor node in the solution space.
        :param Arc arc: The arc connecting the predecessor node to the current node.
        """
        ...

    def update_forward(self, evaluation: Evaluation, successor: Node, arc: Arc) -> None:
        """
        Updates the forward label of the node based on the given evaluation, successor node, and arc.
        See the documentation of the evaluation object for more information.

        :param Evaluation evaluation: The evaluation object for updating the forward label.
        :param Node successor: The successor node in the solution space.
        :param Arc arc: The arc connecting the current node to the successor node.
        """
        ...

    @property
    def backward_label(self) -> AnyBackwardLabel:
        """
        Retrieves the backward label of the node.

        :return: The backward label.
        :rtype: AnyBackwardLabel
        """
        ...

    @property
    def forward_label(self) -> AnyForwardLabel:
        """
        Retrieves the forward label of the node.

        :return: The forward label.
        :rtype: AnyForwardLabel
        """
        ...

    @property
    def vertex(self) -> Vertex:
        """
        Retrieves the vertex represented by this node.

        :return: The associated Vertex object.
        :rtype: Vertex
        """
        ...

    @property
    def vertex_id(self) -> int:
        """
        Shorthand to retrieve the unique id of the node's vertex.

        :return: The unique identifier of the associated vertex.
        :rtype: int
        """
        ...

    @property
    def vertex_strid(self) -> str:
        """
        Shorthand to retrieve the name of the node's vertex.

        :return: The name of the associated vertex.
        """
        ...
