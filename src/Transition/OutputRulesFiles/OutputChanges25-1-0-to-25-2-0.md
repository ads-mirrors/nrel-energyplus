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

### EIO and HTML Table Output: Initialization Summary

A number of changes related to finding duplicated HTML tables (based on FullName) have been made.

See Pull Request [#11106](https://github.com/NREL/EnergyPlus/pull/11106).

#### Warmup Convergence Information

When `Output:Diagnostics` has the `ReportDetailedWarmupConvergence` enabled, both the regular report and the detailed one were named `Warmup Convergence Information` resulting in duplicated HTML tables that would include lines from the other report.

The `ReportDetailedWarmupConvergence` report (the one that shows the status at each timestep/hour until convergence is reached) was renamed to `Warmup Convergence Information - Detailed`.

#### Material:Air

The `Output:Constructions` has two possible keys: `Materials` and `Constructions`. Both were adding a table named `Material:Air`. The Constructions one was renamed to `Material:Air CTF Summary` to match the surrounding tables.
