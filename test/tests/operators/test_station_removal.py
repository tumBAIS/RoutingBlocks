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
    evaluation.overcharge_penalty_factor = 0.
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
