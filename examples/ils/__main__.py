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
