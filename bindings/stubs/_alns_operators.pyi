class _RandomRemovalOperator(DestroyOperator):
    """
    Removes random vertices from the solution. Note that the same vertex may apppear several times, i.e., if two
    visits to the same vertex are removed.
    """
    def __init__(self, random: Random) -> None:
        """
        :param Random random: The random number generator used.
        """

class _RandomInsertionOperator(RepairOperator):
    """
    Inserts vertices at random positions in the solution.
    """
    def __init__(self, random: Random) -> None:
        """
        :param Random random: The random number generator used.
        """
