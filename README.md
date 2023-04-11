# RoutingBlocks

`RoutingBlocks` is an open-source Python package for the implementation of algorithms for Vehicle Routing Problems with
Intermediate Stops.

It provides a set of modular algorithmic components and efficient data structures that can be used as building blocks
for problem-specific metaheuristic algorithms. These components are tailored specifically to tackle the challenges of
VRPIS, but can be used for other classes of vehicle routing problems as well.

## Installation

The package is available on PyPI and can be installed using `pip`:

```bash
pip install routingblocks
```

To obtain the bleeding-edge development version, run

```bash
pip install git+https://github.com/tumBAIS/RoutingBlocks
```

instead.

## Features

* Efficient C++-based solution representation
* Customizable Local Search Solver
* Framework for ALNS-based metaheuristics
* Efficient native implementations of numerous destroy, repair, and local search operators
* Move caches implemented in native code to allow high-performance operator implementations in Python
* Support for custom [native extensions](https://github.com/tumBAIS/routingblocks-native-extension-example)

## Usage

We provide an [example implementation](https://github.com/tumBAIS/RoutingBlocks/tree/main/examples) of an ALNS-based
algorithm for
the [EVRPTW-PR](https://research.sabanciuniv.edu/id/eprint/26033/1/WP_EVRPTW-Partial_Recharge_KeskinCatay.pdf) as part
of this repository.

Further documentation is available at [readthedocs](https://routingblocks.readthedocs.io/en/latest/).

## Contributing

Pull requests are welcome. For major changes, please open an issue first
to discuss what you would like to change.

See [CONTRIBUTING.md](CONTRIBUTING.md) for more information and documentation on setting up a development environment.

## License

[MIT](https://choosealicense.com/licenses/mit/)
