Auxiliary algorithms and data structures
==========================================

Algorithms
----------

.. autoapifunction:: routingblocks.utility.sample_positions

InsertionCache
--------------

.. autoapiclass:: routingblocks.InsertionMove
    :members:
    :undoc-members:

.. autoapiclass:: routingblocks.InsertionCache
    :members:
    :undoc-members:

RemovalCache
------------

.. autoapiclass:: routingblocks.RemovalMove
    :members:
    :undoc-members:

.. autoapiclass:: routingblocks.RemovalCache
    :members:
    :undoc-members:

Miscellaneous
-------------

.. autoapiclass:: routingblocks.Random
    :members:
    :undoc-members:
    :class-doc-from: class

    .. py:method:: __init__(self):

        Initializes a new instance of the Random class without a seed.

        If no seed value is provided, it uses the current system time or another
        system-specific source of randomness to generate random numbers.

    .. py:method:: __init__(self, seed: int):

        Initializes a new instance of the Random class with a provided seed.

        The seed is a number used to initialize the underlying pseudo-random
        number generator.

        :param int seed: The seed value for the random number generator. Providing the
                     same seed will generate the same sequence of random numbers.
