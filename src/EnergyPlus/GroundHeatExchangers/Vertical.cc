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

#include <cpgfunction/boreholes.h>
#include <cpgfunction/gfunction.h>
#include <cpgfunction/segments.h>

#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/GroundHeatExchangers/BoreholeArray.hh>
#include <EnergyPlus/GroundHeatExchangers/State.hh>
#include <EnergyPlus/GroundHeatExchangers/Vertical.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus::GroundHeatExchangers {
GLHEVert::GLHEVert(EnergyPlusData &state, std::string const &objName, nlohmann::json const &j)
{
    // Check for duplicates
    for (auto &existingObj : state.dataGroundHeatExchanger->verticalGLHE) {
        if (objName == existingObj.name) {
            ShowFatalError(state, format("Invalid input for {} object: Duplicate name found: {}", this->moduleName, existingObj.name));
        }
    }

    bool errorsFound = false;

    this->name = objName;

    // get inlet node num
    std::string const inletNodeName = Util::makeUPPER(j["inlet_node_name"].get<std::string>());

    this->inletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                             inletNodeName,
                                                             errorsFound,
                                                             DataLoopNode::ConnectionObjectType::GroundHeatExchangerSystem,
                                                             objName,
                                                             DataLoopNode::NodeFluidType::Water,
                                                             DataLoopNode::ConnectionType::Inlet,
                                                             NodeInputManager::CompFluidStream::Primary,
                                                             DataLoopNode::ObjectIsNotParent);

    // get outlet node num
    std::string const outletNodeName = Util::makeUPPER(j["outlet_node_name"].get<std::string>());

    this->outletNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                              outletNodeName,
                                                              errorsFound,
                                                              DataLoopNode::ConnectionObjectType::GroundHeatExchangerSystem,
                                                              objName,
                                                              DataLoopNode::NodeFluidType::Water,
                                                              DataLoopNode::ConnectionType::Outlet,
                                                              NodeInputManager::CompFluidStream::Primary,
                                                              DataLoopNode::ObjectIsNotParent);
    this->available = true;
    this->on = true;

    BranchNodeConnections::TestCompSet(state, this->moduleName, objName, inletNodeName, outletNodeName, "Condenser Water Nodes");

    this->designFlow = j["design_flow_rate"].get<Real64>();
    PlantUtilities::RegisterPlantCompDesignFlow(state, this->inletNodeNum, this->designFlow);

    this->soil.k = j["ground_thermal_conductivity"].get<Real64>();
    this->soil.rhoCp = j["ground_thermal_heat_capacity"].get<Real64>();

    if (j.find("ghe_vertical_responsefactors_object_name") != j.end()) {
        // Response factors come from IDF object
        this->myRespFactors = GetResponseFactor(state, Util::makeUPPER(j["ghe_vertical_responsefactors_object_name"].get<std::string>()));
        this->gFunctionsExist = true;
    }

    // no g-functions in the input file, so they need to be calculated
    if (!this->gFunctionsExist) {

        // g-function calculation method
        if (j.find("g_function_calculation_method") != j.end()) {
            std::string gFunctionMethodStr = Util::makeUPPER(j["g_function_calculation_method"].get<std::string>());
            if (gFunctionMethodStr == "UHFCALC") {
                this->gFuncCalcMethod = GFuncCalcMethod::UniformHeatFlux;
            } else if (gFunctionMethodStr == "UBHWTCALC") {
                this->gFuncCalcMethod = GFuncCalcMethod::UniformBoreholeWallTemp;
            } else {
                errorsFound = true;
                ShowSevereError(state, fmt::format("g-Function Calculation Method: \"{}\" is invalid", gFunctionMethodStr));
            }
        }

        // get borehole data from array or individual borehole instance objects
        if (j.find("ghe_vertical_array_object_name") != j.end()) {
            // Response factors come from array object
            this->myRespFactors = BuildAndGetResponseFactorObjectFromArray(
                state, GLHEVertArray::GetVertArray(state, Util::makeUPPER(j["ghe_vertical_array_object_name"].get<std::string>())));
        } else {
            if (j.find("vertical_well_locations") == j.end()) {
                // No ResponseFactors, GHEArray, or SingleBH object are referenced
                ShowSevereError(state, "No GHE:ResponseFactors, GHE:Vertical:Array, or GHE:Vertical:Single objects found");
                ShowFatalError(state, format("Check references to these objects for GHE:System object: {}", this->name));
            }

            auto const &vars = j.at("vertical_well_locations");

            // Calculate response factors from individual boreholes
            std::vector<std::shared_ptr<GLHEVertSingle>> tempVectOfBHObjects;

            for (auto const &var : vars) {
                if (!var.at("ghe_vertical_single_object_name").empty()) {
                    std::shared_ptr<GLHEVertSingle> tempBHptr =
                        GLHEVertSingle::GetSingleBH(state, Util::makeUPPER(var.at("ghe_vertical_single_object_name").get<std::string>()));
                    tempVectOfBHObjects.push_back(tempBHptr);
                } else {
                    break;
                }
            }

            this->myRespFactors = BuildAndGetResponseFactorsObjectFromSingleBHs(state, tempVectOfBHObjects);

            if (!this->myRespFactors) {
                errorsFound = true;
                ShowSevereError(state, "GroundHeatExchanger:Vertical:Single objects not found.");
            }
        }
    }

    this->bhDiameter = this->myRespFactors->props->bhDiameter;
    this->bhRadius = this->bhDiameter / 2.0;
    this->bhLength = this->myRespFactors->props->bhLength;
    this->bhUTubeDist = this->myRespFactors->props->bhUTubeDist;

    // pull pipe and grout data up from response factor struct for simplicity
    this->pipe.outDia = this->myRespFactors->props->pipe.outDia;
    this->pipe.innerDia = this->myRespFactors->props->pipe.innerDia;
    this->pipe.outRadius = this->pipe.outDia / 2;
    this->pipe.innerRadius = this->pipe.innerDia / 2;
    this->pipe.thickness = this->myRespFactors->props->pipe.thickness;
    this->pipe.k = this->myRespFactors->props->pipe.k;
    this->pipe.rhoCp = this->myRespFactors->props->pipe.rhoCp;

    this->grout.k = this->myRespFactors->props->grout.k;
    this->grout.rhoCp = this->myRespFactors->props->grout.rhoCp;

    this->myRespFactors->gRefRatio = this->bhRadius / this->bhLength;

    // Number of simulation years from RunPeriod
    this->myRespFactors->maxSimYears = state.dataEnvrn->MaxNumberSimYears;

    // total tube length
    this->totalTubeLength = this->myRespFactors->numBoreholes * this->myRespFactors->props->bhLength;

    // ground thermal diffusivity
    this->soil.diffusivity = this->soil.k / this->soil.rhoCp;

    // multipole method constants
    this->theta_1 = this->bhUTubeDist / (2 * this->bhRadius);
    this->theta_2 = this->bhRadius / this->pipe.outRadius;
    this->theta_3 = 1 / (2 * this->theta_1 * this->theta_2);
    this->sigma = (this->grout.k - this->soil.k) / (this->grout.k + this->soil.k);

    this->SubAGG = 15;
    this->AGG = 192;

    // Allocation of all the dynamic arrays
    this->QnMonthlyAgg.dimension(static_cast<int>(this->myRespFactors->maxSimYears * 12), 0.0);
    this->QnHr.dimension(730 + this->AGG + this->SubAGG, 0.0);
    this->QnSubHr.dimension(static_cast<int>((this->SubAGG + 1) * maxTSinHr + 1), 0.0);
    this->LastHourN.dimension(this->SubAGG + 1, 0);

    this->prevTimeSteps.allocate(static_cast<int>((this->SubAGG + 1) * maxTSinHr + 1));
    this->prevTimeSteps = 0.0;

    GroundTemp::ModelType modelType = static_cast<GroundTemp::ModelType>(
        getEnumValue(GroundTemp::modelTypeNamesUC, Util::makeUPPER(j["undisturbed_ground_temperature_model_type"].get<std::string>())));
    assert(modelType != GroundTemp::ModelType::Invalid);

    // Initialize ground temperature model and get pointer reference
    this->groundTempModel =
        GroundTemp::GetGroundTempModelAndInit(state, modelType, Util::makeUPPER(j["undisturbed_ground_temperature_model_name"].get<std::string>()));

    // Check for Errors
    if (errorsFound) {
        ShowFatalError(state, format("Errors found in processing input for {}", this->moduleName));
    }
}

void GLHEVert::getAnnualTimeConstant()
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Matt Mitchell
    //       DATE WRITTEN:    February 2015

    // PURPOSE OF THIS SUBROUTINE:
    // calculate annual time constant for ground conduction

    constexpr Real64 hrInYear = 8760;

    this->timeSS = (pow_2(this->bhLength) / (9.0 * this->soil.diffusivity)) / Constant::rSecsInHour / hrInYear; // Excuse me?
    this->timeSSFactor = this->timeSS * 8760.0;
}

void GLHEVert::combineShortAndLongTimestepGFunctions() const
{
    std::vector<Real64> GFNC_combined;
    std::vector<Real64> LNTTS_combined;

    Real64 const t_s = pow_2(this->bhLength) / (9.0 * this->soil.diffusivity);

    // Nothing to do. Just put the short time step g-functions on the combined vector
    const unsigned int num_shortTimestepGFunctions = GFNC_shortTimestep.size();
    for (int index_shortTS = 0; index_shortTS < num_shortTimestepGFunctions; ++index_shortTS) {
        GFNC_combined.push_back(GFNC_shortTimestep[index_shortTS]);
        LNTTS_combined.push_back(LNTTS_shortTimestep[index_shortTS]);
    }

    // Add the rest of the long time-step g-functions to the combined curve
    for (int index_longTS = 0; index_longTS < this->myRespFactors->GFNC.size(); ++index_longTS) {
        GFNC_combined.push_back(this->myRespFactors->GFNC[index_longTS]);
        LNTTS_combined.push_back(this->myRespFactors->LNTTS[index_longTS]);
    }

    this->myRespFactors->time = LNTTS_combined;
    std::transform(this->myRespFactors->time.begin(), this->myRespFactors->time.end(), this->myRespFactors->time.begin(), [&t_s](auto const &c) {
        return exp(c) * t_s;
    });

    this->myRespFactors->LNTTS = LNTTS_combined;
    this->myRespFactors->GFNC = GFNC_combined;
}

void GLHEVert::makeThisGLHECacheStruct()
{
    // For convenience
    auto &d = myCacheData["Phys Data"];

    d["Flow Rate"] = this->designFlow;
    d["Soil k"] = this->soil.k;
    d["Soil rhoCp"] = this->soil.rhoCp;
    d["BH Top Depth"] = this->myRespFactors->props->bhTopDepth;
    d["BH Length"] = this->myRespFactors->props->bhLength;
    d["BH Diameter"] = this->myRespFactors->props->bhDiameter;
    d["Grout k"] = this->myRespFactors->props->grout.k;
    d["Grout rhoCp"] = this->myRespFactors->props->grout.rhoCp;
    d["Pipe k"] = this->myRespFactors->props->pipe.k;
    d["Pipe rhoCP"] = this->myRespFactors->props->pipe.rhoCp;
    d["Pipe Diameter"] = this->myRespFactors->props->pipe.outDia;
    d["Pipe Thickness"] = this->myRespFactors->props->pipe.thickness;
    d["U-tube Dist"] = this->myRespFactors->props->bhUTubeDist;
    d["Max Simulation Years"] = this->myRespFactors->maxSimYears;
    d["g-Function Calc Method"] = GroundHeatExchangers::GFuncCalcMethodsStrs[static_cast<int>(this->gFuncCalcMethod)];

    auto &d_bh_data = d["BH Data"];
    int i = 0;
    for (auto const &thisBH : this->myRespFactors->myBorholes) {
        ++i;
        auto &d_bh = d_bh_data[fmt::format("BH {}", i)];
        d_bh["X-Location"] = thisBH->xLoc;
        d_bh["Y-Location"] = thisBH->yLoc;
    }
}

void GLHEVert::readCacheFileAndCompareWithThisGLHECache(EnergyPlusData &state)
{

    if (!(state.files.outputControl.glhe && FileSystem::fileExists(state.dataStrGlobals->outputGLHEFilePath))) {
        // if the file doesn't exist, there are no data to read
        return;
    }
    // file exists -- read data and load if possible

    nlohmann::json const cached_json = FileSystem::readJSON(state.dataStrGlobals->outputGLHEFilePath);

    for (auto const &existing_data : cached_json) {
        if (myCacheData["Phys Data"] == existing_data["Phys Data"]) {
            myCacheData["Response Factors"] = existing_data["Response Factors"];
            gFunctionsExist = true;
            break;
        }
    }

    if (gFunctionsExist) {
        // Populate the time array
        this->myRespFactors->time = std::vector<Real64>(myCacheData["Response Factors"]["time"].get<std::vector<Real64>>());

        // Populate the lntts array
        this->myRespFactors->LNTTS = std::vector<Real64>(myCacheData["Response Factors"]["LNTTS"].get<std::vector<Real64>>());

        // Populate the g-function array
        this->myRespFactors->GFNC = std::vector<Real64>(myCacheData["Response Factors"]["GFNC"].get<std::vector<Real64>>());
    }
}

void GLHEVert::writeGLHECacheToFile(const EnergyPlusData &state) const
{
    // TODO: the key is GHLE here, should be GLHE, but could break cache reading, so I'm leaving it for now
    nlohmann::json cached_json;
    if (FileSystem::fileExists(state.dataStrGlobals->outputGLHEFilePath)) {
        // file exists -- add data
        // open file
        cached_json = FileSystem::readJSON(state.dataStrGlobals->outputGLHEFilePath);

        // add current data
        cached_json.emplace(fmt::format("GHLE {}", cached_json.size() + 1), myCacheData);
    } else {
        // file doesn't exist -- add data
        // add current data
        cached_json.emplace("GHLE 1", myCacheData);
    }
    FileSystem::writeFile<FileSystem::FileTypes::GLHE>(state.dataStrGlobals->outputGLHEFilePath, cached_json, 2);
}

std::vector<Real64> GLHEVert::distances(MyCartesian const &point_i, MyCartesian const &point_j)
{
    std::vector<Real64> sumVals;

    // Calculate the distance between points
    sumVals.push_back(pow_2(point_i.x - point_j.x));
    sumVals.push_back(pow_2(point_i.y - point_j.y));
    sumVals.push_back(pow_2(point_i.z - point_j.z));

    Real64 sumTot = 0.0;
    std::vector<Real64> retVals;
    std::for_each(sumVals.begin(), sumVals.end(), [&](Real64 n) { sumTot += n; });
    retVals.push_back(std::sqrt(sumTot));

    // Calculate distance to mirror point
    sumVals.pop_back();
    sumVals.push_back(pow_2(point_i.z - (-point_j.z)));

    sumTot = 0.0;
    std::for_each(sumVals.begin(), sumVals.end(), [&](Real64 n) { sumTot += n; });
    retVals.push_back(std::sqrt(sumTot));

    return retVals;
}

Real64 GLHEVert::calcResponse(std::vector<Real64> const &dists, Real64 const currTime) const
{
    const Real64 pointToPointResponse = erfc(dists[0] / (2 * sqrt(this->soil.diffusivity * currTime))) / dists[0];
    const Real64 pointToReflectedResponse = erfc(dists[1] / (2 * sqrt(this->soil.diffusivity * currTime))) / dists[1];
    return pointToPointResponse - pointToReflectedResponse;
}

Real64 GLHEVert::integral(MyCartesian const &point_i, std::shared_ptr<GLHEVertSingle> const &bh_j, Real64 const currTime) const
{

    // This code could be optimized in a number of ways.
    // The first, most simple way would be to precompute the distances from point i to point j, then store them for reuse.
    // The second, more intensive method would be to break the calcResponse calls out into four different parts.
    // The first point, last point, odd points, and even points. Then multiply the odd/even points by their respective coefficient for the
    // Simpson's method. After that, all points are summed together and divided by 3.

    Real64 sum_f = 0.0;
    int i = 0;
    int const lastIndex_j = static_cast<int>(bh_j->pointLocations_j.size() - 1u);
    for (auto const &point_j : bh_j->pointLocations_j) {
        std::vector<Real64> dists = distances(point_i, point_j);
        Real64 const f = calcResponse(dists, currTime);

        // Integrate using Simpson's
        if (i == 0 || i == lastIndex_j) {
            sum_f += f;
        } else if (isEven(i)) {
            sum_f += 2 * f;
        } else {
            sum_f += 4 * f;
        }

        ++i;
    }

    return (bh_j->dl_j / 3.0) * sum_f;
}

Real64 GLHEVert::doubleIntegral(std::shared_ptr<GLHEVertSingle> const &bh_i, std::shared_ptr<GLHEVertSingle> const &bh_j, Real64 const currTime) const
{

    // Similar optimizations as discussed above could happen here

    if (bh_i == bh_j) {

        Real64 sum_f = 0;
        int i = 0;
        int const lastIndex = static_cast<int>(bh_i->pointLocations_ii.size() - 1u);
        for (auto &thisPoint : bh_i->pointLocations_ii) {

            Real64 f = integral(thisPoint, bh_j, currTime);

            // Integrate using Simpson's
            if (i == 0 || i == lastIndex) {
                sum_f += f;
            } else if (isEven(i)) {
                sum_f += 2 * f;
            } else {
                sum_f += 4 * f;
            }

            ++i;
        }

        return (bh_i->dl_ii / 3.0) * sum_f;

    } else {

        Real64 sum_f = 0;
        int i = 0;
        int const lastIndex = static_cast<int>(bh_i->pointLocations_i.size() - 1u);
        for (auto const &thisPoint : bh_i->pointLocations_i) {

            Real64 f = integral(thisPoint, bh_j, currTime);

            // Integrate using Simpson's
            if (i == 0 || i == lastIndex) {
                sum_f += f;
            } else if (isEven(i)) {
                sum_f += 2 * f;
            } else {
                sum_f += 4 * f;
            }

            ++i;
        }

        return (bh_i->dl_i / 3.0) * sum_f;
    }
}

