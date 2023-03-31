import vrpis
from examples.evrptw.operators.ShawMoveSelector import ShawMoveSelector
from examples.evrptw.operators.ShawRelatedness import ShawRelatedness
from examples.evrptw.operators.SpatioTemporalRelatedness import SpatioTemporalRelatedness
from vrpis.operators.related_removal import RelatedRemovalOperator, build_relatedness_matrix
from vrpis.operators.move_selectors import random_selector_factory, first_move_selector


def create_shaw_remove_operator(py_instance, cpp_instance, randgen: vrpis.Random, distance_weight=1.0,
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


def create_related_remove_operator(py_instance, cpp_instance, randgen: vrpis.Random, tw_shift_weight, slack_weight):
    relatedness_matrix = build_relatedness_matrix(cpp_instance,
                                                  SpatioTemporalRelatedness(py_instance, cpp_instance,
                                                                            slack_weight=slack_weight,
                                                                            tw_shift_weight=tw_shift_weight))
    return RelatedRemovalOperator(
        relatedness_matrix=relatedness_matrix,
        move_selector=first_move_selector,
        seed_selector=random_selector_factory(randgen),
        initial_seed_selector=first_move_selector)
