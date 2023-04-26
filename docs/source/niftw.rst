.. _NIFTW:

NIFTW
======

This module provides efficient implementations to solve NIFTW problems.

Classification
--------------

The N-I-F-TW class of vehicle routing routing problems with intermediate stops encompassed problems with the following attributes (cf. :cite:t:`SchifferSchneiderEtAl2019`):

- **Node-based (N):** The consumption of operational resources, e.g., goods or materials, occurs at stops or nodes.

- **Independent (I):** The replenishment time is fixed and not dependent on the quantity of the operational resource being replenished, ensuring consistent replenishment durations.

- **Full Replenishment (F):** The operational resources are always fully restocked.

- **Time Windows (TW):** The scheduled stops need to be visited within node-specific periods.

In the N-I-F-TW setting, the routing considerations are centred around node-specific resource consumption, constant replenishment times, the necessity for full replenishment, and strict adherence to time windows.
This scenario is typical in various real-world applications where resources are consumed at stops, replenishment times are consistent, and routes are strictly governed by time constraints, such as in certain types of delivery or service routing problems.


API
---

.. autoapimodule:: routingblocks.niftw
    :members:
    :undoc-members:
    :show-inheritance:
