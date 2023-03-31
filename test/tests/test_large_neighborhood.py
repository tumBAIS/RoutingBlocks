from __future__ import annotations

import pytest

import helpers

from fixtures import *

try:
    import routingblocks as evrptw


    class MockDestroyOperator(evrptw.DestroyOperator):
        def __init__(self, num):
            evrptw.DestroyOperator.__init__(self)
            self.num = num
            self.ops = []

        def can_apply_to(self, solution: evrptw.Solution) -> bool:
            self.ops.append(['can_apply_to', self.num])
            return True

        def name(self) -> str:
            self.ops.append(['name', self.num])
            return 'TestDestroyOperator'

        def apply(self, solution, num_cust, penalty) -> List[int]:
            self.ops.append(['apply', self.num])
            return []


    class MockRepairOperator(evrptw.RepairOperator):
        def __init__(self, num):
            evrptw.RepairOperator.__init__(self)
            self.num = num
            self.ops = []

        def can_apply_to(self, solution: evrptw.Solution) -> bool:
            self.ops.append(['can_apply_to', self.num])
            return True

        def name(self) -> str:
            self.ops.append(['name', self.num])
            return 'TestRepairOperator'

        def apply(self, solution: evrptw.Solution, missing_vertices: List[int], penalty):
            self.ops.append(['apply', self.num])
except ModuleNotFoundError:
    pass


def test_large_neighborhood_empty(instance, random_solution_factory, mock_evaluation, randgen: random.Random):
    py_instance, instance = instance
    solution = random_solution_factory(instance, mock_evaluation)
    large_neighborhood = evrptw.AdaptiveLargeNeighborhood(randgen, 0.2)
    with pytest.raises(RuntimeError):
        large_neighborhood.generate(mock_evaluation, solution, 1)

    # Add single destory/single repair operator
    large_neighborhood.add_destroy_operator(MockDestroyOperator(1))
    with pytest.raises(RuntimeError):
        large_neighborhood.generate(mock_evaluation, solution, 1)

    large_neighborhood = evrptw.AdaptiveLargeNeighborhood(randgen, 0.2)
    large_neighborhood.add_repair_operator(MockRepairOperator(1))
    with pytest.raises(RuntimeError):
        large_neighborhood.generate(mock_evaluation, solution, 1)


def test_large_neighborhood_custom_operators(instance, random_solution_factory, mock_evaluation,
                                             randgen: evrptw.Random):
    py_instance, instance = instance
    solution = random_solution_factory(instance, mock_evaluation)
    large_neighborhood = evrptw.AdaptiveLargeNeighborhood(randgen, 0.2)

    destroy_id = random.random()
    destroy_operator = MockDestroyOperator(destroy_id)
    repair_id = random.random()
    repair_operator = MockRepairOperator(repair_id)
    assert destroy_operator.num == destroy_id
    assert repair_id == repair_operator.num

    large_neighborhood.add_destroy_operator(destroy_operator)
    assert destroy_operator.ops == []

    large_neighborhood.add_repair_operator(repair_operator)
    assert repair_operator.ops == []

    large_neighborhood.generate(mock_evaluation, solution, 1)
    assert destroy_operator.ops == [['can_apply_to', destroy_id], ['apply', destroy_id]]
    assert repair_operator.ops == [['can_apply_to', repair_id], ['apply', repair_id]]


def test_large_neighborhood_custom_operators_lifetime(instance, random_solution_factory, mock_evaluation,
                                                      randgen: evrptw.Random):
    py_instance, instance = instance
    solution = random_solution_factory(instance, mock_evaluation)
    large_neighborhood = evrptw.AdaptiveLargeNeighborhood(randgen, 0.2)

    # Python frees the reference to Mock operators
    large_neighborhood.add_destroy_operator(MockDestroyOperator(0))
    large_neighborhood.add_repair_operator(MockRepairOperator(0))
    # This should still work - the custom python object should still be valid
    large_neighborhood.generate(mock_evaluation, solution, 1)


def test_large_neighborhood_remove(randgen):
    large_neighborhood = evrptw.AdaptiveLargeNeighborhood(randgen, 0.2)

    destroy_id = random.random()
    destroy_operator = MockDestroyOperator(destroy_id)
    repair_id = random.random()
    repair_operator = MockRepairOperator(repair_id)

    added_destroy_operator = large_neighborhood.add_destroy_operator(destroy_operator)
    added_repair_operator = large_neighborhood.add_repair_operator(repair_operator)

    assert id(added_destroy_operator) == id(destroy_operator)
    assert id(added_repair_operator) == id(repair_operator)

    assert list(large_neighborhood.repair_operators) == [repair_operator]
    assert list(large_neighborhood.destroy_operators) == [destroy_operator]

    large_neighborhood.remove_destroy_operator(destroy_operator)
    assert list(large_neighborhood.destroy_operators) == []
    assert list(large_neighborhood.repair_operators) == [repair_operator]

    large_neighborhood.add_destroy_operator(destroy_operator)
    large_neighborhood.remove_repair_operator(repair_operator)
    assert list(large_neighborhood.repair_operators) == []
    assert list(large_neighborhood.destroy_operators) == [destroy_operator]
