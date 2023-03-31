from __future__ import annotations

import itertools
import math
import random
import time
from collections import defaultdict
from pathlib import Path
from typing import Tuple, Callable, Dict, List, Iterable

import pytest

import helpers

from fixtures import *

try:
    import routingblocks as alns
    from routingblocks import adptw
    from routingblocks import niftw
except ModuleNotFoundError:
    pass


def run_local_search(local_search: alns.LocalSearch, solution: alns.Solution, operators):
    local_search.optimize(solution, operators)


def create_operators(instance):
    return [
        alns.SwapOperator_0_1(instance, None),
        alns.SwapOperator_0_2(instance, None),
        alns.SwapOperator_1_1(instance, None),
        alns.SwapOperator_1_2(instance, None),
        alns.InterRouteTwoOptOperator(instance, None)
    ]


@pytest.mark.benchmark(group="local-search")
@pytest.mark.parametrize("instance_name", ['c101_21.txt', 'r101_21.txt'])
@pytest.mark.parametrize("evaluation", ['adptw_native', 'niftw_native'])
def test_local_search_benchmark(instance_parser, instance_name,
                                random_solution_factory, evaluation, benchmark):
    py_instance, instance = instance_parser(instance_name)
    vehicle_battery_capacity = py_instance.parameters.battery_capacity_time
    vehicle_storage_capacity = py_instance.parameters.capacity
    evaluations = {
        'adptw_native': alns.adptw.Evaluation(vehicle_battery_capacity, vehicle_storage_capacity),
        'niftw_native': alns.niftw.Evaluation(vehicle_battery_capacity, vehicle_storage_capacity, 0)
    }
    evaluation = evaluations[evaluation]

    # Create routes
    randgen = random.Random(0)
    solution = random_solution_factory(instance, evaluation, randgen=randgen)
    local_search = alns.LocalSearch(instance, evaluation, None)
    # Add operators
    operators = create_operators(instance)
    benchmark(run_local_search, solution=solution, local_search=local_search, operators=operators)
