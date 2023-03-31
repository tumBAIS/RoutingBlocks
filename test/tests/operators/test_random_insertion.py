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


def test_random_insertion_apply(adptw_instance, mock_evaluation, randgen):
    instance: evrptw.Instance = adptw_instance
    evaluation = mock_evaluation
    operator = alns.RandomInsertionOperator(randgen)
    # Inserts customers at random positions
    customers = list(instance.customers)
    missing_customers = customers[1:]
    solutions = []
    for _ in range(10):
        sol = alns.Solution(evaluation, instance, [alns.create_route(evaluation, instance, [customers[0].vertex_id]),
                                                   alns.Route(evaluation, instance)])
        assert operator.can_apply_to(sol)
        operator.apply(evaluation, sol, [x.vertex_id for x in missing_customers])
        for i in customers:
            assert sol.find(i.vertex_id) != []
        solutions.append(sol)

    # Ensure that at least two solutions do not match
    # And at least one should
    found_missmatch = False
    found_inserted_route_2 = False
    for sol_a, sol_b in itertools.product(solutions, repeat=2):
        if sol_a != sol_b:
            found_missmatch = True
        if not sol_a[1].empty:
            found_inserted_route_2 = True
    if not found_missmatch or not found_inserted_route_2:
        pytest.fail("Random insertion does not behave randomly")
