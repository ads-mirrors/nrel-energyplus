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

* Added new output variables:

```
   (1) * Zone Ideal Loads Zone Heating Fuel Energy Rate [W] *
   (2) * Zone Ideal Loads Zone Cooling Fuel Energy Rate [W]            
   (3) * Zone Ideal Loads Zone Heating Fuel Energy [J]
   (4) * Zone Ideal Loads Zone Cooling Fuel Energy [J]
   (5) * Zone Ideal Loads Supply Air Total Heating Fuel Energy Rate [W] *
   (6) * Zone Ideal Loads Supply Air Total Cooling Fuel Energy Rate [W] *
   (7) * Zone Ideal Loads Supply Air Total Heating Fuel Energy [J] *
   (8) * Zone Ideal Loads Supply Air Total Cooling Fuel Energy [J] *
```

See pull request [#10971] https://github.com/NREL/EnergyPlus/pull/10971