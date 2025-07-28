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

#pragma once

#include <EnergyPlus/GroundHeatExchangers/Base.hh>
#include <EnergyPlus/GroundHeatExchangers/Properties.hh>
#include <EnergyPlus/GroundHeatExchangers/ResponseFactors.hh>
#include <EnergyPlus/GroundTemperatureModeling/BaseGroundTemperatureModel.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantComponent.hh>

namespace EnergyPlus {

struct EnergyPlusData;

namespace GroundHeatExchangers {

    struct GLHEBase : PlantComponent // LCOV_EXCL_LINE
    {
        // Destructor
        ~GLHEBase() override = default;

        // Members
        bool available;   // need an array of logicals--load identifiers of available equipment
        bool on;          // simulate the machine at it's operating part load ratio
        std::string name; // user identifier
        PlantLocation plantLoc;
        int inletNodeNum;  // Node number on the inlet side of the plant
        int outletNodeNum; // Node number on the outlet side of the plant
        ThermophysicalProps soil;
        PipeProps pipe;
        ThermophysicalProps grout;
        std::shared_ptr<GLHEResponseFactors> myRespFactors;
        Real64 designFlow;            // Design volumetric flow rate [m3/s]
        Real64 designMassFlow;        // Design mass flow rate [kg/s]
        Real64 tempGround;            // The far field temperature of the ground [degC]
        Array1D<Real64> QnMonthlyAgg; // Monthly aggregated normalized heat extraction/rejection rate [W/m]
        Array1D<Real64> QnHr;         // Hourly aggregated normalized heat extraction/rejection rate [W/m]
        Array1D<Real64> QnSubHr; // Contains the sub-hourly heat extraction/rejection rate normalized by the total active length of boreholes [W/m]
        int prevHour;
        int AGG;               // Minimum Hourly History required
        int SubAGG;            // Minimum sub-hourly History
        Array1D_int LastHourN; // Stores the Previous hour's N for past hours until the minimum sub-hourly history
        Real64 bhTemp;         // [degC]
        Real64 massFlowRate;   // [kg/s]
        Real64 outletTemp;     // [degC]
        Real64 inletTemp;      // [degC]
        Real64 aveFluidTemp;   // [degC]
        Real64 QGLHE;          // [W] heat transfer rate
        bool myEnvrnFlag;
        bool gFunctionsExist;
        Real64 lastQnSubHr;
        Real64 HXResistance;    // The thermal resistance of the GHX, (K per W/m)
        Real64 totalTubeLength; // The total length of pipe. NumBoreholes * BoreholeDepth OR Pi * Dcoil * NumCoils
        Real64 timeSS;          // Steady state time
        Real64 timeSSFactor;    // Steady state time factor for calculation
        GroundTemp::BaseGroundTempsModel *groundTempModel = nullptr; // non-owning pointer

        // some statics pulled out into member variables
        bool firstTime;
        int numErrorCalls;
        Real64 ToutNew;
        int PrevN;             // The saved value of N at previous time step
        bool updateCurSimTime; // Used to reset the CurSimTime to reset after WarmupFlag
        bool triggerDesignDayReset;
        bool needToSetupOutputVars;

        GLHEBase()
            : available(false), on(false), plantLoc{}, inletNodeNum(0), outletNodeNum(0), designFlow(0.0), designMassFlow(0.0), tempGround(0.0),
              prevHour(1), AGG(0), SubAGG(0), bhTemp(0.0), massFlowRate(0.0), outletTemp(0.0), inletTemp(0.0), aveFluidTemp(0.0), QGLHE(0.0),
              myEnvrnFlag(true), gFunctionsExist(false), lastQnSubHr(0.0), HXResistance(0.0), totalTubeLength(0.0), timeSS(0.0), timeSSFactor(0.0),
              firstTime(true), numErrorCalls(0), ToutNew(19.375), PrevN(1), updateCurSimTime(true), triggerDesignDayReset(false),
              needToSetupOutputVars(true)
        {
        }

        virtual void calcGFunctions(EnergyPlusData &state) = 0;

        void calcAggregateLoad(EnergyPlusData &state);

        void updateGHX(EnergyPlusData &state);

        void calcGroundHeatExchanger(EnergyPlusData &state);

        static bool isEven(int const val)
        {
            return val % 2 == 0;
        };

        [[nodiscard]] Real64 interpGFunc(Real64) const;

        void makeThisGLHECacheAndCompareWithFileCache(EnergyPlusData &state);

        virtual void makeThisGLHECacheStruct() = 0;

        virtual void readCacheFileAndCompareWithThisGLHECache(EnergyPlusData &state) = 0;

        void onInitLoopEquip([[maybe_unused]] EnergyPlusData &state, const PlantLocation &calledFromLocation) override;

        void simulate([[maybe_unused]] EnergyPlusData &state,
                      const PlantLocation &calledFromLocation,
                      bool FirstHVACIteration,
                      Real64 &CurLoad,
                      bool RunFlag) override;

        static GLHEBase *factory(EnergyPlusData &state, DataPlant::PlantEquipmentType objectType, std::string const &objectName);

        virtual Real64 getGFunc(Real64) = 0;

        virtual void initGLHESimVars(EnergyPlusData &state) = 0;

        virtual Real64 calcHXResistance(EnergyPlusData &state) = 0;

        virtual void getAnnualTimeConstant() = 0;

        virtual void initEnvironment(EnergyPlusData &state, Real64 CurTime) = 0;

        void setupOutput(EnergyPlusData &state);
    };

    void GetGroundHeatExchangerInput(EnergyPlusData &state);

    std::vector<Real64> TDMA(std::vector<Real64> const &a, std::vector<Real64> const &b, std::vector<Real64> &c, std::vector<Real64> &d);

} // namespace GroundHeatExchangers

} // namespace EnergyPlus
