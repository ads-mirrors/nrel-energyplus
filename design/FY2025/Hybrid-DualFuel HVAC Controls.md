Controls for Dual Fuel (Hybrid) HVAC System
================

**Michael J. Witte, GARD Analytics, Inc.**

 - Original June 17, 2025
 - Revision Date
 

## Justification for New Feature ##

Fuel-switching can save money and reduce emissions. EnergyPlus has various types of equipment available with various fuel types, but lacks sufficient control options to fully optimize fuel switching.

## E-mail and  Conference Call Conclusions ##

insert text

## Overview ##

The original feature request description is:

  "Develop and incorporate modules in EnergyPlus to simulate Residential and Commercial dual-fuel control strategies that can be used by practitioners and EE programs as an industry-wide recognized methodology in accordance with CSA dual-fuel standards. Equipment pairings such as ASHP/ccASHP with furnace/combi/advanced combi systems; and dual-fuel RTU systems using sophisticated control strategies such as optimizing for efficiency, cost, emissions, and simultaneous operation."
  

## Approach ##

The proposed approach is to add options to the DemandManager system to lockout one fuel type or indicate a preference for one over another. The current demand manager objects act on specific load instances (e.g. Lights or Thermostats). From example file 5ZoneAirCooledDemandLimiting:
```
  DemandManager:ElectricEquipment,
    Eq Mgr Stage 1,          !- Name
    ,                        !- Availability Schedule Name
    FIXED,                   !- Limit Control
    60,                      !- Minimum Limit Duration {minutes}
    0.0,                     !- Maximum Limit Fraction
    ,                        !- Limit Step Change
    ALL,                     !- Selection Control
    ,                        !- Rotation Duration {minutes}
    Space1-1 AllZones with Electric Equipment;  !- Electric Equipment 1 Name
```

Rather than target specific equipment objects, the fuel-switching demand manager will set a global status on each fuel type: `Available`, `Preferred`, or `NotAvailable`.
Zone and airloop HVAC equipment will check the fuel-type status before activating the equipment. For plant equipment, the plant operation scheme will check the fuel-type status when dispatching supply equipment.

An example `DemandManager:FuelSwitching` object:

```
DemandManager:FuelSwitching,
  Dual Fuel Lowest Cost Lowest Carb, !- Name
  ,                   !- Availability Schedule Name
  Electricity,        !- Primary Fuel Type
  NaturalGas,         !- Secondary Fuel Type
  60,                 !- Minimum Duration
  None,               !- Capacity Control Type
  ,                   !- Heating Switchover Outdoor Dry Bulb Temperature
  Cost,               !- Criteria 1
  PreferLowest,       !- Criteria 1 Control Type
  CO2,                !- Criteria 2
  PreferLowest;       !- Criteria 2 Control Type
```

In the example above, the following rules would be applied with a minimum of 60 minutes between status changes:

* Capacity - Always use the primary fuel type (electricity) first, then supplement with the secondary fuel type.
* CO2 - Use the fuel type with the lowest current CO2 emissions rate first given projected fuel consumption for the current timestep. Allow the secondary fuel type to be used if needed to supplement.

So, what happens when different criteria favor different fuel types? The criteria will be applied in sequential fashion, similar to availability managers. When the demand manager is evaluated:

1. Set all fuel types to `Available`.

2. Evaluate the capacity control. 

    a. If the primary capacity is adequate to meet the current load or the capacity control type is `None`, go to the next criteria.

    b. If the primary capacity is inadequate and capacity control type is `SecondaryOnly` then set the primary fuel type to`NotAvailable`, exit and do not evaluate other criteria.

    c.  If the primary capacity is inadequate and the capacity control type is `SecondaryFirst` then set the secondary fuel type to `Preferred`.

3. Evaluate outdoor dry-bulb temperature. If below the specified switchover temperature, then set the secondary fuel type to `Available` and the primary fuel type to `NotAvailable`. Exit and do not evaluate other criteria.