void GLHEVert::calcLongTimestepGFunctions(EnergyPlusData &state) const
{
    switch (this->gFuncCalcMethod) {
    case GFuncCalcMethod::UniformHeatFlux:
        this->calcUniformHeatFluxGFunctions(state);
        break;
    case GFuncCalcMethod::UniformBoreholeWallTemp:
        this->calcUniformBHWallTempGFunctions(state);
        break;
    default:
        assert(false);
    }
}

void GLHEVert::calcUniformBHWallTempGFunctions2(EnergyPlusData &state) const
{
    nlohmann::json gheDesignerInputs;
    gheDesignerInputs["version"] = 2;
    gheDesignerInputs["topology"] = {{{"type", "ground-heat-exchanger"}, {"name", "ghe1"}}};
    gheDesignerInputs["fluid"] = {// TODO: Get from Plant Loop
                                  {"fluid_name", "WATER"},
                                  {"concentration_percent", 0},
                                  {"temperature", 20}};
    nlohmann::json gheObjectInputs;
    // TODO: Detect if there are differences and error
    // TODO: GHEDesigner seems to just want one height, check this
    Real64 const height = this->myRespFactors->myBorholes[0]->props->bhLength;
    Real64 const plantMasMassFlow = this->designMassFlow; // this->plantLoc.loop->MaxMassFlowRate; // TODO: Check if it is auto-sized?
    nlohmann::json grout = {{"conductivity", this->grout.k}, {"rho_cp", this->grout.rhoCp}};
    nlohmann::json soil = {
        {"conductivity", this->soil.k},
        {"rho_cp", this->soil.rhoCp},
        {"undisturbed_temp", this->tempGround},
    };
    Real64 const shankSpacingForGHEDesigner = this->bhUTubeDist - this->pipe.outDia;
    nlohmann::json pipe = {{"inner_diameter", this->pipe.innerDia},
                           {"outer_diameter", this->pipe.outDia},
                           {"shank_spacing", shankSpacingForGHEDesigner},
                           {"roughness", 0.000001}, // TODO: This doesn't seem to be available on inputs
                           {"conductivity", this->pipe.k},
                           {"rho_cp", this->pipe.rhoCp},
                           {"arrangement", "SINGLEUTUBE"}};
    nlohmann::json borehole = {{"buried_depth", this->myRespFactors->props->bhTopDepth}, // TODO: Confirm this is ready for access
                               {"diameter", this->bhDiameter}};
    std::vector<Real64> x, y;
    for (const auto &bh : this->myRespFactors->myBorholes) {
        x.push_back(bh->xLoc);
        y.push_back(bh->yLoc);
        // boreholes.emplace_back(bh->props->bhLength, bh->props->bhTopDepth, bh->props->bhDiameter / 2.0, bh->xLoc, bh->yLoc);
    }
    nlohmann::json preDesigned = {{"arrangement", "MANUAL"}, {"H", height}, {"x", x}, {"y", y}};
    nlohmann::json ghe1 = {{"flow_rate", plantMasMassFlow},
                           {"flow_type", "SYSTEM"},
                           {"grout", grout},
                           {"soil", soil},
                           {"pipe", pipe},
                           {"borehole", borehole},
                           {"pre_designed", preDesigned}};
    gheDesignerInputs["ground-heat-exchanger"] = {{"ghe1", ghe1}};

    std::ofstream output_file("/tmp/ghedesigner_input.json");
    if (!output_file.is_open()) {
        std::cout << "\n Failed to open output file";
    } else {
        output_file << gheDesignerInputs;
        output_file.close();
    }

    DisplayString(state, "Starting up GHEDesigner");
    fs::path exePath;
    if (state.dataGlobal->installRootOverride) {
        exePath = state.dataStrGlobals->exeDirectoryPath / "energyplus";
    } else {
        exePath = FileSystem::getAbsolutePath(FileSystem::getProgramPath());
    }
    std::string const cmd = exePath.string() + " auxiliary ghedesigner /tmp/ghedesigner_input.json /tmp/outputs";
    int const status = FileSystem::systemCall(cmd);
    if (status != 0) {
        ShowFatalError(state, "GHEDesigner failed to calculate G-functions.");
    }
    DisplayString(state, "GHEDesigner complete");
    std::ifstream file("/tmp/outputs/SimulationSummary.json");
    nlohmann::json data = nlohmann::json::parse(file);
    std::vector<double> t = data["log_time"];
    std::vector<double> g = data["g_values"];
    std::vector<double> gbhw = data["g_bhw_values"];

    this->myRespFactors->time = t;
    this->myRespFactors->LNTTS = t; // TODO: Check this
    this->myRespFactors->GFNC = g;
    // this->myRespFactors->LNTTS = {
    //     -14.614018254182,
    //     -13.9208710736221,
    //     -13.5154059655139,
    //     -13.2277238930622,
    //     -13.0045803417479,
    //     -12.822258784954,
    //     -12.6681081051267,
    //     -12.5345767125022,
    //     -12.4167936768458,
    //     -12.311433161188,
    //     -12.2161229813837,
    //     -12.129111604394,
    //     -12.0490688967205,
    //     -11.9749609245668,
    //     -11.9059680530798,
    //     -11.8414295319423,
    //     -11.7808049101258,
    //     -11.7236464962859,
    //     -11.6695792750156,
    //     -11.6182859806281,
    //     -11.5694958164586,
    //     -11.5229758008237,
    //     -11.4785240382529,
    //     -11.4359644238341,
    //     -11.3951424293138,
    //     -11.3559217161606,
    //     -11.3181813881777,
    //     -11.2818137440068,
    //     -11.2467224241956,
    //     -11.2128208725199,
    //     -11.1800310496969,
    //     -11.1482823513823,
    //     -11.1175106927156,
    //     -11.0876577295659,
    //     -11.0586701926926,
    //     -11.0304993157259,
    //     -11.0031003415378,
    //     -10.9764320944557,
    //     -10.9504566080524,
    //     -10.9251388000681,
    //     -10.9004461874777,
    //     -10.8763486358987,
    //     -10.8528181384885,
    //     -10.8298286202638,
    //     -10.8073557644117,
    //     -10.785376857693,
    //     -10.763870652472,
    //     -10.7428172432742,
    //     -10.7221979560714,
    //     -10.7019952487539,
    //     -10.6821926214577,
    //     -10.6627745356006,
    //     -10.6437263406299,
    //     -10.6250342076178,
    //     -10.6066850689496,
    //     -10.5886665634469,
    //     -10.5709669863475,
    //     -10.5535752436356,
    //     -10.5364808102763,
    //     -10.5196736919599,
    //     -10.5031443900087,
    //     -10.486883869137,
    //     -10.4708835277905,
    //     -10.4551351708224,
    //     -10.4396309842864,
    //     -10.4243635121556,
    //     -10.4093256347911,
    //     -10.3945105490059,
    //     -10.3799117495848,
    //     -10.3655230121327,
    //     -10.3513383771407,
    //     -10.337352135166,
    //     -10.3235588130337,
    //     -10.3099531609779,
    //     -10.2965301406457,
    //     -10.2832849138957,
    //     -10.2702128323284,
    //     -10.2573094274925,
    //     -10.244570401715,
    //     -10.2319916195082,
    //     -10.2195690995096,
    //     -10.2072990069178,
    //     -10.1951776463854,
    //     -10.1832014553387,
    //     -10.1713669976917,
    //     -10.1596709579285,
    //     -10.1481101355275,
    //     -10.1366814397038,
    //     -10.1253818844499,
    //     -10.1142085838518,
    //     -10.1031587476652,
    //     -10.092229677133,
    //     -10.0814187610288,
    //     -10.070723471912,
    //     -10.0601413625815,
    //     -10.0496700627142,
    //     -10.0393072756787,
    //     -10.0290507755115,
    //     -10.0188984040475,
    //     -10.008848068194,
    //     -9.99889773734079,
    //     -9.98904544089777,
    //     -9.97928926595241,
    //     -9.96962735504067,
    //     -9.96005790402452,
    //     -9.95057916006998,
    //     -9.94118941972014,
    //     -9.93188702705783,
    //     -9.9226703719529,
    //     -9.91353788838963,
    //     -9.90448805286971,
    //     -9.89551938288695,
    //     -9.88663043546971,
    //     -9.87781980578755,
    //     -9.8690861258188,
    //     -9.86042806307568,
    //     -9.85184431938429,
    //     -9.84333362971638,
    //     -9.83489476107052,
    //     -9.8265265114,
    //     -9.8182277085853,
    //     -9.80999720944879,
    //     -9.80183389880963,
    //     -9.79373668857701,
    //     -9.78570451687975,
    //     -9.77773634723057,
    //     -9.76983116772346,
    //     -9.76198799026243,
    //     -9.75420584982037,
    //     -9.74648380372646,
    //     -9.7388209309809,
    //     -9.73121633159568,
    //     -9.72366912596029,
    //     -9.71617845423113,
    //     -9.70874347574362,
    //     -9.70136336844599,
    //     -9.69403732835392,
    //     -9.68676456902484,
    //     -9.67954432105135,
    //     -9.67237583157274,
    //     -9.66525836380388,
    //     -9.65819119658079,
    //     -9.65117362392214,
    //     -9.64420495460604,
    //     -9.63728451176147,
    //     -9.63041163247371,
    //     -9.62358566740331,
    //     -9.61680598041793,
    //     -9.61007194823659,
    //     -9.60338296008579,
    //     -9.59673841736712,
    //     -9.59013773333577,
    //     -9.58358033278961,
    //     -9.57706565176842,
    //     -9.5705931372628,
    //     -9.56416224693251,
    //     -9.55777244883374,
    //     -9.55142322115508,
    //     -9.54511405196181,
    //     -9.53884443894822,
    //     -9.53261388919758,
    //     -9.52642191894966,
    //     -9.52026805337528,
    //     -9.51415182635785,
    //     -9.50807278028147,
    //     -9.5020304658255,
    //     -9.49602444176529,
    //     -9.49005427477879,
    //     -9.48411953925897,
    //     -9.47821981713178,
    //     -9.47235469767939,
    //     -9.46652377736859,
    //     -9.46072665968427,
    //     -9.45496295496752,
    //     -9.44923228025853,
    //     -9.44353425914389,
    //     -9.43786852160822,
    //     -9.43223470388996,
    //     -9.42663244834129,
    //     -9.42106140329184,
    //     -9.41552122291622,
    //     -9.41001156710525,
    //     -9.40453210134062,
    //     -9.39908249657306,
    //     -9.39366242910372,
    //     -9.38827158046884,
    //     -9.38290963732746,
    //     -9.3775762913521,
    //     -9.3722712391224,
    //     -9.36699418202156,
    //     -9.36174482613542,
    //     -9.35652288215426,
    //     -9.35132806527716,
    //     -9.34616009511872,
    //     -9.3410186956183,
    //     -9.33590359495153,
    //     -9.33081452544406,
    //     -9.32575122348751,
    //     -9.32071342945755,
    //     -9.31570088763401,
    //     -9.31071334612297,
    //     -9.30575055678084,
    //     -9.30081227514026,
    //     -9.29589826033783,
    //     -9.29100827504364,
    //     -9.28614208539247,
    //     -9.28129946091668,
    //     -9.27648017448073,
    //     -9.27168400221724,
    //     -9.26691072346458,
    //     -9.26216012070598,
    //     -9.25743197951003,
    //     -9.25272608847262,
    //     -9.24804223916019,
    //     -9.24338022605438,
    //     -9.23873984649788,
    //     -9.23412090064159,
    //     -9.22952319139296,
    //     -9.22494652436555,
    //     -9.22039070782969,
    //     -9.21585555266429,
    //     -9.21134087230977,
    //     -9.20684648272193,
    //     -9.20237220232701,
    //     -9.19791785197763,
    //     -9.19348325490976,
    //     -9.18906823670064,
    //     -9.18467262522761,
    //     -9.18029625062781,
    //     -9.17593894525885,
    //     -9.17160054366025,
    //     -9.16728088251574,
    //     -9.16297980061635,
    //     -9.15869713882434,
    //     -9.15443274003789,
    //     -9.15018644915644,
    //     -9.14595811304691,
    //     -9.14174758051057,
    //     -9.13755470225054,
    //     -9.13337933084005,
    //     -9.12922132069139,
    //     -9.12508052802536,
    //     -9.1209568108415,
    //     -9.11685002888885,
    //     -9.11276004363732,
    //     -9.10868671824968,
    //     -9.10462991755407,
    //     -9.10058950801706,
    //     -9.09656535771734,
    //     -9.0925573363198,
    //     -9.08856531505026,
    //     -9.08458916667062,
    //     -9.08062876545453,
    //     -9.07668398716351,
    //     -9.07275470902362,
    //     -9.06884080970248,
    //     -9.06494216928683,
    //     -9.06105866926043,
    //     -9.05719019248251,
    //     -9.05333662316652,
    //     -9.04949784685935,
    //     -9.04567375042095,
    //     -9.04186422200428,
    //     -9.03806915103573,
    //     -9.03428842819582,
    //     -9.03052194540035,
    //     -9.0267695957818,
    //     -9.02303127367119,
    //     -9.01930687458021,
    //     -9.01559629518367,
    //     -9.01189943330235,
    //     -9.00821618788605,
    //     -9.00454645899709,
    //     -9.00089014779398,
    //     -8.99724715651547,
    //     -8.5,
    //     -8,
    //     -7.5,
    //     -7,
    //     -6.5,
    //     -6,
    //     -5.5,
    //     -5,
    //     -4.5,
    //     -4,
    //     -3.5
    // };
    // this->myRespFactors->GFNC = {
    //     -3.82504627770424,
    //     -2.94958855127061,
    //     -2.31854577219433,
    //     -1.84317655978799,
    //     -1.47196382992332,
    //     -1.17344226463675,
    //     -0.927557658773714,
    //     -0.721032914031718,
    //     -0.544766356151548,
    //     -0.392316536711836,
    //     -0.258991183388226,
    //     -0.141283034758844,
    //     -0.0365105214894286,
    //     0.057417392488423,
    //     0.142159207272362,
    //     0.219049742944705,
    //     0.289176937949946,
    //     0.353437237578563,
    //     0.412576355254843,
    //     0.467219821683262,
    //     0.517896257502242,
    //     0.565055362235109,
    //     0.609081999480424,
    //     0.650307352122957,
    //     0.689017847002224,
    //     0.72546235973319,
    //     0.759858078225233,
    //     0.792395309374975,
    //     0.823241445398292,
    //     0.852544256385428,
    //     0.880434638591264,
    //     0.90702892008443,
    //     0.932430804166138,
    //     0.956733014670319,
    //     0.980018694617433,
    //     1.00236259981024,
    //     1.02383212117121,
    //     1.04448816344005,
    //     1.06438590291301,
    //     1.08357544293834,
    //     1.10210238267726,
    //     1.12000831203627,
    //     1.13733124355265,
    //     1.15410599027308,
    //     1.17036449723202,
    //     1.18613613295138,
    //     1.20144794639993,
    //     1.21632489403239,
    //     1.23079004084411,
    //     1.24486473880417,
    //     1.25856878554754,
    //     1.27192056580055,
    //     1.28493717767003,
    //     1.29763454563495,
    //     1.31002752183152,
    //     1.32212997701113,
    //     1.33395488237003,
    //     1.3455143832948,
    //     1.35681986593489,
    //     1.36788201739885,
    //     1.3787108802727,
    //     1.38931590207312,
    //     1.39970598017468,
    //     1.40988950268609,
    //     1.41987438569473,
    //     1.42966810725029,
    //     1.43927773841579,
    //     1.4487099716774,
    //     1.45797114697193,
    //     1.46706727556239,
    //     1.47600406196712,
    //     1.48478692412571,
    //     1.49342101196577,
    //     1.50191122451739,
    //     1.51026222570671,
    //     1.51847845894703,
    //     1.52656416063357,
    //     1.53452337263758,
    //     1.54235995388604,
    //     1.55007759110481,
    //     1.55767980879548,
    //     1.56516997850982,
    //     1.57255132747916,
    //     1.57982694665127,
    //     1.58699979818216,
    //     1.59407272242593,
    //     1.60104844446199,
    //     1.60792958019558,
    //     1.61471864206396,
    //     1.62141804437843,
    //     1.62803010832916,
    //     1.63455706667791,
    //     1.64100106816136,
    //     1.64736418162613,
    //     1.65364839991457,
    //     1.65985564351893,
    //     1.66598776402026,
    //     1.67204654732691,
    //     1.6780337167262,
    //     1.68395093576222,
    //     1.68979981095116,
    //     1.69558189434515,
    //     1.70129868595449,
    //     1.70695163603745,
    //     1.71254214726623,
    //     1.71807157677701,
    //     1.72354123811132,
    //     1.72895240305556,
    //     1.73430630338503,
    //     1.7396041325182,
    //     1.74484704708686,
    //     1.75003616842704,
    //     1.75517258399541,
    //     1.76025734871581,
    //     1.76529148625965,
    //     1.77027599026423,
    //     1.77521182549251,
    //     1.78009992893764,
    //     1.78494121087534,
    //     1.78973655586707,
    //     1.79448682371675,
    //     1.79919285038346,
    //     1.80385544885269,
    //     1.80847540996824,
    //     1.81305350322689,
    //     1.81759047753788,
    //     1.82208706194898,
    //     1.82654396634093,
    //     1.83096188209186,
    //     1.83534148271331,
    //     1.83968342445907,
    //     1.84398834690856,
    //     1.84825687352565,
    //     1.85248961219447,
    //     1.85668715573306,
    //     1.86085008238616,
    //     1.86497895629802,
    //     1.8690743279662,
    //     1.87313673467741,
    //     1.87716670092596,
    //     1.88116473881595,
    //     1.88513134844774,
    //     1.88906701828947,
    //     1.89297222553444,
    //     1.8968474364448,
    //     1.90069310668226,
    //     1.90450968162655,
    //     1.90829759668184,
    //     1.912057277572,
    //     1.91578914062491,
    //     1.91949359304655,
    //     1.92317103318508,
    //     1.92682185078551,
    //     1.93044642723528,
    //     1.9340451358011,
    //     1.9376183418576,
    //     1.94116640310786,
    //     1.94468966979636,
    //     1.94818848491463,
    //     1.95166318439985,
    //     1.95511409732666,
    //     1.95854154609259,
    //     1.96194584659719,
    //     1.9653273084153,
    //     1.96868623496446,
    //     1.97202292366695,
    //     1.97533766610649,
    //     1.97863074817982,
    //     1.98190245024345,
    //     1.98515304725574,
    //     1.98838280891435,
    //     1.99159199978949,
    //     1.99478087945282,
    //     1.99794970260249,
    //     2.00109871918424,
    //     2.00422817450865,
    //     2.00733830936503,
    //     2.01042936013162,
    //     2.01350155888257,
    //     2.01655513349171,
    //     2.01959030773313,
    //     2.02260730137885,
    //     2.02560633029357,
    //     2.02858760652662,
    //     2.03155133840129,
    //     2.03449773060144,
    //     2.03742698425574,
    //     2.04033929701944,
    //     2.04323486315375,
    //     2.04611387360307,
    //     2.04897651606993,
    //     2.05182297508796,
    //     2.05465343209265,
    //     2.0574680654903,
    //     2.06026705072501,
    //     2.06305056034375,
    //     2.06581876405984,
    //     2.06857182881448,
    //     2.07130991883681,
    //     2.07403319570225,
    //     2.07674181838929,
    //     2.0794359433348,
    //     2.08211572448788,
    //     2.08478131336223,
    //     2.08743285908724,
    //     2.09007050845769,
    //     2.09269440598216,
    //     2.09530469393023,
    //     2.09790151237845,
    //     2.10048499925511,
    //     2.10305529038394,
    //     2.1056125195266,
    //     2.10815681842426,
    //     2.11068831683798,
    //     2.11320714258817,
    //     2.11571342159306,
    //     2.11820727790623,
    //     2.12068883375319,
    //     2.1231582095671,
    //     2.12561552402359,
    //     2.1280608940748,
    //     2.13049443498252,
    //     2.13291626035058,
    //     2.13532648215649,
    //     2.13772521078229,
    //     2.14011255504467,
    //     2.14248862222443,
    //     2.14485351809521,
    //     2.14720734695156,
    //     2.14955021163636,
    //     2.15188221356762,
    //     2.15420345276464,
    //     2.15651402787361,
    //     2.1588140361926,
    //     2.16110357369597,
    //     2.16338273505829,
    //     2.16565161367762,
    //     2.16791030169843,
    //     2.17015889003379,
    //     2.17239746838733,
    //     2.17462612527446,
    //     2.17684494804332,
    //     2.17905402289516,
    //     2.18125343490432,
    //     2.18344326803778,
    //     2.18562360517426,
    //     2.18779452812296,
    //     2.1899561176418,
    //     2.19210845345543,
    //     2.1942516142727,
    //     2.19638567780384,
    //     2.19851072077728,
    //     2.20062681895614,
    //     2.20273404715424,
    //     2.20483247925198,
    //     2.20692218821173,
    //     2.20900324609301,
    //     2.21107572406725,
    //     2.21313969243239,
    //     2.21519522062703,
    //     2.2172423772444,
    //     2.21928123004602,
    //     2.22131184597508,
    //     2.22333429116954,
    //     2.22534863097497,
    //     2.2273549299572,
    //     2.2293532519146,
    //     2.23134365989023,
    //     2.23332621618367,
    //     2.23530098236273,
    //     2.23726801927473,
    //     2.23922738705782,
    //     2.24117914515185,
    //     2.24312335230919,
    //     2.24506006660526,
    //     2.50871010336184,
    //     2.7562697918052,
    //     3.00386161809726,
    //     3.25136974953115,
    //     3.50267633625207,
    //     3.77709073690747,
    //     4.1130190039782,
    //     4.54924495008536,
    //     5.10215242707132,
    //     5.76038120041805,
    //     6.49558898328387
    // };
    int q = 1;
}

