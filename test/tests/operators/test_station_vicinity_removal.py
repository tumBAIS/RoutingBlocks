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

import random
import sys
from typing import List

import routingblocks
from routingblocks.operators import StationVicinityRemovalOperator, StationSeedSelector
from fixtures import *
import pytest


@pytest.mark.parametrize('raw_routes,can_apply', [
    ([  # Routes
         [1, 2, 3],
         [4, 5]
     ],
     False),
    ([  # Routes
         [1, 2, 3],
         [4, 5, 6]
     ],
     True),
    ([  # Routes
         [6]
     ],
     True),
])
def test_station_vicinity_removal_apply_to(instance, mock_evaluation, raw_routes, can_apply, randgen):
    py_instance, instance = instance
    solution = create_solution(instance, mock_evaluation, raw_routes)
    station_vicinity_operator = StationVicinityRemovalOperator(instance, get_distance=lambda x, y: 0,
                                                               min_radius_factor=1., max_radius_factor=1.,
                                                               randgen=randgen)
    assert station_vicinity_operator.can_apply_to(solution) == can_apply


@pytest.mark.parametrize('raw_routes,num_removed_vertices', [
    ([  # Routes
         [1, 2, 3],
         [4, 5, 6]
     ],  # Expected removed vertices
     3),
    ([  # Routes
         [1, 2, 3],
         [4, 5]
     ],  # Expected removed vertices
     0),
])
def test_station_vicinity_removal(instance, mock_evaluation, randgen, raw_routes, num_removed_vertices):
    py_instance, instance = instance

    solution = create_solution(instance, mock_evaluation, raw_routes)
    station_vicinity_operator = StationVicinityRemovalOperator(instance,
                                                               get_distance=lambda x, y: py_instance.arcs[
                                                                   x.str_id, y.str_id].distance, min_radius_factor=1.,
                                                               max_radius_factor=1., randgen=randgen)

    removed_vertices = station_vicinity_operator.apply(mock_evaluation, solution, num_removed_vertices)
    assert len(removed_vertices) == num_removed_vertices

    picked_station = instance.get_vertex(6)
    expected_vertices = sorted([x for x in list(instance.customers) + list(instance.stations)],
                               key=lambda x: py_instance.arcs[picked_station.str_id, x.str_id].distance)[
                        :num_removed_vertices]
    assert len(removed_vertices) == len(expected_vertices)
    assert set(x.vertex_id for x in expected_vertices) == set(removed_vertices)
