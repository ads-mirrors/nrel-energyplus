// EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

#ifndef AIRFLOWNETWORK_LINKAGES_HPP
#define AIRFLOWNETWORK_LINKAGES_HPP

#include "AirflowNetwork/Properties.hpp"
#include "AirflowNetwork/Types.hpp"
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/ScheduleManager.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;
struct AirflowNetworkData;

namespace AirflowNetwork {

    // Forward declaration
    struct AirflowElement;

    struct MultizoneSurfaceProp // Surface information
    {
        // Members
        std::string SurfName;         // Name of Associated EnergyPlus surface
        std::string OpeningName;      // Name of opening component, either simple or detailed large opening
        std::string ExternalNodeName; // Name of external node, but not used at WPC="INPUT"
        Real64 Factor;                // Crack Actual Value or Window Open Factor for Ventilation
        int SurfNum;                  // Surface number
        std::array<int, 2> NodeNums;  // Positive: Zone numbers; 0: External
        Real64 OpenFactor;            // Surface factor
        Real64 OpenFactorLast;        // Surface factor at previous time step
        bool EMSOpenFactorActuated;   // True if EMS actuation is on
        Real64 EMSOpenFactor;         // Surface factor value from EMS for override
        Real64 Height;                // Surface Height
        Real64 Width;                 // Surface width
        Real64 CHeight;               // Surface central height in z direction
        std::string VentControl;      // Ventilation Control Mode: TEMPERATURE, ENTHALPIC, CONSTANT, ZONELEVEL or NOVENT
        Real64 ModulateFactor;        // Limit Value on Multiplier for Modulating Venting Open Factor
        Real64 LowValueTemp;          // Lower Value on Inside/Outside Temperature Difference for Modulating the Venting Open Factor with temp control
        Real64 UpValueTemp; // Upper Value on Inside/Outside Temperature Difference for Modulating the Venting Open Factor with temp control
        Real64 LowValueEnth; // Lower Value on Inside/Outside Temperature Difference for Modulating the Venting Open Factor with Enthalpic control
        Real64 UpValueEnth; // Upper Value on Inside/Outside Temperature Difference for Modulating the Venting Open Factor with Enthalpic control
        std::string VentTempControlSchName;              // Name of ventilation temperature control schedule
        Sched::Schedule *ventTempControlSched = nullptr; // Ventilation temperature control schedule
        VentControlType VentSurfCtrNum;                  // Ventilation control mode number: 1 "Temperature", 2 "ENTHALPIC", 3 "CONSTANT", 4 "NOVENT"
        std::string VentAvailSchName;                    // Ventilation availability schedule
        Sched::Schedule *ventAvailSched = nullptr;       // Ventilation availability schedule
        int ZonePtr;                                     // Pointer to inside face zone
        bool IndVentControl;                             // Individual surface venting control
        int ExtLargeOpeningErrCount;                     // Exterior large opening error count during HVAC system operation
        int ExtLargeOpeningErrIndex;                     // Exterior large opening error index during HVAC system operation
        int OpenFactorErrCount;                          // Large opening error count at Open factor > 1.0
        int OpenFactorErrIndex;                          // Large opening error error index at Open factor > 1.0
        Real64 Multiplier;                               // Window multiplier
        bool HybridVentClose;                            // Hybrid ventilation window close control logical
        bool HybridCtrlGlobal;                           // Hybrid ventilation global control logical
        bool HybridCtrlMaster;                           // Hybrid ventilation global control master
        Real64 WindModifier;                             // Wind modifier from hybrid ventilation control
        std::string OccupantVentilationControlName;      // Occupant ventilation control name
        int OccupantVentilationControlNum;               // Occupant ventilation control number
        int OpeningStatus;                               // Open status at current time step
        int PrevOpeningstatus;                           // Open status at previous time step
        Real64 CloseElapsedTime;                         // Elapsed time during closing (min)
        Real64 OpenElapsedTime;                          // Elapsed time during closing (min)
        int ClosingProbStatus;                           // Closing probability status
        int OpeningProbStatus;                           // Opening probability status
        bool RAFNflag;                                   // True if this surface is used in AirflowNetwork:IntraZone:Linkage
        bool NonRectangular;                             // True if this surface is not rectangular
        EquivRec EquivRecMethod;        // Equivalent Rectangle Method input: 1 Height; 2 Base surface aspect ratio; 3 User input aspect ratio
        Real64 EquivRecUserAspectRatio; // user input value when EquivRecMethod = 3