void GLHEVert::calcUniformBHWallTempGFunctions(EnergyPlusData &state) const
{
    // construct boreholes vector
    std::vector<gt::boreholes::Borehole> boreholes;
    for (const auto &bh : this->myRespFactors->myBorholes) {
        boreholes.emplace_back(bh->props->bhLength, bh->props->bhTopDepth, bh->props->bhDiameter / 2.0, bh->xLoc, bh->yLoc);
    }

    // Obtain number of segments by adaptive discretization
    gt::segments::adaptive adptDisc;
    const int nSegments = adptDisc.discretize(this->bhLength, this->totalTubeLength);

    this->myRespFactors->GFNC = gt::gfunction::uniform_borehole_wall_temperature(
        boreholes, this->myRespFactors->time, this->soil.diffusivity, nSegments, true, state.dataGlobal->numThread);
}

void GLHEVert::calcGFunctions(EnergyPlusData &state)
{

    // No other choice than to calculate the g-functions here
    // this->calcUniformBHWallTempGFunctions(state);
    this->setupTimeVectors();
    this->calcShortTimestepGFunctions(state);
    this->calcLongTimestepGFunctions(state);
    this->combineShortAndLongTimestepGFunctions();

    // save data for later
    if (state.files.outputControl.glhe && !state.dataSysVars->DisableGLHECaching) {
        myCacheData["Response Factors"]["time"] = std::vector<Real64>(this->myRespFactors->time);
        myCacheData["Response Factors"]["LNTTS"] = std::vector<Real64>(this->myRespFactors->LNTTS);
        myCacheData["Response Factors"]["GFNC"] = std::vector<Real64>(this->myRespFactors->GFNC.begin(), this->myRespFactors->GFNC.end());
        writeGLHECacheToFile(state);
    }
}

