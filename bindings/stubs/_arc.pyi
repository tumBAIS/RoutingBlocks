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

from typing import Any


class Arc:
    """
    A simple arc object that represents a connection between two vertices in a graph. The arc stores additional data
    transparent to the RoutingBlocks package. This data can be used to store additional information about the arc, such as
    distances, durations, or any other attributes relevant to the problem being modeled.
    """

    def __init__(self, data: Any) -> None:
        """
        :param data: Additional data associated with the arc.
        """
        ...

    @property
    def data(self) -> Any:
        """
        Retrieves the arc data.

        :return: The data associated with the arc.
        :rtype: Any
        """

    def __str__(self) -> str:
        """
        Generates a human-readable string representation of the arc.

        :return: A string representation of the arc.
        :rtype: str
        """
        ...
