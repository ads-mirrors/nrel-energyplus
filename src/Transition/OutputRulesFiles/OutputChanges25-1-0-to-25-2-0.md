Output Changes
==============

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Description

This will eventually become a more structured file, but currently it isn't clear what format is best. As an intermediate solution, and to allow the form to be formed organically, this plain text file is being used. Entries should be clearly delimited. It isn't expected that there will be but maybe a couple each release at most. Entries should also include some reference back to the repo. At least a PR number or whatever.

### Table Output, Equipment Summary Report, Air Heat Recovery subtable

* Delete "Name" column.

* Change "Input Object Type" heading to "Type".

* In the "Plate/Rotary" column, "FlatPlate" is now "Plate"

* Reorder, rename, and change units for last two columns:

    "Exhaust Airflow [kg/s]" --> "Exhaust Air Flow Rate [m3/s]
    
    "Outdoor Airflow [kg/s]" --> "Supply Air Flow Rate [m3/s]"

See Pull Request [#10995](https://github.com/NREL/EnergyPlus/pull/10995).

### Component Sizing (eio and tables) for PlantLoop and CondenserLoop

* Change "Sizing option (Coincident/NonCoincident), 1.00000" to "Sizing Option, NonCoincident"

* Always report sizing values whether autosized or hard-sized.

See Pull Request [#10998](https://github.com/NREL/EnergyPlus/pull/10998).

### Table Output, Equipment Summary, PlantLoop or CondenserLoop subtable

* Always report sizing values whether autosized or hard-sized.

* Add columns for "Design Supply Temperature", "Design ReturnTemperature", and "Design Capacity".
  
See Pull Request [#10998](https://github.com/NREL/EnergyPlus/pull/10998).