void GLHEVert::setupTimeVectors() const
{
    // Minimum simulation time for which finite line source method is applicable
    constexpr Real64 lntts_min_for_long_timestep = -8.5;

    // Time scale constant
    Real64 const t_s = pow_2(this->bhLength) / (9 * this->soil.diffusivity);

    // Temporary vector for holding the LNTTS vals
    std::vector<Real64> tempLNTTS;

    tempLNTTS.push_back(lntts_min_for_long_timestep);

    // Determine how many g-function pairs to generate based on user defined maximum simulation time
    while (true) {
        const Real64 maxPossibleSimTime = exp(tempLNTTS.back()) * t_s;
        constexpr int numDaysInYear = 365;
        if (maxPossibleSimTime < this->myRespFactors->maxSimYears * numDaysInYear * Constant::rHoursInDay * Constant::rSecsInHour) {
            constexpr Real64 lnttsStepSize = 0.5;
            tempLNTTS.push_back(tempLNTTS.back() + lnttsStepSize);
        } else {
            break;
        }
    }

    this->myRespFactors->LNTTS = tempLNTTS;
    this->myRespFactors->time = tempLNTTS;
    std::transform(this->myRespFactors->time.begin(), this->myRespFactors->time.end(), this->myRespFactors->time.begin(), [&t_s](auto const &c) {
        return exp(c) * t_s;
    });
    this->myRespFactors->GFNC = std::vector<Real64>(tempLNTTS.size(), 0.0);
}

