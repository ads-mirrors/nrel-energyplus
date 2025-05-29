Output Changes
==============

This file documents the structural changes on the output of EnergyPlus that could affect interfaces, etc.

### Description

This will eventually become a more structured file, but currently it isn't clear what format is best. As an intermediate solution, and to allow the form to be formed organically, this plain text file is being used. Entries should be clearly delimited. It isn't expected that there will be but maybe a couple each release at most. Entries should also include some reference back to the repo. At least a PR number or whatever.

### System Summary table report, Demand Controlled Ventilation using Controller:MechanicalVentilation" Subtable
In the first column use ZoneName for zones with a simple DSOA reference (same as before), and use ZoneName:SpaceName for the spaces in a DSOA:SpaceList.

See Pull Request [#11051](https://github.com/NREL/EnergyPlus/pull/11051).

### Table Output, Equipment Summary Report, Air Heat Recovery subtable
* Delete "Name" column.

* Change "Input Object Type" heading to "Type".

* In the "Plate/Rotary" column, "FlatPlate" is now "Plate"

* Reorder, rename, and change units for last two columns:

    "Exhaust Airflow [kg/s]" --> "Exhaust Air Flow Rate [m3/s]
    
    "Outdoor Airflow [kg/s]" --> "Supply Air Flow Rate [m3/s]"

See Pull Request [#10995](https://github.com/NREL/EnergyPlus/pull/10995).