4. Evaluate criteria 1.

    a. If the control type is `PreferLowest` and all fuel types are `Available` then set the lowest criteria fuel-type to `Preferred`. If a fuel type has already been set to `Preferred`, do not make any changes and move on to the next criteria.

    b. If the control type is `UseOnlyLowest` then set the higher fuel type to  `NotAvailable`, exit and do not evaluate other criteria.

5. Repeat for each remaining criteria.

*This seems overly complicated??*



## Testing/Validation/Data Sources ##

insert text

## Input Output Reference Documentation ##
*Initial rough draft*

## Input Description ##

### DemandManager:FuelSwitching ###
This object controls what HVAC system fuel type is to be used or preferred. Systems which have more than one fuel type available may select a given fuel type based on equipment capacity, fuel cost, or fuel emissions rates. The demand manager checks one or more criteria and then sets one of three status flags for the primary and secondary fuel types: `Available`, `Preferred`, or `NotAvailable`.

The control criteria include equipment capacity (ability to meet the current load), outdoor dry bulb temperature, and one or more optional criteria including cost, source energy, CO2 or other types of emissions. If more than one criteria is specified, the control logic is:

*see control logic description above*


*Name* - Name.

*Availability Schedule Name* - Schedule.

*Primary Fuel Type* - This is the primary fuel type which should be used first unless the demand manager changes the availability. The default is Electricity.

*Secondary Fuel Type* - This is the secondary fuel type which should be used only as needed unless the demand manager changes the availability. The default is NaturalGas.

*Minimum Duration* - The minimum duration in minutes before a change in control status is allowed.

*Control Action When Primary Capacity is Inadequate* - If the equipment powered by the primary fuel type has adequate capacity to meet the load, then no action is taken. If the primary fuel type is inadequate to meet the current load, then take the specified action. There are four choices:

  - *PrimaryThenSecondary* - use the primary fuel type first then supplement with secondary fuel type
  
  - *SecondaryOnly* - lock out the primary fuel type if capacity is inadequate to meet the current load
  
  - *SecondaryFirst* - use secondary fuel type first then supplement with primary fuel type
  
  - *None* - disable control based on capacity

The default is SecondaryOnly.

*Heating Switchover Outdoor Dry Bulb Temperature* - If the outdoor dry bulb temperature is below the switchover temperature, then set the primary fuel type to `NotAvailable` and set the secondary fuel type to `Available` for heating. The default is -99 [C] which indicates no switchover temperature.

*Criteria N* - Criteria used to compare the primary and secondary fuel types to determine which one to use or prefer. The choices are: Cost, SourceEnergy, CO2, CO, CH4, NOx, N2O, SO2, PM, PM10, PM2.5, NH3, NMVOC, Hg, Pb, Water, NuclearHighLevel, NuclearLowLevel. There is no default. If this field is blank, then there is no control action. This field and the next one may be repeated for as many control types as desired.

*Criteria N Control Type* - Control action based on comparing criteria N for the primary and secondary fuel types. The choices are:

  - *PreferLowest* - Sets the lowest fuel type to "Preferred" and allows the higher value fuel to supplement ("Available").
  
  - *UseOnlyLowest* - Locks out the higher value fuel ("NotAvailable").
  
  - *None* - Disables control on Criteria 1. This option is for convenience in paramatric analyses or optimization.
  
The default is None.

```
DemandManager:FuelSwitching,
       \memo Used for fuel switching between two fuels.
       \extensible:2
  A1 , \field Name
       \required-field
       \type alpha
       \reference DemandManagerNames
  A2 , \field Availability Schedule Name
       \note Availability schedule name for this system. Schedule value > 0 means the system is available.
       \note If this field is blank, the system is always available.
       \type object-list
       \object-list ScheduleNames
  A3 , \field Primary Fuel Type
       \type choice
       \key Electricity
       \key NaturalGas
       \key Propane
       \key FuelOilNo1
       \key FuelOilNo2
       \key Diesel
       \key Gasoline
       \key Coal
       \key OtherFuel1
       \key OtherFuel2
       \key DistrictHeatingWater
       \key DistrictHeatingSteam
       \key DistrictCooling
       \default Electricity
  A4 , \field Secondary Fuel Type
       \type choice
       \key Electricity
       \key NaturalGas
       \key Propane
       \key FuelOilNo1
       \key FuelOilNo2
       \key Diesel
       \key Gasoline
       \key Coal
       \key OtherFuel1
       \key OtherFuel2
       \key DistrictHeatingWater
       \key DistrictHeatingSteam
       \key DistrictCooling
       \default NaturalGas
  N1 , \field Minimum Duration
       \note The minimum duration before a change in control status is allowed.
       \type integer
       \minimum 0
       \units minutes
       \note If blank, duration defaults to the zone timestep
  A5 , \field Heating Switchover Outdoor Dry Bulb Temperature
       \note If the outdoor dry bulb temperature is below the switchover temperature, then use only the secondary fuel type for heating. 
       \type real
       \units C
       \default -99.0
  A5 , \field Control Action When Primary Capacity is Inadequate
       \note When the primary fuel type capacity is inadequate to meet the current load:
       \note PrimaryThenSecondary use the primary fuel type first then supplement with secondary fuel type
       \note SecondaryOnly lock out the primary fuel type if capacity is inadequate to meet the current load
       \note SecondaryFirst use secondary fuel type first then supplement with primary fuel type
       \note None disable control on capacity
       \type choice
       \key SecondaryOnly
       \key SecondaryFirst
       \key None
       \default SecondaryOnly
  A6 , \field Criteria 1
       \begin-extensible
       \choice
       \key Cost
       \key SourceEnergy
       \key CO2
       \key CO
       \key CH4
       \key NOx
       \key N2O
       \key SO2
       \key PM
       \key PM10
       \key PM2.5
       \key NH3
       \key NMVOC
       \key Hg
       \key Pb
       \key Water
       \key NuclearHighLevel
       \key NuclearLowLevel
  A7 , \field Criteria 1 Control Type
       \note PreferLowest allows the higher value fuel to supplement
       \note UseOnlyLowest locks out the higher value fuel
       \note None disables control on Criteria 1
       \type choice
       \key None
       \key PreferLowest
       \key UseOnlyLowest
       \default None
  A8 , \field Criteria 2
       \choice
       \key None
       \key Cost
       \key SourceEnergy
       \key CO2
       \key CO
       \key CH4
       \key NOx
       \key N2O
       \key SO2
       \key PM
       \key PM10
       \key PM2.5
       \key NH3
       \key NMVOC
       \key Hg
       \key Pb
       \key Water
       \key NuclearHighLevel
       \key NuclearLowLevel
  A9 ; \field Criteria 2 Control Type
       \note PreferLowest allows the higher value fuel to supplement
       \note UseOnlyLowest locks out the higher value fuel
       \note None disables control on Criteria 2
       \type choice
       \key None
       \key PreferLowest
       \key UseOnlyLowest
       \default None
```

## Outputs Description ##

###Demand Manager Primary Fuel Type Status###
###Demand Manager Secondary Fuel Type Status###
The current availability status for the primary or secondary fuel type.

  - 0 = NotAvailable
  - 1 = Available
  - 2 = Preferred

## Engineering Reference ##

insert text

## Example File and Transition Changes ##

No transition will be necessary.


## References ##

1. ""Hybrid Heat Pump System's Control Optimization for Annual Heating Operating Cost and Emission Minimization""

2. https://store.accuristech.com/ashrae/standards/ch-24-c024-hybrid-heat-pump-system-s-control-optimization-for-annual-heating-operating-cost-and-emission-minimization?product_id=2904875

3. https://www.aceee.org/sites/default/files/pdfs/Program%20%2B%20Presentations%20-%20Final.pdf

4. https://drive.google.com/file/d/1CKpXqUiiNkYdj3yfuuRTxtpY4a-8CMc8/view"
