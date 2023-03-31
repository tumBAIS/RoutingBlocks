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
    operator = alns.RandomRemoveOperator(randgen)
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
