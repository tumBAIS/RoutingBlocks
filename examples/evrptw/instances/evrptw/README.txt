The instances are formatted as follows:

###For each location the instance provides:
-StringId as a unique identifier
-Type indicates the function of the location, i.e,
---d: depot
---f: recharging station
---c: customer location
-x, y are coordinates (distances are assumed to be euclidean) 
-demand specifies the quantity of freight capacity required
-ReadyTime and DueDate are the beginning and the end of the time window (waiting is allowed)
-ServiceTime denotes the entire time spend at customer for loading operations

###For the electric vehicles (all identical):
-"Q Vehicle fuel tank capacity": units of energy available
-"C Vehicle load capacity":      units available for cargo
-"r fuel consumption rate":      reduction of battery capacity when traveling one unit of distance
-"g inverse refueling rate":     units of time required to recharge one unit of energy
-"v average Velocity":           assumed to be constant on all arcs, required to calculate the travel time from distance