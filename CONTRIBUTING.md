# Contributing

## Building from source

`RoutingBlocks` uses [scikit-build](https://github.com/scikit-build/), so building is as simple as executing:

````bash
pip install .
````

Add a `-v` flag to debug any compile-time errors.

## Running tests

First, make sure to install all optional test dependencies:

```bash
pip install '.[test]'
```

Then navigate to `test/` and run

```bash
pytest tests -m "not benchmark"
```

## Running benchmarks

Follow the procedure from "Running tests", then execute:

```bash
pytest tests -m "benchmark"
```

See the [pytest-benchmark docs](https://pytest-benchmark.readthedocs.io/en/latest/usage.html) for possible command line options.