void GLHEVert::calcUniformHeatFluxGFunctions(EnergyPlusData &state) const
{
    DisplayString(state, "Initializing GroundHeatExchanger:System: " + this->name);

    // Calculate the g-functions
    for (size_t lntts_index = 0; lntts_index < this->myRespFactors->LNTTS.size(); ++lntts_index) {
        for (auto const &bh_i : this->myRespFactors->myBorholes) {
            Real64 sum_T_ji = 0;
            for (auto const &bh_j : this->myRespFactors->myBorholes) {
                sum_T_ji += doubleIntegral(bh_i, bh_j, this->myRespFactors->time[lntts_index]);
            }
            this->myRespFactors->GFNC[lntts_index] += sum_T_ji;
        }
        this->myRespFactors->GFNC[lntts_index] /= (2 * this->totalTubeLength);

        std::stringstream ss;
        ss << std::fixed << std::setprecision(1) << static_cast<Real64>(lntts_index) / static_cast<Real64>(this->myRespFactors->LNTTS.size()) * 100.0;

        DisplayString(state, "...progress: " + ss.str() + "%");
    }
}

void GLHEVert::calcShortTimestepGFunctions(EnergyPlusData &state)
{
    // SUBROUTINE PARAMETER DEFINITIONS:
    std::string_view constexpr RoutineName = "calcShortTimestepGFunctions";

    enum class CellType
    {
        Invalid = -1,
        FLUID,
        CONVECTION,
        PIPE,
        GROUT,
        SOIL,
        Num
    };

    struct Cell
    {
        CellType type;
        Real64 radius_center = 0.0;
        Real64 radius_outer = 0.0;
        Real64 radius_inner = 0.0;
        Real64 thickness = 0.0;
        Real64 vol = 0.0;
        Real64 conductivity = 0.0;
        Real64 rhoCp = 0.0;
        Real64 temperature = 0.0;
        Real64 temperature_prev_ts = 0.0;
    };

    // vector to hold 1-D cells
    std::vector<Cell> Cells;

    // setup pipe, convection, and fluid layer geometries
    constexpr int num_pipe_cells = 4;
    constexpr int num_conv_cells = 1;
    constexpr int num_fluid_cells = 3;
    Real64 const pipe_thickness = this->pipe.thickness;
    Real64 const pcf_cell_thickness = pipe_thickness / num_pipe_cells;
    Real64 const radius_pipe_out = std::sqrt(2) * this->pipe.outRadius;
    Real64 const radius_pipe_in = radius_pipe_out - pipe_thickness;
    Real64 const radius_conv = radius_pipe_in - num_conv_cells * pcf_cell_thickness;
    Real64 const radius_fluid = radius_conv - (num_fluid_cells - 0.5) * pcf_cell_thickness; // accounts for half thickness of boundary cell

    // setup grout layer geometry
    constexpr int num_grout_cells = 27;
    Real64 const radius_grout = this->bhRadius;
    Real64 const grout_cell_thickness = (radius_grout - radius_pipe_out) / num_grout_cells;

    // setup soil layer geometry
    constexpr int num_soil_cells = 500;
    constexpr Real64 radius_soil = 10.0;
    Real64 const soil_cell_thickness = (radius_soil - radius_grout) / num_soil_cells;

    // use design flow rate
    this->massFlowRate = this->designMassFlow;

    // calculate equivalent thermal resistance between borehole wall and fluid
    Real64 bhResistance = calcBHAverageResistance(state);
    Real64 bhConvectionResistance = calcPipeConvectionResistance(state);
    Real64 bh_equivalent_resistance_tube_grout = bhResistance - bhConvectionResistance / 2.0;
    Real64 bh_equivalent_resistance_convection = bhResistance - bh_equivalent_resistance_tube_grout;

    Real64 initial_temperature = this->inletTemp;
    Real64 cpFluid_init = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, initial_temperature, RoutineName);
    Real64 fluidDensity_init = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, initial_temperature, RoutineName);

    // initialize the fluid cells
    for (int i = 0; i < num_fluid_cells; ++i) {
        Cell thisCell;
        thisCell.type = CellType::FLUID;
        thisCell.thickness = pcf_cell_thickness;
        thisCell.radius_center = radius_fluid + i * thisCell.thickness;

        // boundary cell is only half thickness
        if (i == 0) {
            thisCell.radius_inner = thisCell.radius_center;
        } else {
            thisCell.radius_inner = thisCell.radius_center - thisCell.thickness / 2.0;
        }

        thisCell.radius_outer = thisCell.radius_center + thisCell.thickness / 2.0;
        thisCell.conductivity = 200;
        thisCell.rhoCp = 2.0 * cpFluid_init * fluidDensity_init * pow_2(this->pipe.innerRadius) / (pow_2(radius_conv) - pow_2(radius_fluid));
        Cells.push_back(thisCell);
    }

    // initialize the convection cells
    for (int i = 0; i < num_conv_cells; ++i) {
        Cell thisCell;
        thisCell.thickness = pcf_cell_thickness;
        thisCell.radius_inner = radius_conv + i * thisCell.thickness;
        thisCell.radius_center = thisCell.radius_inner + thisCell.thickness / 2.0;
        thisCell.radius_outer = thisCell.radius_inner + thisCell.thickness;
        thisCell.conductivity = log(radius_pipe_in / radius_conv) / (2 * Constant::Pi * bh_equivalent_resistance_convection);
        thisCell.rhoCp = 1;
        Cells.push_back(thisCell);
    }

    // initialize pipe cells
    for (int i = 0; i < num_pipe_cells; ++i) {
        Cell thisCell;
        thisCell.type = CellType::PIPE;
        thisCell.thickness = pcf_cell_thickness;
        thisCell.radius_inner = radius_pipe_in + i * thisCell.thickness;
        thisCell.radius_center = thisCell.radius_inner + thisCell.thickness / 2.0;
        thisCell.radius_outer = thisCell.radius_inner + thisCell.thickness;
        thisCell.conductivity = log(radius_grout / radius_pipe_in) / (2 * Constant::Pi * bh_equivalent_resistance_tube_grout);
        thisCell.rhoCp = this->pipe.rhoCp;
        Cells.push_back(thisCell);
    }

    // initialize grout cells
    for (int i = 0; i < num_grout_cells; ++i) {
        Cell thisCell;
        thisCell.type = CellType::GROUT;
        thisCell.thickness = grout_cell_thickness;
        thisCell.radius_inner = radius_pipe_out + i * thisCell.thickness;
        thisCell.radius_center = thisCell.radius_inner + thisCell.thickness / 2.0;
        thisCell.radius_outer = thisCell.radius_inner + thisCell.thickness;
        thisCell.conductivity = log(radius_grout / radius_pipe_in) / (2 * Constant::Pi * bh_equivalent_resistance_tube_grout);
        thisCell.rhoCp = grout.rhoCp;
        Cells.push_back(thisCell);
    }

    // initialize soil cells
    for (int i = 0; i < num_soil_cells; ++i) {
        Cell thisCell;
        thisCell.type = CellType::SOIL;
        thisCell.thickness = soil_cell_thickness;
        thisCell.radius_inner = radius_grout + i * thisCell.thickness;
        thisCell.radius_center = thisCell.radius_inner + thisCell.thickness / 2.0;
        thisCell.radius_outer = thisCell.radius_inner + thisCell.thickness;
        thisCell.conductivity = this->soil.k;
        thisCell.rhoCp = this->soil.rhoCp;
        Cells.push_back(thisCell);
    }

    // other non-geometric specific setup
    for (auto &thisCell : Cells) {
        thisCell.vol = Constant::Pi * (pow_2(thisCell.radius_outer) - pow_2(thisCell.radius_inner));
        thisCell.temperature = initial_temperature;
    }

    // set upper limit of time for the short time-step g-function calcs so there is some overlap
    Real64 constexpr lntts_max_for_short_timestep = -8.6;
    Real64 const t_s = pow_2(this->bhLength) / (9.0 * this->soil.diffusivity);

    Real64 const time_max_for_short_timestep = exp(lntts_max_for_short_timestep) * t_s;
    Real64 total_time = 0.0;

    // time step loop
    while (total_time < time_max_for_short_timestep) {
        Real64 constexpr heat_flux = 1.0; // 40.4;
        Real64 constexpr time_step = 120; // 500.0;

        for (auto &thisCell : Cells) {
            thisCell.temperature_prev_ts = thisCell.temperature;
        }

        std::vector<Real64> a;
        std::vector<Real64> b;
        std::vector<Real64> c;
        std::vector<Real64> d;

        // setup tdma matrices
        unsigned int num_cells = Cells.size();
        for (int cell_index = 0; cell_index < num_cells; ++cell_index) {
            if (cell_index == 0) {
                // heat flux BC

                auto const &thisCell = Cells[cell_index];
                auto const &eastCell = Cells[cell_index + 1];

                Real64 FE1 = log(thisCell.radius_outer / thisCell.radius_center) / (2 * Constant::Pi * thisCell.conductivity);
                Real64 FE2 = log(eastCell.radius_center / eastCell.radius_inner) / (2 * Constant::Pi * eastCell.conductivity);
                Real64 AE = 1 / (FE1 + FE2);

                Real64 AD = thisCell.rhoCp * thisCell.vol / time_step;

                a.push_back(0);
                b.push_back(-AE / AD - 1);
                c.push_back(AE / AD);
                d.push_back(-thisCell.temperature_prev_ts - heat_flux / AD);

            } else if (cell_index == num_cells - 1) {
                // const ground temp bc

                auto &thisCell = Cells[cell_index];

                a.push_back(0);
                b.push_back(1);
                c.push_back(0);
                d.push_back(thisCell.temperature_prev_ts);

            } else {
                // all other cells

                auto const &westCell = Cells[cell_index - 1];
                auto const &eastCell = Cells[cell_index + 1];
                auto const &thisCell = Cells[cell_index];

                Real64 FE1 = log(thisCell.radius_outer / thisCell.radius_center) / (2 * Constant::Pi * thisCell.conductivity);
                Real64 FE2 = log(eastCell.radius_center / eastCell.radius_inner) / (2 * Constant::Pi * eastCell.conductivity);
                Real64 AE = 1 / (FE1 + FE2);

                Real64 FW1 = log(westCell.radius_outer / westCell.radius_center) / (2 * Constant::Pi * westCell.conductivity);
                Real64 FW2 = log(thisCell.radius_center / thisCell.radius_inner) / (2 * Constant::Pi * thisCell.conductivity);
                Real64 AW = -1 / (FW1 + FW2);

                Real64 AD = thisCell.rhoCp * thisCell.vol / time_step;

                a.push_back(-AW / AD);
                b.push_back(AW / AD - AE / AD - 1);
                c.push_back(AE / AD);
                d.push_back(-thisCell.temperature_prev_ts);
            }
        } // end tdma setup

        // solve for new temperatures
        std::vector<Real64> new_temps = TDMA(a, b, c, d);

        for (int cell_index = 0; cell_index < num_cells; ++cell_index) {
            Cells[cell_index].temperature = new_temps[cell_index];
        }

        // The T_bhWall variable was only ever calculated, ever actually used.  I'm commenting it out for now.
        // // calculate bh wall temp
        // Real64 T_bhWall = 0.0;
        // for (int cell_index = 0; cell_index < num_cells; ++cell_index) {
        //     auto const &leftCell = Cells[cell_index];
        //     auto const &rightCell = Cells[cell_index + 1];
        //
        //     if (leftCell.type == CellType::GROUT && rightCell.type == CellType::SOIL) {
        //
        //         Real64 left_conductance = 2 * Constant::Pi * leftCell.conductivity / log(leftCell.radius_outer / leftCell.radius_inner);
        //         Real64 right_conductance = 2 * Constant::Pi * rightCell.conductivity / log(rightCell.radius_center / leftCell.radius_inner);
        //
        //         T_bhWall =
        //             (left_conductance * leftCell.temperature + right_conductance * rightCell.temperature) / (left_conductance + right_conductance);
        //         break;
        //     }
        // }

        total_time += time_step;

        GFNC_shortTimestep.push_back(2 * Constant::Pi * this->soil.k * ((Cells[0].temperature - initial_temperature) / heat_flux - bhResistance));
        LNTTS_shortTimestep.push_back(log(total_time / t_s));
    } // end timestep loop

    std::ostringstream l, g;
    for (auto const val : LNTTS_shortTimestep) {
        l << val << '\n';
    }
    for (auto const val : GFNC_shortTimestep) {
        g << val << '\n';
    }
    std::string l2 = l.str();
    std::string g2 = g.str();
}

