from pathlib import Path
import argparse

from .parsing import parse_instance, create_instance
from .ils import iterated_local_search


def solve(instance_path: Path, number_of_iterations: int = 100):
    vertices, arcs, params = parse_instance(instance_path)
    instance = create_instance(vertices, arcs)
    vehicle_storage_capacity = params['C']
    # Vehicle battery capacity in units of time:
    # battery capacity * inverse refueling rate = battery capacity / refueling rate
    vehicle_battery_capacity_time = params['Q'] * params['g']

    solution = iterated_local_search(instance=instance, vehicle_storage_capacity=vehicle_storage_capacity,
                                     vehicle_battery_capacity_time=vehicle_battery_capacity_time,
                                     number_of_iterations=number_of_iterations
                                     )

    print("Best solution:")
    print(solution)


def main():
    parser = argparse.ArgumentParser(description="Solve a EVRP-TW-PR instance with ILS.")

    parser.add_argument(
        "instance_path",
        type=Path,
        help="Path to the instance file."
    )

    parser.add_argument(
        "-n", "--number_of_iterations",
        type=int,
        default=100,
        help="Number of ILS iterations to perform (default: 100)."
    )

    args = parser.parse_args()

    solve(Path(args.instance_path), args.number_of_iterations)


if __name__ == '__main__':
    main()