        // Default Constructor
        MultizoneSurfaceProp()
            : Factor(0.0), SurfNum(0), NodeNums{{0, 0}}, OpenFactor(0.0), OpenFactorLast(0.0), EMSOpenFactorActuated(false), EMSOpenFactor(0.0),
              Height(0.0), Width(0.0), CHeight(0.0), VentControl("ZONELEVEL"), ModulateFactor(0.0), LowValueTemp(0.0), UpValueTemp(100.0),
              LowValueEnth(0.0), UpValueEnth(300000.0), VentSurfCtrNum(VentControlType::None), ZonePtr(0), IndVentControl(false),
              ExtLargeOpeningErrCount(0), ExtLargeOpeningErrIndex(0), OpenFactorErrCount(0), OpenFactorErrIndex(0), Multiplier(1.0),
              HybridVentClose(false), HybridCtrlGlobal(false), HybridCtrlMaster(false), WindModifier(1.0), OccupantVentilationControlNum(0),
              OpeningStatus(OpenStatus::FreeOperation), PrevOpeningstatus(OpenStatus::FreeOperation), CloseElapsedTime(0.0), OpenElapsedTime(0.0),
              ClosingProbStatus(ProbabilityCheck::NoAction), OpeningProbStatus(ProbabilityCheck::NoAction), RAFNflag(false), NonRectangular(false),
              EquivRecMethod(EquivRec::Height), EquivRecUserAspectRatio(1.0)
        {
        }
    };

    struct AirflowNetworkLinkage // AirflowNetwork linkage data base class
    {
        // Members
        std::string Name;                     // Provide a unique linkage name
        std::array<std::string, 2> NodeNames; // Names of nodes (limited to 2)
        std::array<Real64, 2> NodeHeights;    // Node heights
        std::string CompName;                 // Name of element
        int CompNum;                          // Element Number
        std::array<int, 2> NodeNums;          // Node numbers
        int LinkNum;                          // Linkage number
        AirflowElement *element;              // Pointer to airflow element
        Real64 control;                       // Control value

        // Default Constructor
        AirflowNetworkLinkage() : NodeHeights{{0.0, 0.0}}, CompNum(0), NodeNums{{0, 0}}, LinkNum(0), element(nullptr), control(1.0)
        {
        }

        virtual ~AirflowNetworkLinkage()
        {
        }
    };

    struct IntraZoneLinkageProp : public AirflowNetworkLinkage // Intra zone linkage data
    {
        // Members
        std::string SurfaceName; // Connection Surface Name

        // Default Constructor
        IntraZoneLinkageProp() : AirflowNetworkLinkage()
        {
        }
    };

    struct DisSysLinkageProp : public AirflowNetworkLinkage // Distribution system linkage data
    {
        // Members
        std::string ZoneName; // Name of zone
        int ZoneNum;          // Zone Number

        // Default Constructor
        DisSysLinkageProp() : AirflowNetworkLinkage(), ZoneNum(0)
        {
        }
    };

    struct AirflowNetworkLinkageProp : public AirflowNetworkLinkage // AirflowNetwork linkage data
    {
        // Members
        std::string ZoneName;               // Name of zone
        int ZoneNum;                        // Zone Number
        int DetOpenNum;                     // Large Opening number
        EPlusComponentType ConnectionFlag; // Return and supply connection flag
        bool VAVTermDamper;                 // True if this component is a damper for a VAV terminal
        int LinkageViewFactorObjectNum;
        int AirLoopNum; // Airloop number
        DuctLineType ductLineType;

        // Default Constructor
        AirflowNetworkLinkageProp()
            : AirflowNetworkLinkage(), ZoneNum(0), DetOpenNum(0), ConnectionFlag(EPlusComponentType::Invalid), VAVTermDamper(false),
              LinkageViewFactorObjectNum(0), AirLoopNum(0), ductLineType(DuctLineType::Invalid)
        {
        }
    };

    struct AirflowNetworkLinkSimuData
    {
        // Members
        Real64 FLOW;     // Mass flow rate [kg/s]
        Real64 FLOW2;    // Mass flow rate [kg/s] for two way flow
        Real64 DP;       // Pressure difference across a component
        Real64 VolFLOW;  // Mass flow rate [m3/s]
        Real64 VolFLOW2; // Mass flow rate [m3/s] for two way flow
        Real64 DP1;

        // Default Constructor
        AirflowNetworkLinkSimuData() : FLOW(0.0), FLOW2(0.0), DP(0.0), VolFLOW(0.0), VolFLOW2(0.0), DP1(0.0)
        {
        }
    };

    struct AirflowNetworkLinkReportData
    {
        // Members
        Real64 FLOW;        // Mass flow rate [kg/s]
        Real64 FLOW2;       // Mass flow rate [kg/s] for two way flow
        Real64 VolFLOW;     // Mass flow rate [m^3/s]
        Real64 VolFLOW2;    // Mass flow rate [m^3/s] for two way flow
        Real64 FLOWOFF;     // Mass flow rate during OFF cycle [kg/s]
        Real64 FLOW2OFF;    // Mass flow rate during OFF cycle [kg/s] for two way flow
        Real64 VolFLOWOFF;  // Mass flow rate during OFF cycle [m^3/s]
        Real64 VolFLOW2OFF; // Mass flow rate during OFF cycle [m^3/s] for two way flow
        Real64 DP;          // Average Pressure difference across a component
        Real64 DPON;        // Pressure difference across a component with fan on
        Real64 DPOFF;       // Pressure difference across a component with fan off

        // Default Constructor
        AirflowNetworkLinkReportData()
            : FLOW(0.0), FLOW2(0.0), VolFLOW(0.0), VolFLOW2(0.0), FLOWOFF(0.0), FLOW2OFF(0.0), VolFLOWOFF(0.0), VolFLOW2OFF(0.0), DP(0.0), DPON(0.0),
              DPOFF(0.0)
        {
        }
    };

    struct LinkageSurfaceProp
    {
        // Members
        std::string SurfaceName;
        int SurfaceNum;                 // Name of surface referenced by view factor
        Real64 ViewFactor;              // View factor
        Real64 SurfaceResistanceFactor; // Total radiation heat transfer resistance factor
        Real64 SurfaceRadLoad;          // Duct radiation load from surface [W]

        // Default constructor
        LinkageSurfaceProp() : SurfaceNum(0), ViewFactor(0.0), SurfaceResistanceFactor(0.0), SurfaceRadLoad(0.0)
        {
        }
    };

    struct AirflowNetworkLinkageViewFactorProp
    {
        // Members
        std::string LinkageName;
        Real64 DuctExposureFraction;
        Real64 DuctEmittance;
        Array1D<LinkageSurfaceProp> LinkageSurfaceData;
        int ObjectNum;
        Real64 QRad;
        Real64 QConv;

        AirflowNetworkLinkageViewFactorProp() : DuctExposureFraction(0.0), DuctEmittance(0.0), ObjectNum(0), QRad(0.0), QConv(0.0)
        {
        }
    };

} // namespace AirflowNetwork

} // namespace EnergyPlus

#endif