Real64 GLHEVert::calcBHAverageResistance(EnergyPlusData &state)
{
    // Calculates the average thermal resistance of the borehole using the first-order multipole method.
    // Javed, S. & Spitler, J.D. 2016. 'Accuracy of Borehole Thermal Resistance Calculation Methods
    // for Grouted Single U-tube Ground Heat Exchangers.' Applied Energy.187:790-806.
    // Equation 13

    Real64 const beta = 2 * Constant::Pi * this->grout.k * calcPipeResistance(state);

    Real64 const final_term_1 = log(this->theta_2 / (2 * this->theta_1 * pow(1 - pow_4(this->theta_1), this->sigma)));
    Real64 const num_final_term_2 = pow_2(this->theta_3) * pow_2(1 - (4 * this->sigma * pow_4(this->theta_1)) / (1 - pow_4(this->theta_1)));
    Real64 const den_final_term_2_pt_1 = (1 + beta) / (1 - beta);
    Real64 const den_final_term_2_pt_2 = pow_2(this->theta_3) * (1 + (16 * this->sigma * pow_4(this->theta_1)) / pow_2(1 - pow_4(this->theta_1)));
    Real64 const den_final_term_2 = den_final_term_2_pt_1 + den_final_term_2_pt_2;
    Real64 const final_term_2 = num_final_term_2 / den_final_term_2;

    return (1 / (4 * Constant::Pi * this->grout.k)) * (beta + final_term_1 - final_term_2);
}

//******************************************************************************

Real64 GLHEVert::calcBHTotalInternalResistance(EnergyPlusData &state)
{
    // Calculates the total internal thermal resistance of the borehole using the first-order multipole method.
    // Javed, S. & Spitler, J.D. 2016. 'Accuracy of Borehole Thermal Resistance Calculation Methods
    // for Grouted Single U-tube Ground Heat Exchangers.' Applied Energy. 187:790-806.
    // Equation 26

    Real64 beta = 2 * Constant::Pi * this->grout.k * calcPipeResistance(state);

    Real64 final_term_1 = log(pow(1 + pow_2(this->theta_1), this->sigma) / (this->theta_3 * pow(1 - pow_2(this->theta_1), this->sigma)));
    Real64 num_term_2 = pow_2(this->theta_3) * pow_2(1 - pow_4(this->theta_1) + 4 * this->sigma * pow_2(this->theta_1));
    Real64 den_term_2_pt_1 = (1 + beta) / (1 - beta) * pow_2(1 - pow_4(this->theta_1));
    Real64 den_term_2_pt_2 = pow_2(this->theta_3) * pow_2(1 - pow_4(this->theta_1));
    Real64 den_term_2_pt_3 = 8 * this->sigma * pow_2(this->theta_1) * pow_2(this->theta_3) * (1 + pow_4(this->theta_1));
    Real64 den_term_2 = den_term_2_pt_1 - den_term_2_pt_2 + den_term_2_pt_3;
    Real64 final_term_2 = num_term_2 / den_term_2;

    return (1 / (Constant::Pi * this->grout.k)) * (beta + final_term_1 - final_term_2);
}

//******************************************************************************

Real64 GLHEVert::calcBHGroutResistance(EnergyPlusData &state)
{
    // Calculates grout resistance. Use for validation.
    // Javed, S. & Spitler, J.D. 2016. 'Accuracy of Borehole Thermal Resistance Calculation Methods
    // for Grouted Single U-tube Ground Heat Exchangers.' Applied Energy. 187:790-806.
    // Equation 3

    return calcBHAverageResistance(state) - calcPipeResistance(state) / 2.0;
}

//******************************************************************************

Real64 GLHEVert::calcHXResistance(EnergyPlusData &state)
{
    // Calculates the effective thermal resistance of the borehole assuming a uniform heat flux.
    // Javed, S. & Spitler, J.D. Calculation of Borehole Thermal Resistance. In 'Advances in
    // Ground-Source Heat Pump Systems,' pp. 84. Rees, S.J. ed. Cambridge, MA. Elsevier Ltd. 2016.
    // Eq: 3-67

    if (this->massFlowRate <= 0.0) {
        return 0;
    }
    constexpr std::string_view RoutineName = "calcBHResistance";
    Real64 const cpFluid = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, this->inletTemp, RoutineName);
    return calcBHAverageResistance(state) + 1 / (3 * calcBHTotalInternalResistance(state)) * pow_2(this->bhLength / (this->massFlowRate * cpFluid));
}

//******************************************************************************

Real64 GLHEVert::calcPipeConductionResistance() const
{
    // Calculates the thermal resistance of a pipe, in [K/(W/m)].
    // Javed, S. & Spitler, J.D. 2016. 'Accuracy of Borehole Thermal Resistance Calculation Methods
    // for Grouted Single U-tube Ground Heat Exchangers.' Applied Energy. 187:790-806.

    return log(this->pipe.outDia / this->pipe.innerDia) / (2 * Constant::Pi * this->pipe.k);
}

//******************************************************************************

