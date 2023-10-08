class Random:
    @overload
    def __init__(self) -> None:
        """
        Initializes a new instance of the Random class without a seed.

        If no seed value is provided, it uses the current system time or another
        system-specific source of randomness to generate random numbers.
        """

    @overload
    def __init__(self, seed: int) -> None:
        """
        Initializes a new instance of the Random class with a provided seed.

        The seed is a number used to initialize the underlying pseudo-random
        number generator.

        :param int seed: The seed value for the random number generator. Providing the
                     same seed will generate the same sequence of random numbers.
        """

    def randint(self, min: int, max: int) -> int:
        """
        Returns a random integer from the specified range [min, max], including
        both endpoints.

        :param int min: The lower bound of the range.
        :param int max: The upper bound of the range.

        :return: A random integer value from the specified range [min, max].
        """

    def uniform(self, min: float, max: float) -> float:
        """
        Returns a random floating-point number between the specified min and max values,
        including min and potentially up to max.

        :param float min: The lower bound of the range.
        :param float max: The upper bound of the range.
        :return: A random floating-point number within the specified range [min, max).
        """
