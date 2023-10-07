.. _development:

Development
====================

Setting up a development environment
------------------------------------

Make sure to install the development dependencies. This includes testing and (optionally) documentation dependencies.

.. code-block:: bash

    pip install .[test]
    pip install .[docs]

Running the tests
-----------------

Then run the tests:

.. code-block:: bash

    cd test
    pytest tests

Building documentation
----------------------

.. code-block:: bash

    cd docs
    make html

We recommend `sphinx-autobuild <https://github.com/executablebooks/sphinx-autobuild>`_ for live-reloading the documentation:

.. code-block:: bash

    pip install sphinx-autobuild
    sphinx-autobuild docs/source docs/build/html