Real64 GLHEVert::calcPipeConvectionResistance(EnergyPlusData &state)
{
    // Calculates the convection resistance using Gnielinski and Petukov, in [K/(W/m)]
    // Gneilinski, V. 1976. 'New equations for heat and mass transfer in turbulent pipe and channel flow.'
    // International Chemical Engineering 16(1976), pp. 359-368.

    // SUBROUTINE PARAMETER DEFINITIONS:
    constexpr std::string_view RoutineName("calcPipeConvectionResistance");

    // Get fluid props
    this->inletTemp = state.dataLoopNodes->Node(this->inletNodeNum).Temp;

    Real64 const cpFluid = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getSpecificHeat(state, this->inletTemp, RoutineName);
    Real64 const kFluid = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getConductivity(state, this->inletTemp, RoutineName);
    Real64 const fluidViscosity = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getViscosity(state, this->inletTemp, RoutineName);

    // Smoothing fit limits
    constexpr Real64 lower_limit = 2000;
    constexpr Real64 upper_limit = 4000;

    Real64 const bhMassFlowRate = this->massFlowRate / this->myRespFactors->numBoreholes;
    Real64 const reynoldsNum = 4 * bhMassFlowRate / (fluidViscosity * Constant::Pi * this->pipe.innerDia);

    Real64 nusseltNum = 0.0;
    if (reynoldsNum < lower_limit) {
        nusseltNum = 4.01; // laminar mean(4.36, 3.66)
    } else if (reynoldsNum < upper_limit) {
        Real64 constexpr nu_low = 4.01;               // laminar
        Real64 const f = frictionFactor(reynoldsNum); // turbulent
        Real64 const prandtlNum = (cpFluid * fluidViscosity) / (kFluid);
        Real64 const nu_high = (f / 8) * (reynoldsNum - 1000) * prandtlNum / (1 + 12.7 * std::sqrt(f / 8) * (pow(prandtlNum, 2.0 / 3.0) - 1));
        Real64 const sf = 1 / (1 + std::exp(-(reynoldsNum - 3000) / 150.0)); // smoothing function
        nusseltNum = (1 - sf) * nu_low + sf * nu_high;
    } else {
        Real64 const f = frictionFactor(reynoldsNum);
        Real64 const prandtlNum = (cpFluid * fluidViscosity) / (kFluid);
        nusseltNum = (f / 8) * (reynoldsNum - 1000) * prandtlNum / (1 + 12.7 * std::sqrt(f / 8) * (pow(prandtlNum, 2.0 / 3.0) - 1));
    }

    Real64 h = nusseltNum * kFluid / this->pipe.innerDia;

    return 1 / (h * Constant::Pi * this->pipe.innerDia);
}

//******************************************************************************

Real64 GLHEVert::frictionFactor(Real64 const reynoldsNum)
{
    // Calculates the friction factor in smooth tubes
    // Petukov, B.S. 1970. 'Heat transfer and friction in turbulent pipe flow with variable physical properties.'
    // In Advances in Heat Transfer, ed. T.F. Irvine and J.P. Hartnett, Vol. 6. New York Academic Press.

    // limits picked to be within about 1% of actual values
    constexpr Real64 lower_limit = 1500;
    constexpr Real64 upper_limit = 5000;

    if (reynoldsNum < lower_limit) {
        return 64.0 / reynoldsNum; // pure laminar flow
    } else if (reynoldsNum < upper_limit) {
        // pure laminar flow
        Real64 const f_low = 64.0 / reynoldsNum;
        // pure turbulent flow
        Real64 const f_high = pow(0.79 * log(reynoldsNum) - 1.64, -2.0);
        Real64 const sf = 1 / (1 + exp(-(reynoldsNum - 3000.0) / 450.0)); // smoothing function
        return (1 - sf) * f_low + sf * f_high;
    } else {
        return pow(0.79 * log(reynoldsNum) - 1.64, -2.0); // pure turbulent flow
    }
}

Real64 GLHEVert::calcPipeResistance(EnergyPlusData &state)
{
    // Calculates the combined conduction and convection pipe resistance
    // Javed, S. & Spitler, J.D. 2016. 'Accuracy of Borehole Thermal Resistance Calculation Methods
    // for Grouted Single U-tube Ground Heat Exchangers.' J. Energy Engineering. Draft in progress.
    // Equation 3

    return calcPipeConductionResistance() + calcPipeConvectionResistance(state);
}

Real64 GLHEVert::getGFunc(Real64 const time)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Matt Mitchell
    //       DATE WRITTEN:    February 2015

    // PURPOSE OF THIS SUBROUTINE:
    // Gets the g-function for vertical GHXs Note: Base e here.

    Real64 LNTTS = std::log(time);
    Real64 gFuncVal = interpGFunc(LNTTS);
    Real64 RATIO = this->bhRadius / this->bhLength;

    if (RATIO != this->myRespFactors->gRefRatio) {
        gFuncVal -= std::log(this->bhRadius / (this->bhLength * this->myRespFactors->gRefRatio));
    }

    return gFuncVal;
}

void GLHEVert::initGLHESimVars(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR:          Dan Fisher
    //       DATE WRITTEN:    August, 2000
    //       MODIFIED         Arun Murugappan

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 currTime = ((state.dataGlobal->DayOfSim - 1) * 24 + (state.dataGlobal->HourOfDay - 1) +
                       (state.dataGlobal->TimeStep - 1) * state.dataGlobal->TimeStepZone + state.dataHVACGlobal->SysTimeElapsed) *
                      Constant::rSecsInHour;

    if (this->myEnvrnFlag && state.dataGlobal->BeginEnvrnFlag) {
        this->initEnvironment(state, currTime);
    }

    // Calculate the average ground temperature over the depth of the borehole
    Real64 minDepth = this->myRespFactors->props->bhTopDepth;
    Real64 maxDepth = this->myRespFactors->props->bhLength + minDepth;
    Real64 oneQuarterDepth = minDepth + (maxDepth - minDepth) * 0.25;
    Real64 halfDepth = minDepth + (maxDepth - minDepth) * 0.5;
    Real64 threeQuarterDepth = minDepth + (maxDepth - minDepth) * 0.75;

    this->tempGround = 0;

    this->tempGround += this->groundTempModel->getGroundTempAtTimeInSeconds(state, minDepth, currTime);
    this->tempGround += this->groundTempModel->getGroundTempAtTimeInSeconds(state, maxDepth, currTime);
    this->tempGround += this->groundTempModel->getGroundTempAtTimeInSeconds(state, oneQuarterDepth, currTime);
    this->tempGround += this->groundTempModel->getGroundTempAtTimeInSeconds(state, halfDepth, currTime);
    this->tempGround += this->groundTempModel->getGroundTempAtTimeInSeconds(state, threeQuarterDepth, currTime);

    this->tempGround /= 5;

    this->massFlowRate = PlantUtilities::RegulateCondenserCompFlowReqOp(state, this->plantLoc, this->designMassFlow);

    PlantUtilities::SetComponentFlowRate(state, this->massFlowRate, this->inletNodeNum, this->outletNodeNum, this->plantLoc);

    // Reset local environment init flag
    if (!state.dataGlobal->BeginEnvrnFlag) {
        this->myEnvrnFlag = true;
    }
}

//******************************************************************************

void GLHEVert::initEnvironment(EnergyPlusData &state, [[maybe_unused]] Real64 const CurTime)
{
    constexpr std::string_view RoutineName = "initEnvironment";
    this->myEnvrnFlag = false;

    Real64 fluidDensity = state.dataPlnt->PlantLoop(this->plantLoc.loopNum).glycol->getDensity(state, 20.0, RoutineName);
    this->designMassFlow = this->designFlow * fluidDensity;
    PlantUtilities::InitComponentNodes(state, 0.0, this->designMassFlow, this->inletNodeNum, this->outletNodeNum);

    this->lastQnSubHr = 0.0;
    state.dataLoopNodes->Node(this->inletNodeNum).Temp = this->tempGround;
    state.dataLoopNodes->Node(this->outletNodeNum).Temp = this->tempGround;

    // zero out all history arrays
    this->QnHr = 0.0;
    this->QnMonthlyAgg = 0.0;
    this->QnSubHr = 0.0;
    this->LastHourN = 0;
    this->prevTimeSteps = 0.0;
    this->currentSimTime = 0.0;
    this->QGLHE = 0.0;
    this->prevHour = 1;
}

//******************************************************************************

void GLHEVert::oneTimeInit_new(EnergyPlusData &state)
{
    // Locate the hx on the plant loops for later usage
    bool errFlag = false;
    PlantUtilities::ScanPlantLoopsForObject(
        state, this->name, DataPlant::PlantEquipmentType::GrndHtExchgSystem, this->plantLoc, errFlag, _, _, _, _, _);
    if (errFlag) {
        ShowFatalError(state, "initGLHESimVars: Program terminated due to previous condition(s).");
    }
}

void GLHEVert::oneTimeInit([[maybe_unused]] EnergyPlusData &state)
{
}

} // namespace EnergyPlus::GroundHeatExchangers