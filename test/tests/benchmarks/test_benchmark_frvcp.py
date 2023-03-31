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

import sys

try:
    import routingblocks as alns
    from routingblocks import adptw
except ModuleNotFoundError:
    pass


def run_frvcp_on_routes(routes: List[List[int]], frvcp_factory):
    frvcp = frvcp_factory()
    for route in routes:
        frvcp.optimize(route)


@pytest.mark.benchmark(group="frvcp")
@pytest.mark.parametrize("propagator", ['adptw-cpp'])
def test_frvcp_benchmark_propagators(instance_parser,
                                     random_raw_route_factory, propagator, benchmark):
    py_instance, instance = instance_parser('r101_21.txt')
    factories = {
        'adptw-cpp': lambda: adptw.FRVCP(instance, py_instance.parameters.battery_capacity_time)
    }

    # Create routes
    randgen = random.Random(0)
    routes = [random_raw_route_factory(instance, randgen, include_depot=True) for _ in range(10)]

    benchmark(run_frvcp_on_routes, routes=routes, frvcp_factory=factories[propagator])
