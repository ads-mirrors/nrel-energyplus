Heating and Cooling Fuel Efficiency for Ideal Loads Air System
==============================================================

**Bereket Nigusse**

**Florida Solar Energy Center**

 - First draft: March 5, 2025
 - Modified Date: April 30, 2025

## Justification for New Feature ##

Ideal loads air systems are increasingly used for early-stage design analysis and in a building energy code compliance method. 
However, currently this object has no built-in method for converting the loads to energy consumption. 
As a result, users have to post-process simulation results to convert the loads to energy. 

**- This enhancement is intended to support loads conversion into energy consumption for ideal loads air system object **
**- Also reduces the burden of simulation results post processing and increase the usability of the object **

## E-mail and  Conference Call Conclusions ##

NA

## Overview ##

(1) ZoneHVAC:IdealLoadsAirSystem reports zone loads but not the equivalent energy consumption.
(2) This object needs a method for converting loads into energy consumptions. 
(3) It doesn't have input fields for specifying fuel efficiency values.


## Approach ##

The following two alternative approaches have been proposed for consideration to achieve the intended goal:

These two options will be discussed and the full NFP will be drafted after the initial discussion.

** Option I: Adding EMS Actuators **
Adds EMS actuator for heating and cooling fuel efficiencies to the ZoneHVAC:IdealLoadsAirSystem object. 

 * - Requires adding two actuators: *
 * - (1) EMS actuator “Heating Fuel Efficiency” for heating *
 * - (2) EMS actuator “Cooling Fuel Efficiency” for cooling *

These actuators pass the efficiency values and give access to the conversion method to calculate the energy consumption during runtime.

** Option II: Adding New INput Fields **
Add optional heating and cooling fuel efficiency input fields to the ZoneHVAC:IdealLoadsAirSystem object. 
The cooling and heating efficiency values will be used to calculate the energy consumed from the zone's ideal loads.

 * - Requires adding two new input fields: *
 * - (1) New Input Field: “Heating Fuel Efficiency Schedule Name” for heating *
 * - (2) New Input Field: “Cooling Fuel Efficiency Schedule Name” for cooling *

For both options, the heating and cooling energy rate and energy consumption will be calculated from the corresponding loads and fuel efficiencies as follows:

	 {E_dot_{heat}} = {Q_dot_{load,heat}} / {Fuel_Eff_{heat}}
	 {E_dot_{cool}} = {Q_dot_{load,cool}} / {Fuel_Eff_{cool}}
	 
	 where,
	 {Q_{dot,cool}}     = Zone Ideal Loads Zone Total Cooling Rate, [W]
	 {Q_{dot,heat}}     = Zone Ideal Loads Zone Total Heating Rate, [W]
	 {E_dot_{cool}}     = Zone Ideal Loads Zone Cooling Energy Rate, [W]
	 {E_dot_{heat}}     = Zone Ideal Loads Zone Heating Energy Rate, [W]
	 {E_{cool}}         = Zone Ideal Loads Zone Cooling Energy, [J]
	 {E_{heat}}         = Zone Ideal Loads Zone Heating Energy, [J]	 
	 {Fuel_Eff_{cool}}  = Cooling Fuel Efficiency, [-]
	 {Fuel_Eff_{heat}}  = Heating Fuel Efficiency, [-]
	 
### Questions for Discussion:

* (1) Which alternative approach is preferred? *

*     - Option II requires transition while option I does not *
*     - Option II does not require transition if the new fields are appended as an optional fields * 

* (2) The efficiencies apply to the total (sensible and latent components) heating and cooling loads *

* (3) Existing reporting variables will not change *

* (4) Adds the following new report variables: *

      * - Zone Ideal Loads Zone Cooling Energy Rate [W] *
      * - Zone Ideal Loads Zone Cooling Energy [J] *
      * - Zone Ideal Loads Zone Heating Energy Rate [W] *
      * - Zone Ideal Loads Zone Heating Energy [J] *

* (5) Consider adding the following new report variables: *

      * - Zone Ideal Loads Supply Air Heating Energy Rate [W] *
      * - Zone Ideal Loads Supply Air Heating Energy [J] *
      * - Zone Ideal Loads Supply Air Cooling Energy Rate [W] *
      * - Zone Ideal Loads Supply Air Cooling Energy [J] *

      * - these four new variables will also be derived from the corresponding the following report variables by applying the fuel efficiencies *

      * - Zone Ideal Loads Supply Air Total Heating Rate [W] *
      * - Zone Ideal Loads Supply Air Total Cooling Rate [W] *

* (6) What should be the maximum limit for the fuel efficiency value, or no maximum limit?
	  
## Testing/Validation/Data Source(s): ##

Demonstrate that the fuel efficiency input fields set to 1.0 duplicates the current results. Unit tests will be added to demonstrate the enhancement.

## Input Output Reference Documentation ##

Update the documentations for the changed sections.

```
ZoneHVAC:IdealLoadsAirSystem,
       \memo Ideal system used to calculate loads without modeling a full HVAC system. All that is
       \memo required for the ideal system are zone controls, zone equipment configurations, and
       \memo the ideal loads system component. This component can be thought of as an ideal unit
       \memo that mixes zone air with the specified amount of outdoor air and then adds or removes
       \memo heat and moisture at 100% efficiency in order to meet the specified controls. Energy
       \memo use is reported as DistrictHeatingWater and DistrictCooling.
       \min-fields 27
  A1 , \field Name
       \required-field
       \reference ZoneEquipmentNames
  A2 , \field Availability Schedule Name
       \note Availability schedule name for this system. Schedule value > 0 means the system is available.
       \note If this field is blank, the system is always available.
       \type object-list
       \object-list ScheduleNames
  A3 , \field Zone Supply Air Node Name
       \note Must match a zone air inlet node name.
       \required-field
       \type node
  A4 , \field Zone Exhaust Air Node Name
       \note Should match a zone air exhaust node name.
       \note This field is optional, but is required if this
       \note this object is used with other forced air equipment.
       \type node
  A5 , \field System Inlet Air Node Name
       \note This field is only required when the Ideal Loads Air System is connected to an
       \note AirloopHVAC:ZoneReturnPlenum, otherwise leave this field blank. When connected to a plenum
       \note the return plenum Outlet Node Name (or Induced Air Outlet Node Name when connecting multiple
       \note ideal loads air systems) is entered here. The two ideal loads air system node name fields described above,
       \note the Zone Supply Air Node Name and the Zone Exhaust Air Node Name must also be entered.
       \note The Zone Supply Air Node Name must match a zone inlet air node name for the zone where this
       \note Ideal Loads Air System is connected. The Zone Exhaust Air Node Name must match an inlet air
       \note node name of an AirloopHVAC:ReturnAirPlenum object.
       \type node
  N1 , \field Maximum Heating Supply Air Temperature
       \units C
       \minimum> 0
       \maximum< 100
       \default 50
  N2 , \field Minimum Cooling Supply Air Temperature
       \units C
       \minimum> -100
       \maximum< 50
       \default 13
  N3 , \field Maximum Heating Supply Air Humidity Ratio
       \units kgWater/kgDryAir
       \minimum> 0
       \default 0.0156
  N4 , \field Minimum Cooling Supply Air Humidity Ratio
       \units kgWater/kgDryAir
       \minimum> 0
       \default 0.0077

        ....

  N10, \field Sensible Heat Recovery Effectiveness
       \units dimensionless
       \minimum 0.0
       \maximum 1.0
       \default 0.70
  N11, \field Latent Heat Recovery Effectiveness
       \note Applicable only if Heat Recovery Type is Enthalpy.
       \units dimensionless
       \minimum 0.0
       \maximum 1.0
       \default 0.65
  A17, \field Design Specification ZoneHVAC Sizing Object Name
       \note Enter the name of a DesignSpecificationZoneHVACSizing object.
       \type object-list
       \object-list DesignSpecificationZoneHVACSizingName
```

  **  The following two optional NEW input fields will be added  **

```
  A18, \field Heating Fuel Efficiency Schedule Name
       \type real
       \units dimensionless
       \minimum> 0.0
       \maximum 1.0
       \default 1.0
       \note Reference heating fuel efficiency for converting heating 
       \note ideal air loads into fuel energy consumption.
  A19; \field Cooling Fuel Efficiency Schedule Name
       \type real
       \units dimensionless
       \minimum> 0.0
       \default 1.0
       \note Reference cooling fuel efficiency for converting cooling 
       \note ideal air loads into fuel energy consumption.
```   

## Engineering Reference ##

As needed.

## Example File and Transition Changes ##

An example file will be modified to demonstrate the use of availability schedule. Simulation results will be examined and sample results will be provided.

Transition is required if option II is selected.

## Proposed Report Variables: ##

As proposed.


## References ##

N/A
