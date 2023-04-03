import random
import sys
from typing import List

import routingblocks
from routingblocks.operators import SeedSelector, ClusterMemberSelector, DistanceBasedClusterMemberSelector, \
    ClusterRemovalOperator
from fixtures import *
import pytest


class MockSeedSelector:
    def __init__(self, seed_sequence: List[routingblocks.Vertex]):
        self._next_return_idx = 0
        self._seed_sequence = seed_sequence
        self.calls = []

    def __call__(self, evaluation: routingblocks.Evaluation, solution: routingblocks.Solution,
                 removed_vertices) -> routingblocks.Vertex:
        # Abort when no further seeds are available
        if self._next_return_idx >= len(self._seed_sequence):
            self.calls.append(((evaluation, solution), None))
            raise StopIteration
        next_seed = self._seed_sequence[self._next_return_idx]
        self._next_return_idx += 1
        self.calls.append(((evaluation, solution), next_seed))
        return next_seed


class MockClusterMemberSelector:
    def __init__(self, cluster_sequence: List[List[routingblocks.Vertex]]):
        self._next_return_idx = 0
        self._cluster_sequence = cluster_sequence
        self.calls = []

    def __call__(self, evaluation: routingblocks.Evaluation,
                 solution: routingblocks.Solution, seed: routingblocks.Vertex) -> List[routingblocks.Vertex]:
        next_cluster = self._cluster_sequence[self._next_return_idx]
        self.calls.append(((evaluation, solution, seed), next_cluster))
        self._next_return_idx += 1
        return next_cluster


@pytest.mark.parametrize('raw_routes,expected_clusters', [
    ([  # Routes
         [1, 2, 3],
         [4, 5]
     ],
     [  # Clusters
         # seed, cluster
         (1, [3, 5]),  # Case where the cluster seed is not part of the cluster
         (1, [4]),
         (5, [2])
     ],
    ),
    # Test case with duplicate vertices. Should only be removed if they occur twice in the cluster.
    ([  # Routes
         [1, 2, 6, 3],
         [4, 5, 6, 6, 6]
     ], [
         # seed, cluster
         (1, [6, 5]),  # Case where the cluster seed is not part of the cluster
         (1, [4, 6, 6]),
         (5, [2])
     ])
])
def test_cluster_removal(instance, mock_evaluation, raw_routes, expected_clusters):
    py_instance, instance = instance
    # Tests:
    #   - Mock seed selector should be called to determine the seed
    #   - Mock cluster member selector should be called to determine the cluster members
    #   - Cluster members should be removed
    cluster_member_selector, cluster_removal_operator, seed_selector, solution = setup_solution_and_operator(
        expected_clusters, instance, mock_evaluation, raw_routes)

    # All cluster members should be removed
    expected_removed_customers = [x for cluster in expected_clusters for x in cluster[1]]
    number_of_removed_vertices = len(expected_removed_customers)
    removed_customers = cluster_removal_operator.apply(mock_evaluation, solution, number_of_removed_vertices)

    assert [x[1] for x in seed_selector.calls] == [x[0] for x in expected_clusters]
    assert [x[1] for x in cluster_member_selector.calls] == [x[1] for x in expected_clusters]

    assert removed_customers == expected_removed_customers


def build_distance_function(vertices: List[routingblocks.Vertex], expected_picks: List[routingblocks.Vertex],
                            seed: routingblocks.Vertex, radius: float):
    assert len(expected_picks) < len(vertices)
    max_distance = len(vertices) * 100.
    threshold = max_distance * radius
    distance_matrix = []
    put_max_dist = False
    for idx, i in enumerate(vertices):
        assert i.vertex_id == idx, "Please sort the list of vertices by id and make sure that the ids are consecutive."
        distance_matrix.append([])
        for j in vertices:
            if i == seed and j in expected_picks:
                # Pick a distance that is below the threshold
                dist = random.uniform(0, threshold - sys.float_info.epsilon)
            else:
                # Pick a distance that is certainly above the threshold or equal to it
                if put_max_dist:
                    dist = random.uniform(threshold, max_distance)
                else:
                    dist = max_distance
                    put_max_dist = True
            distance_matrix[i.vertex_id].append(dist)

    def get_distance(v1: routingblocks.Vertex, v2: routingblocks.Vertex):
        return distance_matrix[v1.vertex_id][v2.vertex_id]

    return get_distance


@pytest.mark.parametrize('vertices,seed_vertex_idx,expected_picks_idx,radius', [
    ([routingblocks.Vertex(i, str(i), False, False, None) for i in range(0, 6)],
     0, [0, 1, 2], 0.5),
    ([routingblocks.Vertex(i, str(i), False, False, None) for i in range(0, 6)],
     1, [0, 1, 2], 0.8),
])
def test_cluster_removal_distance_based_selector(vertices, seed_vertex_idx, expected_picks_idx, radius):
    seed_vertex = vertices[seed_vertex_idx]
    expected_pick = [vertices[i] for i in expected_picks_idx]
    get_distance = build_distance_function(vertices, expected_pick, seed_vertex, radius)

    distance_cluster_member_selector = DistanceBasedClusterMemberSelector(vertices, get_distance, radius, radius)
    cluster = distance_cluster_member_selector(None, None, seed_vertex)
    assert len(cluster) == len(expected_pick), f"Expected the cluster to contain the seed and the expected picks.\n" \
                                               f"Got: {[x.str_id for x in cluster]}, expected: {[x.str_id for x in expected_pick]}"
    assert set(cluster) == set(expected_pick), f"Expected the cluster to contain the seed and the expected picks.\n" \
                                               f"Got: {[x.str_id for x in cluster]}, expected: {[x.str_id for x in expected_pick]}"


@pytest.mark.parametrize('raw_routes,expected_clusters', [
    ([  # Routes
         [1, 2, 3],
         [4, 5]
     ],
     [  # Clusters
         # seed, cluster
         (1, [2]),
     ],
    )
])
def test_cluster_removal_stop_iteration(instance, mock_evaluation, raw_routes, expected_clusters):
    py_instance, instance = instance
    # Test if cases where no further clusters can be selected work

    cluster_member_selector, cluster_removal_operator, seed_selector, solution = setup_solution_and_operator(
        expected_clusters, instance, mock_evaluation, raw_routes)

    # All cluster members should be removed
    expected_removed_customers = [x for cluster in expected_clusters for x in cluster[1]]
    # Request to remove more vertices than can be provided by the seed selector/cluster member selector
    number_of_removed_vertices = len(expected_removed_customers) + 1
    removed_customers = cluster_removal_operator.apply(mock_evaluation, solution, number_of_removed_vertices)

    # Should be called for the first valid cluster and then again for the StopIteration case (None)
    assert [x[1] for x in seed_selector.calls] == [x[0] for x in expected_clusters] + [None]
    assert [x[1] for x in cluster_member_selector.calls] == [x[1] for x in expected_clusters]

    assert removed_customers == expected_removed_customers


def setup_solution_and_operator(expected_clusters, instance, mock_evaluation, raw_routes):
    seed_selector = MockSeedSelector([x[0] for x in expected_clusters])
    cluster_member_selector = MockClusterMemberSelector([x[1] for x in expected_clusters])
    cluster_removal_operator = ClusterRemovalOperator(seed_selector, cluster_member_selector)
    solution = create_solution(instance, mock_evaluation, raw_routes)
    return cluster_member_selector, cluster_removal_operator, seed_selector, solution
