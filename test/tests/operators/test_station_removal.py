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

from itertools import islice

import pytest
from fixtures import *
import routingblocks
from routingblocks import niftw


def test_station_removal_search(large_instance):
    py_instance, instance = large_instance
    evaluation = niftw.Evaluation(py_instance.parameters.battery_capacity_time,
                                  py_instance.parameters.capacity, 0.)

    # Make sure that there is no penalty for removing stations.
    evaluation.resource_penalty_factor = 0.
    evaluation.overload_penalty_factor = 0.
    evaluation.time_shift_penalty_factor = 0.

    solution = create_solution(instance, evaluation, [
        [station.vertex_id for station in islice(instance.stations, 1, 3)],
        [1, 2, 3, *(station.vertex_id for station in islice(instance.stations, 1, 3))]
    ])

    station_removal_operator = routingblocks.operators.RemoveStationOperator(instance)
    ls = routingblocks.LocalSearch(instance, evaluation, None, routingblocks.FirstImprovementPivotingRule())
    ls.optimize(solution, [station_removal_operator])

    # All stations should be removed.
    assert sum(1 for route in solution for x in route if x.vertex.is_station) == 0
