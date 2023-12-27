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
from dataclasses import dataclass
from pathlib import Path
from typing import Tuple, Callable, Dict, List, TypeVar, Union

import pytest

import helpers

from fixtures import *

try:
    import routingblocks as alns
except ModuleNotFoundError:
    pass


def test_random_removal_apply(adptw_instance, mock_evaluation, random_solution_factory, randgen):
    instance: evrptw.Instance = adptw_instance
    operator = alns.operators.RandomRemovalOperator(randgen)
    evaluation = mock_evaluation
    # Throws if removing more customers than are in the solution
    solution = random_solution_factory(instance, evaluation, [next(instance.customers)], n_routes=1)
    with pytest.raises(RuntimeError):
        operator.apply(evaluation, solution, 2)
    # Single customer in solution
    solution = random_solution_factory(instance, evaluation, [next(instance.customers)], n_routes=1)
    removed_vertices = operator.apply(evaluation, solution, 1)
    assert removed_vertices == [next(instance.customers).vertex_id]
    assert all(not instance.get_vertex(x).is_depot for x in removed_vertices)
    # Random tests
    vertices = list(instance.customers) + list(instance.stations)
    for num_cust in range(len(vertices)):
        solution = random_solution_factory(instance,
                                           evaluation,
                                           vertices,
                                           n_routes=instance.fleet_size)
        removed_vertices = operator.apply(evaluation, solution, num_cust)
        assert len(removed_vertices) == num_cust
        assert len(removed_vertices) == len(set(removed_vertices))
        assert all(not instance.get_vertex(x).is_depot for x in removed_vertices)
