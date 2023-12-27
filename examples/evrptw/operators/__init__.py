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

import routingblocks
from evrptw.operators.ShawMoveSelector import ShawMoveSelector
from evrptw.operators.ShawRelatedness import ShawRelatedness
from evrptw.operators.SpatioTemporalRelatedness import SpatioTemporalRelatedness
from routingblocks.operators.related_removal import RelatedRemovalOperator, build_relatedness_matrix
from routingblocks.operators.move_selectors import random_selector_factory, first_move_selector


def create_shaw_remove_operator(py_instance, cpp_instance, randgen: routingblocks.Random, distance_weight=1.0,
                                demand_weight=1.0, time_weight=1.0, shaw_exponent=1.0):
    relatedness_matrix = build_relatedness_matrix(cpp_instance,
                                                  ShawRelatedness(py_instance, cpp_instance,
                                                                  distance_weight=distance_weight,
                                                                  demand_weight=demand_weight, time_weight=time_weight))
    return RelatedRemovalOperator(
        relatedness_matrix=relatedness_matrix,
        move_selector=ShawMoveSelector(py_instance, cpp_instance, randgen, shaw_exponent=shaw_exponent),
        seed_selector=random_selector_factory(randgen),
        initial_seed_selector=first_move_selector)


def create_related_remove_operator(py_instance, cpp_instance, randgen: routingblocks.Random, tw_shift_weight,
                                   slack_weight):
    relatedness_matrix = build_relatedness_matrix(cpp_instance,
                                                  SpatioTemporalRelatedness(py_instance, cpp_instance,
                                                                            slack_weight=slack_weight,
                                                                            tw_shift_weight=tw_shift_weight))
    return RelatedRemovalOperator(
        relatedness_matrix=relatedness_matrix,
        move_selector=first_move_selector,
        seed_selector=random_selector_factory(randgen),
        initial_seed_selector=first_move_selector)
