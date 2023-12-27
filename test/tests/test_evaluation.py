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

from __future__ import annotations

import itertools
import random
from collections import defaultdict
from pathlib import Path
from typing import Tuple, Callable, Dict, List, Iterable, Union

import pytest

import helpers

from fixtures import *

import sys

try:
    import routingblocks as evrptw
except ModuleNotFoundError:
    pass


def create_segments(sequence):
    segments = []
    prev_segment_end = 1
    while True:
        next_segment_end = prev_segment_end + random.randint(1, len(sequence) - prev_segment_end - 1)

        segments.append((prev_segment_end, next_segment_end))

        prev_segment_end = next_segment_end
        if prev_segment_end == len(sequence) - 1:
            break
    return segments


try:
    class MockLabel:
        def __init__(self, msg):
            self.msg = msg

        def __repr__(self):
            return self.msg

        def __str__(self):
            return self.msg


    class MockEvaluation(evrptw.PyConcatenationBasedEvaluation):
        def __init__(self):
            evrptw.PyConcatenationBasedEvaluation.__init__(self)

        def propagate_forward(self, *args) -> MockLabel:
            return MockLabel("forward")

        def propagate_backward(self, *args) -> MockLabel:
            return MockLabel("backward")

        def create_forward_label(self, *args) -> MockLabel:
            return MockLabel("forward_created")

        def create_backward_label(self, *args) -> MockLabel:
            return MockLabel("backward_created")

        def concatenate(self, *args) -> float:
            return 0

        def compute_cost(self, arg0) -> float:
            return 0
except:
    pass


def test_evaluation_lifetime(instance):
    _, instance = instance
    eval = MockEvaluation()
    route = evrptw.Route(eval, instance)
    # Evaluation should still work even after freeing the object on the python side
    route.update()
    del eval
    route.update()
