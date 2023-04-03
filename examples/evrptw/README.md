# EVRPTW-PR

This directory contains an example implementation of an ALNS-based algorithm for the EVRPTW-PR problem.
To run the example, first install the requirements:

```bash
pip install -r requirements.txt
```

Then, cd to the parent directory and run the example with

```bash
python -m evrptw instances/evrptw/100/c101_21.txt evrptw/config.json --max-iterations=5000 --max-iterations-since-last-improvement=1100
```

Run

```bash
python -m evrptw -h
```

for a list of supported command line arguments.