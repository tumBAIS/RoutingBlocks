.. _ADPTW:

ADPTW
==========================================

This module provides efficient implementations to solve ADPTW problems.

Classification
------------------------------------------

The A-D-P-TW class of vehicle routing routing problems with intermediate stops encompassed problems with the following attributes (cf. :cite:t:`SchifferSchneiderEtAl2019`):

- **Arc-based (A):** The consumption of operational resources, e.g. fuel, is continuous and occurs when travelling between nodes.

- **Dependent (D):** The replenishment time is dependent on the quantity of the operational resource being replenished, meaning more time is needed for larger quantities.

- **Partial Replenishment (P):** The operational resources can be partially restocked, implying that the replenishment process can be interrupted at any time, i.e., does not need to fully replenish the resource in question.

- **Time Windows (TW):** The scheduled stops need to be visited within node-specific periods.

In this setting, the routing strategy must consider continuous resource consumption, variable replenishment times, the option for partial replenishment, and adherence to designated time windows.
This combination of factors is one of the most challenging routing problems, and is often encountered in real-world applications, e.g., the Electric Vehicle Routing Problem with Time Windows and Partial Replenishment (EVRP-TW-PR).


API
------------------------------------------

.. autoapimodule:: routingblocks.adptw
   :members:
   :undoc-members:
   :show-inheritance:
