import sys
import math

from pyenergyplus.plugin import EnergyPlusPlugin

class DirectSolarHeating(EnergyPlusPlugin):

    def __init__(self):
        # init parent class
        super().__init__()

        # members
        self.DSAH_MdotIn = 0.0
        self.DSAH_MdotOut = 0.0
        self.DSAH_Tinlet = 0.0
        self.DSAH_Winlet = 0.0
        self.DSAH_Toutlet = 0.0
        self.DSAH_Woutlet = 0.0

        # handles
        self.need_to_get_handles = True
        self.handles = {}

        # psych api instance
        self.psych = None

    def get_handles(self, state):
        self.handles["DSAH_Tinlet"] = self.api.exchange.get_internal_variable_handle(
            state,
            "Inlet Temperature for Primary Air Connection",
            "DirectSolarAirHeater"
        )

        self.handles["DSAH_Winlet"] = self.api.exchange.get_internal_variable_handle(
            state,
            "Inlet Humidity Ratio for Primary Air Connection",
            "DirectSolarAirHeater"
        )
        
        self.handles["DSAH_MdotOut"] = self.api.exchange.get_actuator_handle(
            state,
            "Primary Air Connection",
            "Outlet Mass Flow Rate",
            "DirectSolarAirHeater"
        )
        self.handles["DSAH_MdotIn"] = self.api.exchange.get_actuator_handle(
            state,
            "Primary Air Connection",
            "Inlet Mass Flow Rate",
            "DirectSolarAirHeater"
        )
        self.handles["DSAH_Toutlet"] = self.api.exchange.get_actuator_handle(
            state,
            "Primary Air Connection",
            "Outlet Temperature",
            "DirectSolarAirHeater"
        )
        self.handles["DSAH_Woutlet"] = self.api.exchange.get_actuator_handle(
            state,
            "Primary Air Connection",
            "Outlet Humidity Ratio",
            "DirectSolarAirHeater"
        )
        self.need_to_get_handles = False

    def handles_gotten_properly(self, state):
        handles_ok = True

        for (k, v) in self.handles.items():
            if v == -1:
                handles_ok = False
                self.api.runtime.issue_severe(state, f"Handle not found for '{k}'")

        return handles_ok

    def initialize(self, state):
        self.DSAH_Tinlet = self.api.exchange.get_internal_variable_value(state, self.handles["DSAH_Tinlet"])
        self.DSAH_Winlet = self.api.exchange.get_internal_variable_value(state, self.handles["DSAH_Winlet"])

    def simulate(self, state):
        # First simulation, assumed a predefined Heating Capacity, the DSAH will heat the air
        # according to it
        HCapacity = 3000 #W
        QDesign = 1.0 #m^3/s
        Patm = 77700 #Pa
        VspIn = self.psych.specific_volume(state, self.DSAH_Tinlet, self.DSAH_Winlet, Patm)
        self.DSAH_MdotIn = QDesign / VspIn
        self.DSAH_MdotOut = self.DSAH_MdotIn
        hInlet = self.psych.enthalpy(state, self.DSAH_Tinlet, self.DSAH_Winlet)
        hOutlet = hInlet + HCapacity / self.DSAH_MdotOut
        self.DSAH_Toutlet = self.psych.dry_bulb(state, hOutlet, self.DSAH_Winlet)
        self.DSAH_Woutlet = self.DSAH_Winlet

    def report(self, state):
        self.api.exchange.set_actuator_value(state, self.handles["DSAH_MdotOut"], self.DSAH_MdotOut)
        self.api.exchange.set_actuator_value(state, self.handles["DSAH_MdotIn"], self.DSAH_MdotIn)
        self.api.exchange.set_actuator_value(state, self.handles["DSAH_Toutlet"], self.DSAH_Toutlet)
        self.api.exchange.set_actuator_value(state, self.handles["DSAH_Woutlet"], self.DSAH_Woutlet)

    def on_user_defined_component_model(self, state) -> int:
        if not self.psych:
            self.psych = self.api.functional.psychrometrics(state)
        if self.need_to_get_handles:
            self.get_handles(state)
            if not self.handles_gotten_properly(state):
                return 1
        self.initialize(state)
        self.simulate(state)
        self.report(state)
        return 0

class IndirectSolarHeating(EnergyPlusPlugin):

    def __init__(self):
        # init parent class
        super().__init__()

        # members
        self.ISAH_MdotIn = 0.0
        self.ISAH_MdotOut = 0.0
        self.ISAH_Tinlet = 0.0
        self.ISAH_Winlet = 0.0
        self.ISAH_Toutlet = 0.0
        self.ISAH_Woutlet = 0.0

        # handles
        self.need_to_get_handles = True
        self.handles = {}

        # psych api instance
        self.psych = None

    def get_handles(self, state):
        self.handles["ISAH_Tinlet"] = self.api.exchange.get_internal_variable_handle(
            state,
            "Inlet Temperature for Primary Air Connection",
            "IndirectSolarAirHeater"
        )

        self.handles["ISAH_Winlet"] = self.api.exchange.get_internal_variable_handle(
            state,
            "Inlet Humidity Ratio for Primary Air Connection",
            "IndirectSolarAirHeater"
        )
        
        self.handles["ISAH_MdotOut"] = self.api.exchange.get_actuator_handle(
            state,
            "Primary Air Connection",
            "Outlet Mass Flow Rate",
            "IndirectSolarAirHeater"
        )
        self.handles["ISAH_MdotIn"] = self.api.exchange.get_actuator_handle(
            state,
            "Primary Air Connection",
            "Inlet Mass Flow Rate",
            "IndirectSolarAirHeater"
        )
        self.handles["ISAH_Toutlet"] = self.api.exchange.get_actuator_handle(
            state,
            "Primary Air Connection",
            "Outlet Temperature",
            "IndirectSolarAirHeater"
        )
        self.handles["ISAH_Woutlet"] = self.api.exchange.get_actuator_handle(
            state,
            "Primary Air Connection",
            "Outlet Humidity Ratio",
            "IndirectSolarAirHeater"
        )
        self.need_to_get_handles = False

    def handles_gotten_properly(self, state):
        handles_ok = True

        for (k, v) in self.handles.items():
            if v == -1:
                handles_ok = False
                self.api.runtime.issue_severe(state, f"Handle not found for '{k}'")

        return handles_ok

    def initialize(self, state):
        self.ISAH_Tinlet = self.api.exchange.get_internal_variable_value(state, self.handles["ISAH_Tinlet"])
        self.ISAH_Winlet = self.api.exchange.get_internal_variable_value(state, self.handles["ISAH_Winlet"])

    def simulate(self, state):
        # First simulation, assumed a predefined Evaporative Cooling Effectiveness of 0.6 
        QDesign = 2.0 #m^3/s
        Patm = 77700 #Pa
        VspIn = self.psych.specific_volume(state, self.ISAH_Tinlet, self.ISAH_Winlet, Patm)
        self.ISAH_MdotIn = QDesign / VspIn
        self.ISAH_MdotOut = self.ISAH_MdotIn
        Twb = self.psych.wet_bulb(state, self.ISAH_Tinlet, self.ISAH_Winlet, Patm)
        self.ISAH_Toutlet = 0.6 * (self.ISAH_Tinlet - Twb)
        self.ISAH_Woutlet = self.psych.humidity_ratio_d(state, self.ISAH_Tinlet, Twb, Patm)

    def report(self, state):
        self.api.exchange.set_actuator_value(state, self.handles["ISAH_MdotOut"], self.ISAH_MdotOut)
        self.api.exchange.set_actuator_value(state, self.handles["ISAH_MdotIn"], self.ISAH_MdotIn)
        self.api.exchange.set_actuator_value(state, self.handles["ISAH_Toutlet"], self.ISAH_Toutlet)
        self.api.exchange.set_actuator_value(state, self.handles["ISAH_Woutlet"], self.ISAH_Woutlet)

    def on_user_defined_component_model(self, state) -> int:
        if not self.psych:
            self.psych = self.api.functional.psychrometrics(state)
        if self.need_to_get_handles:
            self.get_handles(state)
            if not self.handles_gotten_properly(state):
                return 1
        self.initialize(state)
        self.simulate(state)
        self.report(state)
        return 0
    
class AirMixing(EnergyPlusPlugin):

    def __init__(self):
        # init parent class
        super().__init__()

        # members
        self.AMXR_MdotIn1 = 0.0
        self.AMXR_MdotIn2 = 0.0
        self.AMXR_MdotOut = 0.0
        self.AMXR_Tinlet1 = 0.0
        self.AMXR_Winlet1 = 0.0
        self.AMXR_Tinlet2 = 0.0
        self.AMXR_Winlet2 = 0.0
        self.AMXR_Toutlet = 0.0
        self.AMXR_Woutlet = 0.0

        # handles
        self.need_to_get_handles = True
        self.handles = {}

        # psych api instance
        self.psych = None

    def get_handles(self, state):
        self.handles["AMXR_MdotIn1"] = self.api.exchange.get_variable_handle(
            state,
            "System Node Mass Flow Rate",
            "DSAHOutletNode"
        )
#
        self.handles["AMXR_MdotIn2"] = self.api.exchange.get_variable_handle(
            state,
            "System Node Mass Flow Rate",
            "ISAHOutletNode"
        )

        self.handles["AMXR_Tinlet1"] = self.api.exchange.get_internal_variable_handle(
            state,
            "Inlet Temperature for Primary Air Connection",
            "AirMixer"
        )

        self.handles["AMXR_Winlet1"] = self.api.exchange.get_internal_variable_handle(
            state,
            "Inlet Humidity Ratio for Primary Air Connection",
            "AirMixer"
        )

        self.handles["AMXR_Tinlet2"] = self.api.exchange.get_internal_variable_handle(
            state,
            "Inlet Temperature for Secondary Air Connection",
            "AirMixer"
        )

        self.handles["AMXR_Winlet2"] = self.api.exchange.get_internal_variable_handle(
            state,
            "Inlet Humidity Ratio for Secondary Air Connection",
            "AirMixer"
        )
        
        self.handles["AMXR_MdotOut"] = self.api.exchange.get_actuator_handle(
            state,
            "Primary Air Connection",
            "Outlet Mass Flow Rate",
            "AirMixer"
        )

        self.handles["AMXR_Toutlet"] = self.api.exchange.get_actuator_handle(
            state,
            "Primary Air Connection",
            "Outlet Temperature",
            "AirMixer"
        )

        self.handles["AMXR_Woutlet"] = self.api.exchange.get_actuator_handle(
            state,
            "Primary Air Connection",
            "Outlet Humidity Ratio",
            "AirMixer"
        )

        self.need_to_get_handles = False

    def handles_gotten_properly(self, state):
        handles_ok = True

        for (k, v) in self.handles.items():
            if v == -1:
                handles_ok = False
                self.api.runtime.issue_severe(state, f"Handle not found for '{k}'")

        return handles_ok

    def initialize(self, state):
        self.AMXR_Tinlet1 = self.api.exchange.get_internal_variable_value(state, self.handles["AMXR_Tinlet1"])
        self.AMXR_Winlet1 = self.api.exchange.get_internal_variable_value(state, self.handles["AMXR_Winlet1"])
        self.AMXR_MdotIn1 = self.api.exchange.get_variable_value(state, self.handles["AMXR_MdotIn1"])
        self.AMXR_Tinlet2 = self.api.exchange.get_internal_variable_value(state, self.handles["AMXR_Tinlet2"])
        self.AMXR_Winlet2 = self.api.exchange.get_internal_variable_value(state, self.handles["AMXR_Winlet1"])
        self.AMXR_MdotIn2 = self.api.exchange.get_variable_value(state, self.handles["AMXR_MdotIn2"])

    def simulate(self, state):
        # Second simulation, the function will find output conditions of a mixed air
        self.AMXR_MdotOut = self.AMXR_MdotIn1 + self.AMXR_MdotIn2
        try:
            rmix = self.AMXR_MdotIn2 / self.AMXR_MdotIn1
            h1 = self.psych.enthalpy(self.AMXR_Tinlet1, self.AMXR_Winlet1)
            h2 = self.psych.enthalpy(self.AMXR_Tinlet2, self.AMXR_Winlet2)
            h3 = h1 + rmix * (h2 - h1)
            self.AMXR_Woutlet = self.AMXR_Winlet1 + rmix * (self.AMXR_Winlet2 - self.AMXR_Winlet1)
            self.AMXR_Toutlet = self.psych.dry_bulb(h3, self.AMXR_Woutlet)
        except ZeroDivisionError as e:
            if self.AMXR_MdotIn1 == 0.0 and self.AMXR_MdotIn2 == 0.0:
                self.AMXR_MdotOut = 0.0
                self.AMXR_Woutlet = (self.AMXR_Winlet1 + self.AMXR_Winlet2) / 2.
                self.AMXR_Toutlet = (self.AMXR_Tinlet1 + self.AMXR_Tinlet2) / 2.
            else:
                self.AMXR_MdotOut = self.AMXR_MdotIn2
                self.AMXR_Woutlet = self.AMXR_Winlet2
                self.AMXR_Toutlet = self.AMXR_Tinlet2

    def report(self, state):
        self.api.exchange.set_actuator_value(state, self.handles["AMXR_MdotOut"], self.AMXR_MdotOut)
        self.api.exchange.set_actuator_value(state, self.handles["AMXR_Toutlet"], self.AMXR_Toutlet)
        self.api.exchange.set_actuator_value(state, self.handles["AMXR_Woutlet"], self.AMXR_Woutlet)

    def on_begin_zone_timestep_before_init_heat_balance(self, state) -> int:
        if not self.psych:
            self.psych = self.api.functional.psychrometrics(state)
        if self.need_to_get_handles:
            self.get_handles(state)
            if not self.handles_gotten_properly(state):
                return 1
        self.initialize(state)
        self.simulate(state)
        self.report(state)
        return 0