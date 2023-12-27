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

from dataclasses import dataclass
from typing import Tuple


@dataclass
class ALNSParams:
    adaptive_smoothing_factor: float = 0.2
    min_removed_customer_percentage: float = 0.1
    max_removed_customer_percentage: float = 0.5
    new_best_feasible_score: float = 9
    new_best_score: float = 4
    new_improvement_score: float = 1

    num_starting_solutions: int = 1

    delta_local_search: float = 0.3
    delta_fpo: float = 0.05

    adaptive_period_length: int = 50
    penalty_period_length: int = 50

    target_feasibility_ratios: Tuple[float] = (0.9, 0.9, 0.9)
    penalty_increase_factor: float = 1.2
    penalty_decrease_factor: float = 0.8
    initial_penalties: Tuple[float] = (100., 100., 100.)

    use_best_improvement: bool = True
    shuffle_operators: bool = False

    # Shaw removal operator
    distance_weight: float = 0.75
    demand_weight: float = 0.1
    time_weight: float = 0.1
    shaw_exponent: float = 4.

    # Related Removal Operator
    tw_shift_weight: float = 1.0
    slack_weight: float = 1.0

    vehicle_decrease_period_length: int = 250
    vehicle_decreased_search_period_length: int = 150

    worst_removal_blink_probability: float = 0.01
    best_insertion_blink_probability: float = 0.15
    granularity: int = 30
