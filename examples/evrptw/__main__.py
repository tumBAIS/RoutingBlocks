import json
import sys
from pathlib import Path
import random
import click

import routingblocks
from routingblocks import adptw as adptw

from .alns import ALNS
from .parameters import ALNSParams
from .instance import parse_evrptw_instance, create_cpp_instance


def compute_initial_penalties(instance: routingblocks.Instance):
    max_demand = max([customer.demand for customer in instance.customers])
    max_dist = max(x.cost for x in instance.arcs.values())
    return [1.0, max_dist / max_demand, 10., 10.]


def parse_config(config_path: Path):
    with config_path.open() as f:
        data = json.load(f)
        return ALNSParams(**data)


def format_solution(solution: routingblocks.Solution):
    return {
        'cost': solution.cost,
        'feasible': solution.feasible,
        'number_of_vehicles': sum(1 for r in solution if not r.empty),
        'routes': [
            {
                'cost': route.cost,
                'feasible': route.feasible,
                'cost_components': route.cost_components,
                'nodes': [node.vertex_strid for node in route]
            }
            for route in solution.routes
        ]
    }


@click.command('evrptw')
@click.argument('instance-path', type=click.Path(exists=True, dir_okay=False, file_okay=True))
@click.option('--config-path', type=click.Path(exists=True, dir_okay=False, file_okay=True), required=True)
@click.option('--output-path', type=click.Path(exists=True, dir_okay=True, file_okay=False), default=Path('.'))
@click.option('--seed', type=int, default=None)
@click.option('--time-limit', type=int, default=300)
@click.option('--max-iterations', type=int, default=None)
@click.option('--max-iterations-since-last-improvement', type=int, default=None)
def main(instance_path: Path, config_path: Path, output_path: Path, seed: int, time_limit: int, max_iterations: int,
         max_iterations_since_last_improvement: int):
    instance_path = Path(instance_path)
    config_path = Path(config_path)
    output_path = Path(output_path)

    py_instance = parse_evrptw_instance(instance_path)
    instance = create_cpp_instance(py_instance)
    params = parse_config(config_path)

    evaluation = adptw.Evaluation(py_instance.parameters.battery_capacity_time, py_instance.parameters.capacity)

    # Set initial penalties
    evaluation.penalty_factors = compute_initial_penalties(py_instance)

    if seed is None:
        seed = random.randint(0, 10000)
    if max_iterations is None:
        max_iterations = sys.maxsize
    if max_iterations_since_last_improvement is None:
        max_iterations_since_last_improvement = sys.maxsize

    random.seed(seed)
    alns = ALNS(evaluation=evaluation, py_instance=py_instance, cpp_instance=instance, params=params, seed=seed)
    solution = alns.run(time_limit, max_iterations, max_iterations_since_last_improvement)

    with (output_path / f'solution-{instance_path.name.rpartition(".")[0]}.json').open('w') as f:
        output = {'runtime': alns.elapsed, 'total_iterations': alns._iters, 'iterations_since_last_improvement'
        : alns._iters_since_improvement, 'solution': format_solution(solution), 'config': params.__dict__, 'seed': seed}
        json.dump(output, f)


if __name__ == '__main__':
    main()
