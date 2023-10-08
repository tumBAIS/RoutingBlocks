class Propagator:
    """
    The Propagator class implements problem-specific ordering, dominance, and propagation functions. It's design bases on the
    concepts introduced in :cite:t:`Irnich2008`.
    Note that this class is an interface: it's not meant to be instantiated or used directly. Please use the concrete
    implementations of this interface instead.
    """

    def __init__(self) -> None: ...

    def cheaper_than(self, label: Any, other_label: Any) -> bool:
        """
        Checks whether the label is cheaper than the other label, i.e., has lower cost.

        :param label: The (potentially) cheaper label.
        :param other_label: The (potentially) more expensive label.
        :return: True if the label is cheaper than the other label, False otherwise.
        """
        ...

    def create_root_label(self) -> Any:
        """
        Creates the initial label for the dynamic programming algorithm. Represents the state at the source node, i.e. the
        depot node.
        :return: The initial label.
        """
        ...

    def dominates(self, label: Any, other_label: Any) -> bool:
        """
        Checks whether the label dominates the other label.

        :param label: The (potentially) dominating label.
        :param other_label: The (potentially) dominated label.
        :return:
        """
        ...

    def extract_path(self, label: Any) -> List[VertexID]:
        """
        Extracts the path represented by label, converting it to a list of vertex IDs.

        :param label: The label that represents the path.
        :return: The list of vertex IDs visited on the path.
        """
        ...

    def is_final_label(self, label: Any) -> bool:
        """
        Checks whether the label is the final label, i.e. whether it represents a depot-depot path.
        :param label: The label to check.
        :return: True if the label is final, False otherwise.
        """
        ...

    def order_before(self, label: Any, other_label: Any) -> bool:
        """
        Whether the label should be ordered before the other label. This is used to determine the order in which labels are
        stored in the set of settled labels, which is important for dominance checks: the checks considers only labels that
        would be ordered before the label being checked.

        :param label: The (potentially) earlier label.
        :param other_label: The (potentially) later label.
        :return: True if the label should be ordered before the other label, False otherwise.
        """
        ...

    def prepare(self, route_vertex_ids: List[VertexID]) -> None:
        """
        Prepare the propagator before running the algorithm on the route represented by the given vertex IDs.
        :param route_vertex_ids: The vertex IDs of the route.
        """
        ...

    def propagate(self, label: Any, origin_vertex: Vertex, target_vertex: Vertex, arc: Arc) -> Optional[
        Any]:
        """
        Propagates the label to the target vertex, using the given arc. This creates a new label that represents the state
        at the target vertex. Return None if the resulting path would be infeasible.

        :param label: The label to propagate.
        :param origin_vertex: The origin vertex of the arc.
        :param target_vertex: The target vertex of the arc.
        :param arc: The arc to propagate the label along.
        :return: The propagated label, or None if the resulting path would be infeasible.
        """
        ...


class FacilityPlacementOptimizer:
    """
    Algorithm that inserts visits to replenishment facilities at optimal locations into a route.
    """

    def __init__(self, instance: Instance, propagator: Propagator) -> None:
        """

        :param instance: The instance.
        :param propagator: The propagator to use.
        """
        ...

    def optimize(self, route_vertex_ids: List[VertexID]) -> List[VertexID]:
        """
        Inserts visits to replenishment facilities at optimal locations into the route represented by the given vertex IDs.

        :param route_vertex_ids: The vertex IDs of the route.
        :return: The vertex IDs of the optimized route.
        """
        ...
