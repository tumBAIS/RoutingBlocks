from pathlib import Path

from .parsing import parse_instance, create_instance
from .ils import iterated_local_search

import routingblocks as rb


def solve(instance_path: Path):
    vertices, arcs, params = parse_instance(instance_path)
    instance = create_instance(vertices, arcs)
    vehicle_storage_capacity = params['C']
    # Vehicle battery capacity in units of time:
    # battery capacity * inverse refueling rate = battery capacity / refueling rate
    vehicle_battery_capacity_time = params['Q'] * params['g']

    solution = iterated_local_search(instance=instance, vehicle_storage_capacity=vehicle_storage_capacity,
                                     vehicle_battery_capacity_time=vehicle_battery_capacity_time)

    print("Best solution:")
    print(solution)


if __name__ == '__main__':
    solve(Path('evrptw/instances/evrptw/5/c101C5.txt'))
