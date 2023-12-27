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

import random
from typing import List

import pytest

from fixtures import *

try:
    from routingblocks import adptw
except ModuleNotFoundError:
    pass


def run_frvcp_on_routes(routes: List[List[int]], frvcp_factory):
    frvcp = frvcp_factory()
    for route in routes:
        frvcp.optimize(route)


@pytest.mark.benchmark(group="facility-placement-optimizer")
@pytest.mark.parametrize("propagator", ['adptw-cpp'])
def test_frvcp_benchmark_propagators(instance_parser,
                                     random_raw_route_factory, propagator, benchmark):
    py_instance, instance = instance_parser('r101_21.txt')
    factories = {
        'adptw-cpp': lambda: adptw.FacilityPlacementOptimizer(instance, py_instance.parameters.battery_capacity_time)
    }

    # Create routes
    randgen = random.Random(0)
    routes = [random_raw_route_factory(instance, randgen, include_depot=True) for _ in range(10)]

    benchmark(run_frvcp_on_routes, routes=routes, frvcp_factory=factories[propagator])
