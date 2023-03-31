from __future__ import annotations

from fixtures import *

try:
    import routingblocks as alns
    from routingblocks import adptw
except ModuleNotFoundError:
    pass


def test_lns_helpers_sample_positions_without_depots(instance, random_solution_factory, mock_evaluation,
                                                     randgen: alns.Random):
    py_instance, instance = instance
    solution = random_solution_factory(instance=instance, evaluation=mock_evaluation)
    num_nodes = sum((len(x) - 2) for x in solution)
    for i in range(0, num_nodes):
        rand_positions = alns.sample_positions(solution, randgen, i, False)
        for rand_pos in rand_positions:
            assert 0 <= rand_pos.route <= len(solution)
            assert 1 <= rand_pos.position <= len(solution[rand_pos.route]) - 1
            # Random position should not be a depot
            assert not solution[rand_pos.route][rand_pos.position].vertex.is_depot

    with pytest.raises(RuntimeError):
        alns.sample_positions(solution, randgen, num_nodes + 1, False)


def test_lns_helpers_sample_positions_with_depot(instance, random_solution_factory, randgen: alns.Random):
    py_instance, instance = instance
    adptw_evaluation = adptw.Evaluation(py_instance.parameters.battery_capacity_time, py_instance.parameters.capacity)
    solution = random_solution_factory(evaluation=adptw_evaluation, instance=instance)
    num_nodes = sum((len(x) - 1) for x in solution)
    for i in range(0, num_nodes):
        rand_positions = alns.sample_positions(solution, randgen, i, True)
        for rand_pos in rand_positions:
            assert 0 <= rand_pos.route <= len(solution)
            assert 0 <= rand_pos.position <= len(solution[rand_pos.route]) - 1

    with pytest.raises(RuntimeError):
        alns.sample_positions(solution, randgen, num_nodes + 1, True)

    # Should pick the depot position
    empty_solution = alns.Solution(adptw_evaluation, instance, 1)
    empty_solution_position = alns.sample_positions(empty_solution, randgen, 1, True)[0]
    assert empty_solution_position.route == 0 and empty_solution_position.position == 0
