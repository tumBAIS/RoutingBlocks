class Random:
    @overload
    def __init__(self) -> None: ...

    @overload
    def __init__(self, seed: int) -> None: ...

    def randint(self, min: int, max: int) -> int: ...

    def uniform(self, min: float, max: float) -> float: ...


def sample_locations(solution: Solution, randgen: Random, k: int, include_first_depot: bool) -> List[NodeLocation]: ...
