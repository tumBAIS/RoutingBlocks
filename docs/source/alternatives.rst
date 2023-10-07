Alternatives
================

There exist a variety of libraries that deal with solving combinatorial optimization problems in general, and vehicle routing problems in particular.
The following flowchart gives an example of how to choose the right library for your problem.
We have limited ourselves to libraries that base on native implementations and expose a Python interface. We do not include any commercial libraries or software.

.. note::

    Our recommendation bases on the contributors' personal opinions. It is not meant to be a comprehensive list of all available libraries, nor is it meant to be a complete list of all features of the libraries mentioned.

.. mermaid::
    :align: center

    graph TD;
        A{Exact solution</br>desired}
        B{"CVRP(-TW)"}
        C("<a href='https://vrpsolvereasy.readthedocs.io/en/latest'>PyVRPEasy</a>")
        D("<a href='https://pyvrp.readthedocs.io/en/latest/'>PyVRP</a>")
        E("RoutingBlocks")

        A -->|yes| B
        A -->|no| C
        B -->|yes| D
        B -->|no| E

The main difference between RoutingBlocks and `https://pyvrp.readthedocs.io/en/latest/ <PyVRP>`_ relates to the level of control offered by the library. PyVRP is a wrapper around `https://github.com/vidalt/HGS-CVRP/ <HGS-CVRP>`_, a monolithic algorithm implemented in C++. As such, it is limited to a select set of problems, i.e., CVRP, CVRP-TW, and Prize-collecting variants.
It does not allow to implement custom evaluation functions, operators, or other components of the algorithm. RoutingBlocks, on the other hand, is a framework that is built with this very use-case in mind. This of course comes with a price: RoutingBlocks is not as fast as PyVRP, and it requires more effort to implement a (custom) solution algorithm.

We note that RoutingBlocks can technically utilize any other library in it's components. This requires converting the solution and instance representations of the other library to the one used by RoutingBlocks and vice versa.
A potential use-case could be a matheuristic that (repeatably) decomposes a large vehicle routing problem into several subproblems solvable exactly by a library like `https://pyvrp.readthedocs.io/en/latest/ <PyVRPEasy>`_.