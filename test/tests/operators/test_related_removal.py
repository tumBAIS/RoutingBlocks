from typing import List
import math
import pytest

import routingblocks
from routingblocks.operators.related_removal import RelatedVertexRemovalMove, RelatedRemovalOperator
from fixtures import *


class MockSeedSelector:
    def __init__(self, expected_moves, cluster_size):
        self.expected_moves = expected_moves
        self._cluster_size = cluster_size
        self.index = 0
        self.num_calls = 0

    def __call__(self, moves):
        assert list(moves) == self.expected_moves[:self.index + 1]
        self.index += self._cluster_size
        self.num_calls += 1
        return self.expected_moves[self.index - self._cluster_size]


class MockInitialSeedSelector:
    def __init__(self, initial_seed):
        self.calls = 0
        self.initial_seed = initial_seed

    def __call__(self, moves):
        self.calls += 1
        return self.initial_seed


class MockMoveSelector:
    def __init__(self):
        self.calls = 0

    def __call__(self, moves):
        self.calls += 1
        return next(iter(moves))


def create_relatedness_matrix(instance, expected_moves: List[RelatedVertexRemovalMove], cluster_size: int):
    relatedness_matrix = [[0.] * instance.number_of_vertices for _ in range(instance.number_of_vertices)]
    for seed_move_index in range(0, len(expected_moves), cluster_size):
        # Vertex should
        for i, next_related_move in enumerate(expected_moves[seed_move_index + 1:seed_move_index + cluster_size + 1]):
            relatedness_matrix[expected_moves[seed_move_index].vertex_id][
                next_related_move.vertex_id] = float(cluster_size - i)

    return relatedness_matrix


@pytest.mark.parametrize('cluster_size', [1, 2])
@pytest.mark.parametrize('raw_routes,expected_moves', [
    ([
         [1, 2],
         [3, 4, 5],
         []
         # Pick 2 as initial seed
         # Then most related vertex 3
         # Then most related vertex 4
     ], [routingblocks.NodeLocation(0, 2), routingblocks.NodeLocation(1, 2), routingblocks.NodeLocation(1, 3)]),
    ([
         [1, 2],
         [3, 4, 5],
         []
         # Pick 2 as initial seed
         # Then most related vertex 3
         # Then most related vertex 4
     ], [routingblocks.NodeLocation(0, 2), routingblocks.NodeLocation(1, 2), routingblocks.NodeLocation(1, 3),
         routingblocks.NodeLocation(1, 1)])
])
def test_related_remove(instance, mock_evaluation, cluster_size, raw_routes, expected_moves):
    py_instance, instance = instance

    solution = create_solution(instance, mock_evaluation, raw_routes)

    number_of_vertices_to_remove = len(expected_moves)
    expected_moves: List[routingblocks.NodeLocation] = expected_moves[:number_of_vertices_to_remove + 1]
    expected_moves: List[RelatedVertexRemovalMove] = [
        RelatedVertexRemovalMove(solution[x.route][x.position].vertex_id, 1., x) for x in expected_moves]

    relatedness_matrix = create_relatedness_matrix(instance, expected_moves, cluster_size)
    mock_initial_seed_selector = MockInitialSeedSelector(expected_moves[0].location)
    mock_seed_selector = MockSeedSelector(expected_moves, cluster_size)
    mock_move_selector = MockMoveSelector()

    operator = RelatedRemovalOperator(relatedness_matrix, mock_move_selector, mock_seed_selector,
                                      mock_initial_seed_selector, cluster_size=cluster_size)
    removed_vertices = operator.apply(mock_evaluation, solution, number_of_vertices_to_remove)
    assert removed_vertices == [x.vertex_id for x in expected_moves]
    assert mock_initial_seed_selector.calls == 1
    # Seeds should be selected once for each cluster
    assert mock_seed_selector.num_calls == math.ceil((len(expected_moves) - 1) / cluster_size)
    # Move selector should be classed once for each vertex to remove except the first one
    assert mock_move_selector.calls == len(expected_moves) - 1
