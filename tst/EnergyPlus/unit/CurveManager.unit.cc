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

// Google Test Headers
#include <gtest/gtest.h>

// EnergyPlus Headers
#include <EnergyPlus/ConfiguredFunctions.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/FileSystem.hh>

#include "Fixtures/EnergyPlusFixture.hh"

using namespace EnergyPlus;

TEST_F(EnergyPlusFixture, CurveExponentialSkewNormal_MaximumCurveOutputTest)
{
    std::string const idf_objects = delimited_string({
        "Curve:ExponentialSkewNormal,",
        "  FanEff120CPLANormal,     !- Name",
        "  0.072613,                !- Coefficient1 C1",
        "  0.833213,                !- Coefficient2 C2",
        "  0.,                      !- Coefficient3 C3",
        "  0.013911,                !- Coefficient4 C4",
        "  -4.,                     !- Minimum Value of x",
        "  5.,                      !- Maximum Value of x",
        "  0.1,                     !- Minimum Curve Output",
        "  1.;                      !- Maximum Curve Output",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_EQ(0, state->dataCurveManager->NumCurves);
    Curve::GetCurveInput(*state);
    state->dataCurveManager->GetCurvesInputFlag = false;
    ASSERT_EQ(1, state->dataCurveManager->NumCurves);

    EXPECT_EQ(1.0, state->dataCurveManager->PerfCurve(1)->outputLimits.max);
    EXPECT_TRUE(state->dataCurveManager->PerfCurve(1)->outputLimits.maxPresent);

    EXPECT_EQ(0.1, state->dataCurveManager->PerfCurve(1)->outputLimits.min);
    EXPECT_TRUE(state->dataCurveManager->PerfCurve(1)->outputLimits.minPresent);
}

TEST_F(EnergyPlusFixture, QuadraticCurve)
{
    std::string const idf_objects = delimited_string({
        "Curve:QuadLinear,",
        "  MinDsnWBCurveName, ! Curve Name",
        "  -3.3333,           ! CoefficientC1",
        "  0.1,               ! CoefficientC2",
        "  38.9,              ! CoefficientC3",
        "  0.1,                ! CoefficientC4",
        "  0.5,                ! CoefficientC5",
        "  -30.,              ! Minimum Value of w",
        "  40.,               ! Maximum Value of w",
        "  0.,                ! Minimum Value of x",
        "  1.,                ! Maximum Value of x",
        "  5.,                ! Minimum Value of y",
        "  38.,               ! Maximum Value of y",
        "  0,                 ! Minimum Value of z",
        "  20,                ! Maximum Value of z",
        "  0.,                ! Minimum Curve Output",
        "  38.;               ! Maximum Curve Output",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_EQ(0, state->dataCurveManager->NumCurves);
    Curve::GetCurveInput(*state);
    state->dataCurveManager->GetCurvesInputFlag = false;
    ASSERT_EQ(1, state->dataCurveManager->NumCurves);

    EXPECT_EQ(38.0, state->dataCurveManager->PerfCurve(1)->outputLimits.max);
    EXPECT_TRUE(state->dataCurveManager->PerfCurve(1)->outputLimits.maxPresent);

    EXPECT_EQ(0., state->dataCurveManager->PerfCurve(1)->outputLimits.min);
    EXPECT_TRUE(state->dataCurveManager->PerfCurve(1)->outputLimits.minPresent);

    double var1 = 1, var2 = 0.1, var3 = 20, var4 = 10;
    double expected_value = -3.3333 + (0.1 * 1) + (38.9 * 0.1) + (0.1 * 20) + (0.5 * 10);
    EXPECT_EQ(expected_value, Curve::CurveValue(*state, 1, var1, var2, var3, var4));
}

TEST_F(EnergyPlusFixture, QuintLinearCurve)
{
    std::string const idf_objects = delimited_string({
        "Curve:QuintLinear,",
        "  MinDsnWBCurveName, ! Curve Name",
        "  -3.3333,           ! CoefficientC1",
        "  0.1,               ! CoefficientC2",
        "  38.9,              ! CoefficientC3",
        "  0.1,                ! CoefficientC4",
        "  0.5,                ! CoefficientC5",
        "  1.5,                ! CoefficientC6",
        "  0.,                ! Minimum Value of v",
        "  10.,               ! Maximum Value of v",
        "  -30.,              ! Minimum Value of w",
        "  40.,               ! Maximum Value of w",
        "  0.,                ! Minimum Value of x",
        "  1.,                ! Maximum Value of x",
        "  5.,                ! Minimum Value of y",
        "  38.,               ! Maximum Value of y",
        "  0,                 ! Minimum Value of z",
        "  20,                ! Maximum Value of z",
        "  0.,                ! Minimum Curve Output",
        "  38.;               ! Maximum Curve Output",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_EQ(0, state->dataCurveManager->NumCurves);
    Curve::GetCurveInput(*state);
    state->dataCurveManager->GetCurvesInputFlag = false;
    ASSERT_EQ(1, state->dataCurveManager->NumCurves);

    EXPECT_EQ(38.0, state->dataCurveManager->PerfCurve(1)->outputLimits.max);
    EXPECT_TRUE(state->dataCurveManager->PerfCurve(1)->outputLimits.maxPresent);

    EXPECT_EQ(0., state->dataCurveManager->PerfCurve(1)->outputLimits.min);
    EXPECT_TRUE(state->dataCurveManager->PerfCurve(1)->outputLimits.minPresent);

    double var1 = 1, var2 = 0.1, var3 = 0.5, var4 = 10, var5 = 15;
    double expected_value = -3.3333 + (0.1 * 1) + (38.9 * 0.1) + (0.1 * 0.5) + (0.5 * 10) + (1.5 * 15);
    EXPECT_EQ(expected_value, Curve::CurveValue(*state, 1, var1, var2, var3, var4, var5));
}

TEST_F(EnergyPlusFixture, TableLookup)
{
    std::string const idf_objects = delimited_string({"Table:IndependentVariable,",
                                                      "  SAFlow,                    !- Name",
                                                      "  Cubic,                     !- Interpolation Method",
                                                      "  Constant,                  !- Extrapolation Method",
                                                      "  0.714,                     !- Minimum Value",
                                                      "  1.2857,                    !- Maximum Value",
                                                      "  ,                          !- Normalization Reference Value",
                                                      "  Dimensionless,             !- Unit Type",
                                                      "  ,                          !- External File Name",
                                                      "  ,                          !- External File Column Number",
                                                      "  ,                          !- External File Starting Row Number",
                                                      "  0.714286,                  !- Value 1",
                                                      "  1.0,",
                                                      "  1.2857;",
                                                      "Table:IndependentVariableList,",
                                                      "  SAFlow_Variables,          !- Name",
                                                      "  SAFlow;                    !- Independent Variable 1 Name",
                                                      "Table:Lookup,",
                                                      "  CoolCapModFuncOfSAFlow,    !- Name",
                                                      "  SAFlow_Variables,          !- Independent Variable List Name",
                                                      "  ,                          !- Normalization Method",
                                                      "  ,                          !- Normalization Divisor",
                                                      "  0.8234,                    !- Minimum Output",
                                                      "  1.1256,                    !- Maximum Output",
                                                      "  Dimensionless,             !- Output Unit Type",
                                                      "  ,                          !- External File Name",
                                                      "  ,                          !- External File Column Number",
                                                      "  ,                          !- External File Starting Row Number",
                                                      "  0.823403,                  !- Output Value 1",
                                                      "  1.0,",
                                                      "  1.1256;",
                                                      "Table:Lookup,",
                                                      "  HeatCapModFuncOfSAFlow,    !- Name",
                                                      "  SAFlow_Variables,          !- Independent Variable List Name",
                                                      "  ,                          !- Normalization Method",
                                                      "  ,                          !- Normalization Divisor",
                                                      "  0.8554,                    !- Minimum Output",
                                                      "  1.0778,                    !- Maximum Output",
                                                      "  Dimensionless,             !- Output Unit Type",
                                                      "  ,                          !- External File Name",
                                                      "  ,                          !- External File Column Number",
                                                      "  ,                          !- External File Starting Row Number",
                                                      "  0.8554,                    !- Output Value 1",
                                                      "  1.0,",
                                                      "  1.0778;",
                                                      "Table:IndependentVariable,",
                                                      "  WaterFlow,                 !- Name",
                                                      "  Cubic,                     !- Interpolation Method",
                                                      "  Constant,                  !- Extrapolation Method",
                                                      "  0.0,                       !- Minimum Value",
                                                      "  1.333333,                  !- Maximum Value",
                                                      "  ,                          !- Normalization Reference Value",
                                                      "  Dimensionless,             !- Unit Type",
                                                      "  ,                          !- External File Name",
                                                      "  ,                          !- External File Column Number",
                                                      "  ,                          !- External File Starting Row Number",
                                                      "  0.0,                       !- Value 1,",
                                                      "  0.05,",
                                                      "  0.33333,",
                                                      "  0.5,",
                                                      "  0.666667,",
                                                      "  0.833333,",
                                                      "  1.0,",
                                                      "  1.333333;",
                                                      "Table:IndependentVariableList,",
                                                      "  WaterFlow_Variables,       !- Name",
                                                      "  WaterFlow;                 !- Independent Variable 1 Name",
                                                      "Table:Lookup,",
                                                      "  CapModFuncOfWaterFlow,     !- Name",
                                                      "  WaterFlow_Variables,       !- Independent Variable List Name",
                                                      "  ,                          !- Normalization Method",
                                                      "  ,                          !- Normalization Divisor",
                                                      "  0.0,                       !- Minimum Output",
                                                      "  1.04,                      !- Maximum Output",
                                                      "  Dimensionless,             !- Output Unit Type",
                                                      "  ,                          !- External File Name",
                                                      "  ,                          !- External File Column Number",
                                                      "  ,                          !- External File Starting Row Number",
                                                      "  0.0,                       !- Output Value 1",
                                                      "  0.001,",
                                                      "  0.71,",
                                                      "  0.85,",
                                                      "  0.92,",
                                                      "  0.97,",
                                                      "  1.0,",
                                                      "  1.04;"});

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_EQ(0, state->dataCurveManager->NumCurves);

    Curve::GetCurveInput(*state);
    state->dataCurveManager->GetCurvesInputFlag = false;
    ASSERT_EQ(3, state->dataCurveManager->NumCurves);

    EXPECT_ENUM_EQ(Curve::InterpType::BtwxtMethod, state->dataCurveManager->PerfCurve(1)->interpolationType);
    EXPECT_EQ("CAPMODFUNCOFWATERFLOW", state->dataCurveManager->PerfCurve(1)->Name);

    EXPECT_TRUE(state->dataCurveManager->PerfCurve(1)->outputLimits.minPresent);
    EXPECT_EQ(0.0, state->dataCurveManager->PerfCurve(1)->outputLimits.min);

    EXPECT_TRUE(state->dataCurveManager->PerfCurve(1)->outputLimits.maxPresent);
    EXPECT_EQ(1.04, state->dataCurveManager->PerfCurve(1)->outputLimits.max);
}

TEST_F(EnergyPlusFixture, DivisorNormalizationNone)
{
    //    /*
    //     *  Test: Normalization Method = None
    //     *
    //     *  The Table:Lookup object constructed in this test corresponds directly to the function:
    //     *      f(x_1, x_2) = x_1 * x_2
    //     *
    //     *  The curve of this data is therefore Linear, making interpolated data points easily calculated
    //     *  This idf will default to Cubic interpolation and Linear extrapolation
    //     */

    double expected_curve_min{2.0};
    double expected_curve_max{21.0};

    std::vector<std::pair<double, double>> table_data{
        {2.0, 1.0}, // 2.0
        {2.0, 2.0}, // 4.0
        {2.0, 3.0}, // 6.0
        {7.0, 1.0}, // 7.0
        {7.0, 2.0}, // 14.0
        {7.0, 3.0}, // 21.0
        {3.0, 3.0}, // 9.0
        {5.0, 2.0}, // 10.0
    };

    std::string const idf_objects = delimited_string({
        "Table:Lookup,",
        "y_values,                              !- Name",
        "y_values_list,                         !- Independent Variable List Name",
        ",                                      !- Normalization Method",
        ",                                      !- Normalization Divisor",
        "2.0,                                   !- Minimum Output",
        "21.0,                                  !- Maximum Output",
        "Dimensionless,                         !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        "2.0,                                   !- Value 1",
        "4.0,                                   !- Value 2",
        "6.0,                                   !- Value 3",
        "7.0,                                   !- Value 4",
        "14.0,                                  !- Value 5",
        "21.0;                                  !- Value 6",

        "Table:IndependentVariableList,",
        "y_values_list,                         !- Name",
        "x_values_1,                            !- Independent Variable Name 1",
        "x_values_2;                            !- Independent Variable Name 2",

        "Table:IndependentVariable,",
        "x_values_1,                            !- Name",
        ",                                      !- Interpolation Method",
        ",                                      !- Extrapolation Method",
        ",                                      !- Minimum value",
        ",                                      !- Maximum value",
        ",                                      !- Normalization Reference Value",
        "Dimensionless                          !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        ",",
        "2.0,                                   !- Value 1",
        "7.0;                                   !- Value 1",

        "Table:IndependentVariable,",
        "x_values_2,                            !- Name",
        "Linear,                                !- Interpolation Method",
        "Linear,                                !- Extrapolation Method",
        ",                                      !- Minimum value",
        ",                                      !- Maximum value",
        ",                                      !- Normalization Reference Value",
        "Dimensionless                          !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        ",",
        "1.0,                                   !- Value 1",
        "2.0,                                   !- Value 2",
        "3.0;                                   !- Value 3",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_EQ(0, state->dataCurveManager->NumCurves);

    Curve::GetCurveInput(*state);
    state->dataCurveManager->GetCurvesInputFlag = false;
    ASSERT_EQ(1, state->dataCurveManager->NumCurves);

    EXPECT_TRUE(state->dataCurveManager->PerfCurve(1)->outputLimits.minPresent);
    EXPECT_EQ(expected_curve_min, state->dataCurveManager->PerfCurve(1)->outputLimits.min);

    EXPECT_TRUE(state->dataCurveManager->PerfCurve(1)->outputLimits.maxPresent);
    EXPECT_EQ(expected_curve_max, state->dataCurveManager->PerfCurve(1)->outputLimits.max);

    for (auto data_point : table_data) {
        EXPECT_DOUBLE_EQ(data_point.first * data_point.second, Curve::CurveValue(*state, 1, data_point.first, data_point.second));
    }
}

TEST_F(EnergyPlusFixture, DivisorNormalizationDivisorOnly)
{
    //    /*
    //     *  Test: Normalization Method = DivisorOnly
    //     *
    //     *  The Table:Lookup object constructed in this test corresponds directly to the function:
    //     *      f(x_1, x_2) = x_1 * x_2
    //     *
    //     *  The curve of this data is therefore Linear, making interpolated data points easily calculated
    //     *  This idf will default to Cubic interpolation and Linear extrapolation
    //     */

    double expected_divisor{3.0};
    double expected_curve_min{2.0 / expected_divisor};
    double expected_curve_max{21.0 / expected_divisor};

    std::vector<std::pair<double, double>> table_data{
        {2.0, 1.0}, // 2.0
        {2.0, 2.0}, // 4.0
        {2.0, 3.0}, // 6.0
        {7.0, 1.0}, // 7.0
        {7.0, 2.0}, // 14.0
        {7.0, 3.0}, // 21.0
        {3.0, 3.0}, // 9.0
        {5.0, 2.0}, // 10.0
    };

    std::string const idf_objects = delimited_string({
        "Table:Lookup,",
        "y_values,                              !- Name",
        "y_values_list,                         !- Independent Variable List Name",
        "DivisorOnly,                           !- Normalization Method",
        "3.0,                                   !- Normalization Divisor",
        "2.0,                                   !- Minimum Output",
        "21.0,                                  !- Maximum Output",
        "Dimensionless,                         !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        "2.0,                                   !- Value 1",
        "4.0,                                   !- Value 2",
        "6.0,                                   !- Value 3",
        "7.0,                                   !- Value 4",
        "14.0,                                  !- Value 5",
        "21.0;                                  !- Value 6",

        "Table:IndependentVariableList,",
        "y_values_list,                         !- Name",
        "x_values_1,                            !- Independent Variable Name 1",
        "x_values_2;                            !- Independent Variable Name 2",

        "Table:IndependentVariable,",
        "x_values_1,                            !- Name",
        ",                                      !- Interpolation Method",
        ",                                      !- Extrapolation Method",
        ",                                      !- Minimum value",
        ",                                      !- Maximum value",
        ",                                      !- Normalization Reference Value",
        "Dimensionless                          !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        ",",
        "2.0,                                   !- Value 1",
        "7.0;                                   !- Value 1",

        "Table:IndependentVariable,",
        "x_values_2,                            !- Name",
        "Linear,                                !- Interpolation Method",
        "Linear,                                !- Extrapolation Method",
        ",                                      !- Minimum value",
        ",                                      !- Maximum value",
        ",                                      !- Normalization Reference Value",
        "Dimensionless                          !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        ",",
        "1.0,                                   !- Value 1",
        "2.0,                                   !- Value 2",
        "3.0;                                   !- Value 3",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_EQ(0, state->dataCurveManager->NumCurves);

    Curve::GetCurveInput(*state);
    state->dataCurveManager->GetCurvesInputFlag = false;
    ASSERT_EQ(1, state->dataCurveManager->NumCurves);

    EXPECT_TRUE(state->dataCurveManager->PerfCurve(1)->outputLimits.minPresent);
    EXPECT_EQ(expected_curve_min, state->dataCurveManager->PerfCurve(1)->outputLimits.min);

    EXPECT_TRUE(state->dataCurveManager->PerfCurve(1)->outputLimits.maxPresent);
    EXPECT_EQ(expected_curve_max, state->dataCurveManager->PerfCurve(1)->outputLimits.max);

    for (auto data_point : table_data) {
        EXPECT_DOUBLE_EQ(data_point.first * data_point.second / expected_divisor, Curve::CurveValue(*state, 1, data_point.first, data_point.second));
    }
}

TEST_F(EnergyPlusFixture, DivisorNormalizationDivisorOnlyButItIsZero)
{
    std::string const idf_objects = delimited_string({"Table:Lookup,",
                                                      "y_values,                              !- Name",
                                                      "y_values_list,                         !- Independent Variable List Name",
                                                      "DivisorOnly,                           !- Normalization Method",
                                                      "0.0,                                   !- Normalization Divisor",
                                                      "2.0,                                   !- Minimum Output",
                                                      "21.0,                                  !- Maximum Output",
                                                      "Dimensionless,                         !- Output Unit Type",
                                                      ",                                      !- External File Name",
                                                      ",                                      !- External File Column Number",
                                                      ",                                      !- External File Starting Row Number",
                                                      "2.0,                                   !- Value 1",
                                                      "4.0;                                   !- Value 2",

                                                      "Table:IndependentVariableList,",
                                                      "y_values_list,                         !- Name",
                                                      "x_values_1;                            !- Independent Variable Name 1",

                                                      "Table:IndependentVariable,",
                                                      "x_values_1,                            !- Name",
                                                      ",                                      !- Interpolation Method",
                                                      ",                                      !- Extrapolation Method",
                                                      ",                                      !- Minimum value",
                                                      ",                                      !- Maximum value",
                                                      ",                                      !- Normalization Reference Value",
                                                      "Dimensionless                          !- Output Unit Type",
                                                      ",                                      !- External File Name",
                                                      ",                                      !- External File Column Number",
                                                      ",                                      !- External File Starting Row Number",
                                                      ",",
                                                      "2.0,                                   !- Value 1",
                                                      "3.0;                                   !- Value 3"});

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_EQ(0, state->dataCurveManager->NumCurves);

    EXPECT_THROW(Curve::GetCurveInput(*state), std::runtime_error);
    EXPECT_TRUE(this->compare_err_stream_substring("Normalization divisor entered as zero"));
}

TEST_F(EnergyPlusFixture, DivisorNormalizationAutomaticWithDivisor)
{
    //    /*
    //     *  Test: Normalization Method = AutomaticWithDivisor
    //     *      Case: Default 'Normalization Divisor' Field
    //     *
    //     *  The Table:Lookup object constructed in this test corresponds directly to the function:
    //     *      f(x_1, x_2) = x_1 * x_2
    //     *
    //     *  The curve of this data is therefore Linear, making interpolated data points easily calculated
    //     *  This idf will default to Cubic interpolation and Linear extrapolation
    //     */

    double expected_auto_divisor{6.0};
    double expected_curve_max{21.0 / expected_auto_divisor};
    double expected_curve_min{2.0 / expected_auto_divisor};

    std::vector<std::pair<double, double>> table_data{
        {2.0, 1.0}, // 2.0
        {2.0, 2.0}, // 4.0
        {2.0, 3.0}, // 6.0
        {7.0, 1.0}, // 7.0
        {7.0, 2.0}, // 14.0
        {7.0, 3.0}, // 21.0
        {3.0, 3.0}, // 9.0
        {5.0, 2.0}, // 10.0
    };

    std::string const idf_objects = delimited_string({
        "Table:Lookup,",
        "y_values,                              !- Name",
        "y_values_list,                         !- Independent Variable List Name",
        "AutomaticWithDivisor,                  !- Normalization Method",
        ",                                      !- Normalization Divisor",
        "2.0,                                   !- Minimum Output",
        "21.0,                                  !- Maximum Output",
        "Dimensionless,                         !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        "2.0,                                   !- Value 1",
        "4.0,                                   !- Value 2",
        "6.0,                                   !- Value 3",
        "7.0,                                   !- Value 4",
        "14.0,                                  !- Value 5",
        "21.0;                                  !- Value 6",

        "Table:IndependentVariableList,",
        "y_values_list,                         !- Name",
        "x_values_1,                            !- Independent Variable Name 1",
        "x_values_2;                            !- Independent Variable Name 2",

        "Table:IndependentVariable,",
        "x_values_1,                            !- Name",
        ",                                      !- Interpolation Method",
        ",                                      !- Extrapolation Method",
        ",                                      !- Minimum value",
        ",                                      !- Maximum value",
        "3.0,                                   !- Normalization Reference Value",
        "Dimensionless                          !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        ",",
        "2.0,                                   !- Value 1",
        "7.0;                                   !- Value 1",

        "Table:IndependentVariable,",
        "x_values_2,                            !- Name",
        ",                                      !- Interpolation Method",
        ",                                      !- Extrapolation Method",
        ",                                      !- Minimum value",
        ",                                      !- Maximum value",
        "2.0,                                   !- Normalization Reference Value",
        "Dimensionless                          !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        ",",
        "1.0,                                   !- Value 1",
        "2.0,                                   !- Value 2",
        "3.0;                                   !- Value 3",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_EQ(0, state->dataCurveManager->NumCurves);

    Curve::GetCurveInput(*state);
    state->dataCurveManager->GetCurvesInputFlag = false;
    ASSERT_EQ(1, state->dataCurveManager->NumCurves);

    EXPECT_TRUE(state->dataCurveManager->PerfCurve(1)->outputLimits.minPresent);
    EXPECT_EQ(expected_curve_min, state->dataCurveManager->PerfCurve(1)->outputLimits.min);

    EXPECT_TRUE(state->dataCurveManager->PerfCurve(1)->outputLimits.maxPresent);
    EXPECT_EQ(expected_curve_max, state->dataCurveManager->PerfCurve(1)->outputLimits.max);

    for (auto data_point : table_data) {
        EXPECT_DOUBLE_EQ(data_point.first * data_point.second / expected_auto_divisor,
                         Curve::CurveValue(*state, 1, data_point.first, data_point.second));
    }
}

TEST_F(EnergyPlusFixture, NormalizationAutomaticWithDivisorAndSpecifiedDivisor)
{
    //    /*
    //     *  Test: Normalization Method = AutomaticWithDivisor
    //     *      Case: Additionally Specified Divisor in 'Normalization Divisor' Field
    //     *
    //     *  The Table:Lookup object constructed in this test corresponds directly to the function:
    //     *      f(x_1, x_2) = x_1 * x_2
    //     *
    //     *  The curve of this data is therefore Linear, making interpolated data points easily calculated
    //     *  This idf will default to Cubic interpolation and Linear extrapolation
    //     */

    double expected_auto_divisor{6.0};
    double normalization_divisor{4.0};
    double expected_curve_max{21.0 / expected_auto_divisor / normalization_divisor};
    double expected_curve_min{2.0 / expected_auto_divisor / normalization_divisor};

    std::vector<std::pair<double, double>> table_data{
        {2.0, 1.0}, // 2.0
        {2.0, 2.0}, // 4.0
        {2.0, 3.0}, // 6.0
        {7.0, 1.0}, // 7.0
        {7.0, 2.0}, // 14.0
        {7.0, 3.0}, // 21.0
        {3.0, 3.0}, // 9.0
        {5.0, 2.0}, // 10.0
    };

    std::string const idf_objects = delimited_string({
        "Table:Lookup,",
        "y_values,                              !- Name",
        "y_values_list,                         !- Independent Variable List Name",
        "AutomaticWithDivisor,                  !- Normalization Method",
        "4.0,                                   !- Normalization Divisor",
        "2.0,                                   !- Minimum Output",
        "21.0,                                  !- Maximum Output",
        "Dimensionless,                         !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        "2.0,                                   !- Value 1",
        "4.0,                                   !- Value 2",
        "6.0,                                   !- Value 3",
        "7.0,                                   !- Value 4",
        "14.0,                                  !- Value 5",
        "21.0;                                  !- Value 6",

        "Table:IndependentVariableList,",
        "y_values_list,                         !- Name",
        "x_values_1,                            !- Independent Variable Name 1",
        "x_values_2;                            !- Independent Variable Name 2",

        "Table:IndependentVariable,",
        "x_values_1,                            !- Name",
        ",                                      !- Interpolation Method",
        ",                                      !- Extrapolation Method",
        ",                                      !- Minimum value",
        ",                                      !- Maximum value",
        "3.0,                                   !- Normalization Reference Value",
        "Dimensionless                          !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        ",",
        "2.0,                                   !- Value 1",
        "7.0;                                   !- Value 1",

        "Table:IndependentVariable,",
        "x_values_2,                            !- Name",
        ",                                      !- Interpolation Method",
        ",                                      !- Extrapolation Method",
        ",                                      !- Minimum value",
        ",                                      !- Maximum value",
        "2.0,                                   !- Normalization Reference Value",
        "Dimensionless                          !- Output Unit Type",
        ",                                      !- External File Name",
        ",                                      !- External File Column Number",
        ",                                      !- External File Starting Row Number",
        ",",
        "1.0,                                   !- Value 1",
        "2.0,                                   !- Value 2",
        "3.0;                                   !- Value 3",
    });

    ASSERT_TRUE(process_idf(idf_objects));
    EXPECT_EQ(0, state->dataCurveManager->NumCurves);

    Curve::GetCurveInput(*state);
    state->dataCurveManager->GetCurvesInputFlag = false;
    ASSERT_EQ(1, state->dataCurveManager->NumCurves);

    EXPECT_TRUE(state->dataCurveManager->PerfCurve(1)->outputLimits.minPresent);
    EXPECT_EQ(expected_curve_min, state->dataCurveManager->PerfCurve(1)->outputLimits.min);

    EXPECT_TRUE(state->dataCurveManager->PerfCurve(1)->outputLimits.maxPresent);
    EXPECT_EQ(expected_curve_max, state->dataCurveManager->PerfCurve(1)->outputLimits.max);

    for (auto data_point : table_data) {
        EXPECT_DOUBLE_EQ(data_point.first * data_point.second / expected_auto_divisor / normalization_divisor,
                         Curve::CurveValue(*state, 1, data_point.first, data_point.second));
    }
}

TEST_F(EnergyPlusFixture, CSV_CarriageReturns_Handling)
{
    Curve::TableFile testTableFile = Curve::TableFile();
    fs::path testCSV = configured_source_directory() / "tst/EnergyPlus/unit/Resources/TestCarriageReturn.csv";
    testTableFile.filePath = testCSV;
    testTableFile.load(*state, testCSV);
    std::vector<double> TestArray;
    std::size_t col = 2;
    std::size_t row = 1;
    std::size_t expected_length = 168;
    TestArray = testTableFile.getArray(*state, std::make_pair(col, row));
    EXPECT_EQ(TestArray.size(), expected_length);

    for (std::size_t i = 0; i < TestArray.size(); i++) {
        EXPECT_FALSE(std::isnan(TestArray[i]));
    }
}

TEST_F(EnergyPlusFixture, QuadraticCurve_CheckCurveMinMaxValues)
{
    std::string const idf_objects = delimited_string({
        "Curve:Quadratic,",
        "  DummyEIRfPLR,                       !- Name",
        "  1,                                  !- Coefficient1 Constant",
        "  1,                                  !- Coefficient2 x",
        "  0,                                  !- Coefficient3 x**2",
        "  0.8,                                !- Minimum Value of x {BasedOnField A2}",
        "  0.5,                                !- Maximum Value of x {BasedOnField A2}",
        "  ,                                   !- Minimum Curve Output {BasedOnField A3}",
        "  ,                                   !- Maximum Curve Output {BasedOnField A3}",
        "  ,                                   !- Input Unit Type for X",
        "  ;                                   !- Output Unit Type",
    });

    EXPECT_TRUE(process_idf(idf_objects));
    EXPECT_EQ(0, state->dataCurveManager->NumCurves);
    ASSERT_THROW(Curve::GetCurveInput(*state), std::runtime_error);
    state->dataCurveManager->GetCurvesInputFlag = false;
    ASSERT_EQ(1, state->dataCurveManager->NumCurves);

    auto const expected_error = delimited_string({
        "   ** Severe  ** GetCurveInput: For Curve:Quadratic: ",
        "   **   ~~~   ** Minimum Value of x [0.80] > Maximum Value of x [0.50]",
        "   **  Fatal  ** GetCurveInput: Errors found in getting Curve Objects.  Preceding condition(s) cause termination.",
    });
    EXPECT_TRUE(compare_err_stream_substring(expected_error));
}

class CurveManagerValidationFixture : public EnergyPlusFixture,
                                      public ::testing::WithParamInterface<std::tuple<std::string, std::string, std::string, std::string>>
{
public:
    static std::string delimited_string(std::vector<std::string> const &strings, std::string const &delimiter = "\n")
    {
        return EnergyPlusFixture::delimited_string(strings, delimiter);
    }
};

TEST_P(CurveManagerValidationFixture, CurveMinMaxValues)
{
    const auto &[object_name, tested_dim, idf_objects, expected_error] = GetParam();
    EXPECT_TRUE(process_idf(idf_objects));
    EXPECT_EQ(0, state->dataCurveManager->NumCurves);
    ASSERT_THROW(Curve::GetCurveInput(*state), std::runtime_error);
    state->dataCurveManager->GetCurvesInputFlag = false;
    ASSERT_EQ(1, state->dataCurveManager->NumCurves);
    EXPECT_TRUE(compare_err_stream_substring(expected_error));
}

INSTANTIATE_TEST_SUITE_P(CurveManager,
                         CurveManagerValidationFixture,
                         ::testing::Values(
                             // Curve:Functional:PressureDrop: 0 dimensions

                             // Curve:Linear: 1 dimensions
                             std::make_tuple("Curve:Linear",
                                             "x",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:Linear,",
                                                 "  Linear,                                 !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 x",
                                                 "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                 "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                 "  Dimensionless,                          !- Input Unit Type for X",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of x [0.80] > Maximum Value of x [0.50]"),

                             // Curve:Quadratic: 1 dimensions
                             std::make_tuple("Curve:Quadratic",
                                             "x",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:Quadratic,",
                                                 "  Quadratic,                              !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 x",
                                                 "  1,                                      !- Coefficient3 x**2",
                                                 "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                 "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                 "  Dimensionless,                          !- Input Unit Type for X",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of x [0.80] > Maximum Value of x [0.50]"),

                             // Curve:Cubic: 1 dimensions
                             std::make_tuple("Curve:Cubic",
                                             "x",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:Cubic,",
                                                 "  Cubic,                                  !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 x",
                                                 "  1,                                      !- Coefficient3 x**2",
                                                 "  1,                                      !- Coefficient4 x**3",
                                                 "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                 "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                 "  Dimensionless,                          !- Input Unit Type for X",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of x [0.80] > Maximum Value of x [0.50]"),

                             // Curve:Quartic: 1 dimensions
                             std::make_tuple("Curve:Quartic",
                                             "x",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:Quartic,",
                                                 "  Quartic,                                !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 x",
                                                 "  1,                                      !- Coefficient3 x**2",
                                                 "  1,                                      !- Coefficient4 x**3",
                                                 "  1,                                      !- Coefficient5 x**4",
                                                 "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                 "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                 "  Dimensionless,                          !- Input Unit Type for X",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of x [0.80] > Maximum Value of x [0.50]"),

                             // Curve:Exponent: 1 dimensions
                             std::make_tuple("Curve:Exponent",
                                             "x",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:Exponent,",
                                                 "  Exponent,                               !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 Constant",
                                                 "  1,                                      !- Coefficient3 Constant",
                                                 "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                 "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                 "  Dimensionless,                          !- Input Unit Type for X",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of x [0.80] > Maximum Value of x [0.50]"),

                             // Curve:ExponentialSkewNormal: 1 dimensions
                             std::make_tuple("Curve:ExponentialSkewNormal",
                                             "x",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:ExponentialSkewNormal,",
                                                 "  ExponentialSkewNormal,                  !- Name",
                                                 "  1,                                      !- Coefficient1 C1",
                                                 "  1,                                      !- Coefficient2 C2",
                                                 "  1,                                      !- Coefficient3 C3",
                                                 "  1,                                      !- Coefficient4 C4",
                                                 "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                 "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                 "  Dimensionless,                          !- Input Unit Type for x",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of x [0.80] > Maximum Value of x [0.50]"),

                             // Curve:Sigmoid: 1 dimensions
                             std::make_tuple("Curve:Sigmoid",
                                             "x",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:Sigmoid,",
                                                 "  Sigmoid,                                !- Name",
                                                 "  1,                                      !- Coefficient1 C1",
                                                 "  1,                                      !- Coefficient2 C2",
                                                 "  1,                                      !- Coefficient3 C3",
                                                 "  1,                                      !- Coefficient4 C4",
                                                 "  1,                                      !- Coefficient5 C5",
                                                 "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                 "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                 "  Dimensionless,                          !- Input Unit Type for x",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of x [0.80] > Maximum Value of x [0.50]"),

                             // Curve:RectangularHyperbola1: 1 dimensions
                             std::make_tuple("Curve:RectangularHyperbola1",
                                             "x",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:RectangularHyperbola1,",
                                                 "  RectangularHyperbola1,                  !- Name",
                                                 "  1,                                      !- Coefficient1 C1",
                                                 "  1,                                      !- Coefficient2 C2",
                                                 "  1,                                      !- Coefficient3 C3",
                                                 "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                 "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                 "  Dimensionless,                          !- Input Unit Type for x",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of x [0.80] > Maximum Value of x [0.50]"),

                             // Curve:RectangularHyperbola2: 1 dimensions
                             std::make_tuple("Curve:RectangularHyperbola2",
                                             "x",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:RectangularHyperbola2,",
                                                 "  RectangularHyperbola2,                  !- Name",
                                                 "  1,                                      !- Coefficient1 C1",
                                                 "  1,                                      !- Coefficient2 C2",
                                                 "  1,                                      !- Coefficient3 C3",
                                                 "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                 "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                 "  Dimensionless,                          !- Input Unit Type for x",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of x [0.80] > Maximum Value of x [0.50]"),

                             // Curve:ExponentialDecay: 1 dimensions
                             std::make_tuple("Curve:ExponentialDecay",
                                             "x",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:ExponentialDecay,",
                                                 "  ExponentialDecay,                       !- Name",
                                                 "  1,                                      !- Coefficient1 C1",
                                                 "  1,                                      !- Coefficient2 C2",
                                                 "  1,                                      !- Coefficient3 C3",
                                                 "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                 "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                 "  Dimensionless,                          !- Input Unit Type for x",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of x [0.80] > Maximum Value of x [0.50]"),

                             // Curve:DoubleExponentialDecay: 1 dimensions
                             std::make_tuple("Curve:DoubleExponentialDecay",
                                             "x",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:DoubleExponentialDecay,",
                                                 "  DoubleExponentialDecay,                 !- Name",
                                                 "  1,                                      !- Coefficient1 C1",
                                                 "  1,                                      !- Coefficient2 C2",
                                                 "  1,                                      !- Coefficient3 C3",
                                                 "  1,                                      !- Coefficient4 C4",
                                                 "  1,                                      !- Coefficient5 C5",
                                                 "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                 "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A3}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A3}",
                                                 "  Dimensionless,                          !- Input Unit Type for x",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of x [0.80] > Maximum Value of x [0.50]"),

                             // Curve:Bicubic: 2 dimensions
                             std::make_tuple("Curve:Bicubic",
                                             "x",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:Bicubic,",
                                                 "  Bicubic,                                !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 x",
                                                 "  1,                                      !- Coefficient3 x**2",
                                                 "  1,                                      !- Coefficient4 y",
                                                 "  1,                                      !- Coefficient5 y**2",
                                                 "  1,                                      !- Coefficient6 x*y",
                                                 "  1,                                      !- Coefficient7 x**3",
                                                 "  1,                                      !- Coefficient8 y**3",
                                                 "  1,                                      !- Coefficient9 x**2*y",
                                                 "  1,                                      !- Coefficient10 x*y**2",
                                                 "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                 "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                 "  0,                                      !- Minimum Value of y {BasedOnField A3}",
                                                 "  1,                                      !- Maximum Value of y {BasedOnField A3}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                 "  Dimensionless,                          !- Input Unit Type for X",
                                                 "  Dimensionless,                          !- Input Unit Type for Y",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of x [0.80] > Maximum Value of x [0.50]"),
                             std::make_tuple("Curve:Bicubic",
                                             "y",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:Bicubic,",
                                                 "  Bicubic,                                !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 x",
                                                 "  1,                                      !- Coefficient3 x**2",
                                                 "  1,                                      !- Coefficient4 y",
                                                 "  1,                                      !- Coefficient5 y**2",
                                                 "  1,                                      !- Coefficient6 x*y",
                                                 "  1,                                      !- Coefficient7 x**3",
                                                 "  1,                                      !- Coefficient8 y**3",
                                                 "  1,                                      !- Coefficient9 x**2*y",
                                                 "  1,                                      !- Coefficient10 x*y**2",
                                                 "  0,                                      !- Minimum Value of x {BasedOnField A2}",
                                                 "  1,                                      !- Maximum Value of x {BasedOnField A2}",
                                                 "  0.8,                                    !- Minimum Value of y {BasedOnField A3}",
                                                 "  0.5,                                    !- Maximum Value of y {BasedOnField A3}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                 "  Dimensionless,                          !- Input Unit Type for X",
                                                 "  Dimensionless,                          !- Input Unit Type for Y",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of y [0.80] > Maximum Value of y [0.50]"),

                             // Curve:Biquadratic: 2 dimensions
                             std::make_tuple("Curve:Biquadratic",
                                             "x",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:Biquadratic,",
                                                 "  Biquadratic,                            !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 x",
                                                 "  1,                                      !- Coefficient3 x**2",
                                                 "  1,                                      !- Coefficient4 y",
                                                 "  1,                                      !- Coefficient5 y**2",
                                                 "  1,                                      !- Coefficient6 x*y",
                                                 "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                 "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                 "  0,                                      !- Minimum Value of y {BasedOnField A3}",
                                                 "  1,                                      !- Maximum Value of y {BasedOnField A3}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                 "  Dimensionless,                          !- Input Unit Type for X",
                                                 "  Dimensionless,                          !- Input Unit Type for Y",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of x [0.80] > Maximum Value of x [0.50]"),
                             std::make_tuple("Curve:Biquadratic",
                                             "y",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:Biquadratic,",
                                                 "  Biquadratic,                            !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 x",
                                                 "  1,                                      !- Coefficient3 x**2",
                                                 "  1,                                      !- Coefficient4 y",
                                                 "  1,                                      !- Coefficient5 y**2",
                                                 "  1,                                      !- Coefficient6 x*y",
                                                 "  0,                                      !- Minimum Value of x {BasedOnField A2}",
                                                 "  1,                                      !- Maximum Value of x {BasedOnField A2}",
                                                 "  0.8,                                    !- Minimum Value of y {BasedOnField A3}",
                                                 "  0.5,                                    !- Maximum Value of y {BasedOnField A3}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                 "  Dimensionless,                          !- Input Unit Type for X",
                                                 "  Dimensionless,                          !- Input Unit Type for Y",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of y [0.80] > Maximum Value of y [0.50]"),

                             // Curve:QuadraticLinear: 2 dimensions
                             std::make_tuple("Curve:QuadraticLinear",
                                             "x",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:QuadraticLinear,",
                                                 "  QuadraticLinear,                        !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 x",
                                                 "  1,                                      !- Coefficient3 x**2",
                                                 "  1,                                      !- Coefficient4 y",
                                                 "  1,                                      !- Coefficient5 x*y",
                                                 "  1,                                      !- Coefficient6 x**2*y",
                                                 "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                 "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                 "  0,                                      !- Minimum Value of y {BasedOnField A3}",
                                                 "  1,                                      !- Maximum Value of y {BasedOnField A3}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                 "  Dimensionless,                          !- Input Unit Type for X",
                                                 "  Dimensionless,                          !- Input Unit Type for Y",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of x [0.80] > Maximum Value of x [0.50]"),
                             std::make_tuple("Curve:QuadraticLinear",
                                             "y",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:QuadraticLinear,",
                                                 "  QuadraticLinear,                        !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 x",
                                                 "  1,                                      !- Coefficient3 x**2",
                                                 "  1,                                      !- Coefficient4 y",
                                                 "  1,                                      !- Coefficient5 x*y",
                                                 "  1,                                      !- Coefficient6 x**2*y",
                                                 "  0,                                      !- Minimum Value of x {BasedOnField A2}",
                                                 "  1,                                      !- Maximum Value of x {BasedOnField A2}",
                                                 "  0.8,                                    !- Minimum Value of y {BasedOnField A3}",
                                                 "  0.5,                                    !- Maximum Value of y {BasedOnField A3}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                 "  Dimensionless,                          !- Input Unit Type for X",
                                                 "  Dimensionless,                          !- Input Unit Type for Y",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of y [0.80] > Maximum Value of y [0.50]"),

                             // Curve:CubicLinear: 2 dimensions
                             std::make_tuple("Curve:CubicLinear",
                                             "x",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:CubicLinear,",
                                                 "  CubicLinear,                            !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 x",
                                                 "  1,                                      !- Coefficient3 x**2",
                                                 "  1,                                      !- Coefficient4 x**3",
                                                 "  1,                                      !- Coefficient5 y",
                                                 "  1,                                      !- Coefficient6 x*y",
                                                 "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                 "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                 "  0,                                      !- Minimum Value of y {BasedOnField A3}",
                                                 "  1,                                      !- Maximum Value of y {BasedOnField A3}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                 "  Dimensionless,                          !- Input Unit Type for X",
                                                 "  Dimensionless,                          !- Input Unit Type for Y",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of x [0.80] > Maximum Value of x [0.50]"),
                             std::make_tuple("Curve:CubicLinear",
                                             "y",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:CubicLinear,",
                                                 "  CubicLinear,                            !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 x",
                                                 "  1,                                      !- Coefficient3 x**2",
                                                 "  1,                                      !- Coefficient4 x**3",
                                                 "  1,                                      !- Coefficient5 y",
                                                 "  1,                                      !- Coefficient6 x*y",
                                                 "  0,                                      !- Minimum Value of x {BasedOnField A2}",
                                                 "  1,                                      !- Maximum Value of x {BasedOnField A2}",
                                                 "  0.8,                                    !- Minimum Value of y {BasedOnField A3}",
                                                 "  0.5,                                    !- Maximum Value of y {BasedOnField A3}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                 "  Dimensionless,                          !- Input Unit Type for X",
                                                 "  Dimensionless,                          !- Input Unit Type for Y",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of y [0.80] > Maximum Value of y [0.50]"),

                             // Curve:FanPressureRise: 2 dimensions
                             std::make_tuple("Curve:FanPressureRise",
                                             "Qfan",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:FanPressureRise,",
                                                 "  FanPressureRise,                        !- Name",
                                                 "  1,                                      !- Coefficient1 C1",
                                                 "  1,                                      !- Coefficient2 C2",
                                                 "  1,                                      !- Coefficient3 C3",
                                                 "  1,                                      !- Coefficient4 C4",
                                                 "  0.8,                                    !- Minimum Value of Qfan {m3/s}",
                                                 "  0.5,                                    !- Maximum Value of Qfan {m3/s}",
                                                 "  0,                                      !- Minimum Value of Psm {Pa}",
                                                 "  1,                                      !- Maximum Value of Psm {Pa}",
                                                 "  ,                                       !- Minimum Curve Output {Pa}",
                                                 "  ;                                       !- Maximum Curve Output {Pa}",
                                             }),
                                             "Minimum Value of Qfan [0.80] > Maximum Value of Qfan [0.50]"),
                             std::make_tuple("Curve:FanPressureRise",
                                             "Psm",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:FanPressureRise,",
                                                 "  FanPressureRise,                        !- Name",
                                                 "  1,                                      !- Coefficient1 C1",
                                                 "  1,                                      !- Coefficient2 C2",
                                                 "  1,                                      !- Coefficient3 C3",
                                                 "  1,                                      !- Coefficient4 C4",
                                                 "  0,                                      !- Minimum Value of Qfan {m3/s}",
                                                 "  1,                                      !- Maximum Value of Qfan {m3/s}",
                                                 "  0.8,                                    !- Minimum Value of Psm {Pa}",
                                                 "  0.5,                                    !- Maximum Value of Psm {Pa}",
                                                 "  ,                                       !- Minimum Curve Output {Pa}",
                                                 "  ;                                       !- Maximum Curve Output {Pa}",
                                             }),
                                             "Minimum Value of Psm [0.80] > Maximum Value of Psm [0.50]"),

                             // Curve:Triquadratic: 3 dimensions
                             std::make_tuple("Curve:Triquadratic",
                                             "x",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:Triquadratic,",
                                                 "  Triquadratic,                           !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 x**2",
                                                 "  1,                                      !- Coefficient3 x",
                                                 "  1,                                      !- Coefficient4 y**2",
                                                 "  1,                                      !- Coefficient5 y",
                                                 "  1,                                      !- Coefficient6 z**2",
                                                 "  1,                                      !- Coefficient7 z",
                                                 "  1,                                      !- Coefficient8 x**2*y**2",
                                                 "  1,                                      !- Coefficient9 x*y",
                                                 "  1,                                      !- Coefficient10 x*y**2",
                                                 "  1,                                      !- Coefficient11 x**2*y",
                                                 "  1,                                      !- Coefficient12 x**2*z**2",
                                                 "  1,                                      !- Coefficient13 x*z",
                                                 "  1,                                      !- Coefficient14 x*z**2",
                                                 "  1,                                      !- Coefficient15 x**2*z",
                                                 "  1,                                      !- Coefficient16 y**2*z**2",
                                                 "  1,                                      !- Coefficient17 y*z",
                                                 "  1,                                      !- Coefficient18 y*z**2",
                                                 "  1,                                      !- Coefficient19 y**2*z",
                                                 "  1,                                      !- Coefficient20 x**2*y**2*z**2",
                                                 "  1,                                      !- Coefficient21 x**2*y**2*z",
                                                 "  1,                                      !- Coefficient22 x**2*y*z**2",
                                                 "  1,                                      !- Coefficient23 x*y**2*z**2",
                                                 "  1,                                      !- Coefficient24 x**2*y*z",
                                                 "  1,                                      !- Coefficient25 x*y**2*z",
                                                 "  1,                                      !- Coefficient26 x*y*z**2",
                                                 "  1,                                      !- Coefficient27 x*y*z",
                                                 "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                 "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                 "  0,                                      !- Minimum Value of y {BasedOnField A3}",
                                                 "  1,                                      !- Maximum Value of y {BasedOnField A3}",
                                                 "  0,                                      !- Minimum Value of z {BasedOnField A4}",
                                                 "  1,                                      !- Maximum Value of z {BasedOnField A4}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A5}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A5}",
                                                 "  Dimensionless,                          !- Input Unit Type for X",
                                                 "  Dimensionless,                          !- Input Unit Type for Y",
                                                 "  Dimensionless,                          !- Input Unit Type for Z",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of x [0.80] > Maximum Value of x [0.50]"),
                             std::make_tuple("Curve:Triquadratic",
                                             "y",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:Triquadratic,",
                                                 "  Triquadratic,                           !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 x**2",
                                                 "  1,                                      !- Coefficient3 x",
                                                 "  1,                                      !- Coefficient4 y**2",
                                                 "  1,                                      !- Coefficient5 y",
                                                 "  1,                                      !- Coefficient6 z**2",
                                                 "  1,                                      !- Coefficient7 z",
                                                 "  1,                                      !- Coefficient8 x**2*y**2",
                                                 "  1,                                      !- Coefficient9 x*y",
                                                 "  1,                                      !- Coefficient10 x*y**2",
                                                 "  1,                                      !- Coefficient11 x**2*y",
                                                 "  1,                                      !- Coefficient12 x**2*z**2",
                                                 "  1,                                      !- Coefficient13 x*z",
                                                 "  1,                                      !- Coefficient14 x*z**2",
                                                 "  1,                                      !- Coefficient15 x**2*z",
                                                 "  1,                                      !- Coefficient16 y**2*z**2",
                                                 "  1,                                      !- Coefficient17 y*z",
                                                 "  1,                                      !- Coefficient18 y*z**2",
                                                 "  1,                                      !- Coefficient19 y**2*z",
                                                 "  1,                                      !- Coefficient20 x**2*y**2*z**2",
                                                 "  1,                                      !- Coefficient21 x**2*y**2*z",
                                                 "  1,                                      !- Coefficient22 x**2*y*z**2",
                                                 "  1,                                      !- Coefficient23 x*y**2*z**2",
                                                 "  1,                                      !- Coefficient24 x**2*y*z",
                                                 "  1,                                      !- Coefficient25 x*y**2*z",
                                                 "  1,                                      !- Coefficient26 x*y*z**2",
                                                 "  1,                                      !- Coefficient27 x*y*z",
                                                 "  0,                                      !- Minimum Value of x {BasedOnField A2}",
                                                 "  1,                                      !- Maximum Value of x {BasedOnField A2}",
                                                 "  0.8,                                    !- Minimum Value of y {BasedOnField A3}",
                                                 "  0.5,                                    !- Maximum Value of y {BasedOnField A3}",
                                                 "  0,                                      !- Minimum Value of z {BasedOnField A4}",
                                                 "  1,                                      !- Maximum Value of z {BasedOnField A4}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A5}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A5}",
                                                 "  Dimensionless,                          !- Input Unit Type for X",
                                                 "  Dimensionless,                          !- Input Unit Type for Y",
                                                 "  Dimensionless,                          !- Input Unit Type for Z",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of y [0.80] > Maximum Value of y [0.50]"),
                             std::make_tuple("Curve:Triquadratic",
                                             "z",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:Triquadratic,",
                                                 "  Triquadratic,                           !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 x**2",
                                                 "  1,                                      !- Coefficient3 x",
                                                 "  1,                                      !- Coefficient4 y**2",
                                                 "  1,                                      !- Coefficient5 y",
                                                 "  1,                                      !- Coefficient6 z**2",
                                                 "  1,                                      !- Coefficient7 z",
                                                 "  1,                                      !- Coefficient8 x**2*y**2",
                                                 "  1,                                      !- Coefficient9 x*y",
                                                 "  1,                                      !- Coefficient10 x*y**2",
                                                 "  1,                                      !- Coefficient11 x**2*y",
                                                 "  1,                                      !- Coefficient12 x**2*z**2",
                                                 "  1,                                      !- Coefficient13 x*z",
                                                 "  1,                                      !- Coefficient14 x*z**2",
                                                 "  1,                                      !- Coefficient15 x**2*z",
                                                 "  1,                                      !- Coefficient16 y**2*z**2",
                                                 "  1,                                      !- Coefficient17 y*z",
                                                 "  1,                                      !- Coefficient18 y*z**2",
                                                 "  1,                                      !- Coefficient19 y**2*z",
                                                 "  1,                                      !- Coefficient20 x**2*y**2*z**2",
                                                 "  1,                                      !- Coefficient21 x**2*y**2*z",
                                                 "  1,                                      !- Coefficient22 x**2*y*z**2",
                                                 "  1,                                      !- Coefficient23 x*y**2*z**2",
                                                 "  1,                                      !- Coefficient24 x**2*y*z",
                                                 "  1,                                      !- Coefficient25 x*y**2*z",
                                                 "  1,                                      !- Coefficient26 x*y*z**2",
                                                 "  1,                                      !- Coefficient27 x*y*z",
                                                 "  0,                                      !- Minimum Value of x {BasedOnField A2}",
                                                 "  1,                                      !- Maximum Value of x {BasedOnField A2}",
                                                 "  0,                                      !- Minimum Value of y {BasedOnField A3}",
                                                 "  1,                                      !- Maximum Value of y {BasedOnField A3}",
                                                 "  0.8,                                    !- Minimum Value of z {BasedOnField A4}",
                                                 "  0.5,                                    !- Maximum Value of z {BasedOnField A4}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A5}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A5}",
                                                 "  Dimensionless,                          !- Input Unit Type for X",
                                                 "  Dimensionless,                          !- Input Unit Type for Y",
                                                 "  Dimensionless,                          !- Input Unit Type for Z",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of z [0.80] > Maximum Value of z [0.50]"),

                             // Curve:ChillerPartLoadWithLift: 3 dimensions
                             std::make_tuple("Curve:ChillerPartLoadWithLift",
                                             "x",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:ChillerPartLoadWithLift,",
                                                 "  ChillerPartLoadWithLift,                !- Name",
                                                 "  1,                                      !- Coefficient1 C1",
                                                 "  1,                                      !- Coefficient2 C2",
                                                 "  1,                                      !- Coefficient3 C3",
                                                 "  1,                                      !- Coefficient4 C4",
                                                 "  1,                                      !- Coefficient5 C5",
                                                 "  1,                                      !- Coefficient6 C6",
                                                 "  1,                                      !- Coefficient7 C7",
                                                 "  1,                                      !- Coefficient8 C8",
                                                 "  1,                                      !- Coefficient9 C9",
                                                 "  1,                                      !- Coefficient10 C10",
                                                 "  1,                                      !- Coefficient11 C11",
                                                 "  1,                                      !- Coefficient12 C12",
                                                 "  0.8,                                    !- Minimum Value of x {BasedOnField A2}",
                                                 "  0.5,                                    !- Maximum Value of x {BasedOnField A2}",
                                                 "  0,                                      !- Minimum Value of y {BasedOnField A3}",
                                                 "  1,                                      !- Maximum Value of y {BasedOnField A3}",
                                                 "  0,                                      !- Minimum Value of z {BasedOnField A4}",
                                                 "  1,                                      !- Maximum Value of z {BasedOnField A4}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A5}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A5}",
                                                 "  Dimensionless,                          !- Input Unit Type for x",
                                                 "  Dimensionless,                          !- Input Unit Type for y",
                                                 "  Dimensionless,                          !- Input Unit Type for z",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of x [0.80] > Maximum Value of x [0.50]"),
                             std::make_tuple("Curve:ChillerPartLoadWithLift",
                                             "y",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:ChillerPartLoadWithLift,",
                                                 "  ChillerPartLoadWithLift,                !- Name",
                                                 "  1,                                      !- Coefficient1 C1",
                                                 "  1,                                      !- Coefficient2 C2",
                                                 "  1,                                      !- Coefficient3 C3",
                                                 "  1,                                      !- Coefficient4 C4",
                                                 "  1,                                      !- Coefficient5 C5",
                                                 "  1,                                      !- Coefficient6 C6",
                                                 "  1,                                      !- Coefficient7 C7",
                                                 "  1,                                      !- Coefficient8 C8",
                                                 "  1,                                      !- Coefficient9 C9",
                                                 "  1,                                      !- Coefficient10 C10",
                                                 "  1,                                      !- Coefficient11 C11",
                                                 "  1,                                      !- Coefficient12 C12",
                                                 "  0,                                      !- Minimum Value of x {BasedOnField A2}",
                                                 "  1,                                      !- Maximum Value of x {BasedOnField A2}",
                                                 "  0.8,                                    !- Minimum Value of y {BasedOnField A3}",
                                                 "  0.5,                                    !- Maximum Value of y {BasedOnField A3}",
                                                 "  0,                                      !- Minimum Value of z {BasedOnField A4}",
                                                 "  1,                                      !- Maximum Value of z {BasedOnField A4}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A5}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A5}",
                                                 "  Dimensionless,                          !- Input Unit Type for x",
                                                 "  Dimensionless,                          !- Input Unit Type for y",
                                                 "  Dimensionless,                          !- Input Unit Type for z",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of y [0.80] > Maximum Value of y [0.50]"),
                             std::make_tuple("Curve:ChillerPartLoadWithLift",
                                             "z",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:ChillerPartLoadWithLift,",
                                                 "  ChillerPartLoadWithLift,                !- Name",
                                                 "  1,                                      !- Coefficient1 C1",
                                                 "  1,                                      !- Coefficient2 C2",
                                                 "  1,                                      !- Coefficient3 C3",
                                                 "  1,                                      !- Coefficient4 C4",
                                                 "  1,                                      !- Coefficient5 C5",
                                                 "  1,                                      !- Coefficient6 C6",
                                                 "  1,                                      !- Coefficient7 C7",
                                                 "  1,                                      !- Coefficient8 C8",
                                                 "  1,                                      !- Coefficient9 C9",
                                                 "  1,                                      !- Coefficient10 C10",
                                                 "  1,                                      !- Coefficient11 C11",
                                                 "  1,                                      !- Coefficient12 C12",
                                                 "  0,                                      !- Minimum Value of x {BasedOnField A2}",
                                                 "  1,                                      !- Maximum Value of x {BasedOnField A2}",
                                                 "  0,                                      !- Minimum Value of y {BasedOnField A3}",
                                                 "  1,                                      !- Maximum Value of y {BasedOnField A3}",
                                                 "  0.8,                                    !- Minimum Value of z {BasedOnField A4}",
                                                 "  0.5,                                    !- Maximum Value of z {BasedOnField A4}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A5}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A5}",
                                                 "  Dimensionless,                          !- Input Unit Type for x",
                                                 "  Dimensionless,                          !- Input Unit Type for y",
                                                 "  Dimensionless,                          !- Input Unit Type for z",
                                                 "  Dimensionless;                          !- Output Unit Type",
                                             }),
                                             "Minimum Value of z [0.80] > Maximum Value of z [0.50]"),

                             // Curve:QuadLinear: 4 dimensions
                             std::make_tuple("Curve:QuadLinear",
                                             "w",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:QuadLinear,",
                                                 "  QuadLinear,                             !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 w",
                                                 "  1,                                      !- Coefficient3 x",
                                                 "  1,                                      !- Coefficient4 y",
                                                 "  1,                                      !- Coefficient5 z",
                                                 "  0.8,                                    !- Minimum Value of w {BasedOnField A2}",
                                                 "  0.5,                                    !- Maximum Value of w {BasedOnField A2}",
                                                 "  0,                                      !- Minimum Value of x {BasedOnField A3}",
                                                 "  1,                                      !- Maximum Value of x {BasedOnField A3}",
                                                 "  0,                                      !- Minimum Value of y {BasedOnField A4}",
                                                 "  1,                                      !- Maximum Value of y {BasedOnField A4}",
                                                 "  0,                                      !- Minimum Value of z {BasedOnField A5}",
                                                 "  1,                                      !- Maximum Value of z {BasedOnField A5}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                 "  Dimensionless,                          !- Input Unit Type for w",
                                                 "  Dimensionless,                          !- Input Unit Type for x",
                                                 "  Dimensionless,                          !- Input Unit Type for y",
                                                 "  Dimensionless;                          !- Input Unit Type for z",
                                             }),
                                             "Minimum Value of w [0.80] > Maximum Value of w [0.50]"),
                             std::make_tuple("Curve:QuadLinear",
                                             "x",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:QuadLinear,",
                                                 "  QuadLinear,                             !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 w",
                                                 "  1,                                      !- Coefficient3 x",
                                                 "  1,                                      !- Coefficient4 y",
                                                 "  1,                                      !- Coefficient5 z",
                                                 "  0,                                      !- Minimum Value of w {BasedOnField A2}",
                                                 "  1,                                      !- Maximum Value of w {BasedOnField A2}",
                                                 "  0.8,                                    !- Minimum Value of x {BasedOnField A3}",
                                                 "  0.5,                                    !- Maximum Value of x {BasedOnField A3}",
                                                 "  0,                                      !- Minimum Value of y {BasedOnField A4}",
                                                 "  1,                                      !- Maximum Value of y {BasedOnField A4}",
                                                 "  0,                                      !- Minimum Value of z {BasedOnField A5}",
                                                 "  1,                                      !- Maximum Value of z {BasedOnField A5}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                 "  Dimensionless,                          !- Input Unit Type for w",
                                                 "  Dimensionless,                          !- Input Unit Type for x",
                                                 "  Dimensionless,                          !- Input Unit Type for y",
                                                 "  Dimensionless;                          !- Input Unit Type for z",
                                             }),
                                             "Minimum Value of x [0.80] > Maximum Value of x [0.50]"),
                             std::make_tuple("Curve:QuadLinear",
                                             "y",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:QuadLinear,",
                                                 "  QuadLinear,                             !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 w",
                                                 "  1,                                      !- Coefficient3 x",
                                                 "  1,                                      !- Coefficient4 y",
                                                 "  1,                                      !- Coefficient5 z",
                                                 "  0,                                      !- Minimum Value of w {BasedOnField A2}",
                                                 "  1,                                      !- Maximum Value of w {BasedOnField A2}",
                                                 "  0,                                      !- Minimum Value of x {BasedOnField A3}",
                                                 "  1,                                      !- Maximum Value of x {BasedOnField A3}",
                                                 "  0.8,                                    !- Minimum Value of y {BasedOnField A4}",
                                                 "  0.5,                                    !- Maximum Value of y {BasedOnField A4}",
                                                 "  0,                                      !- Minimum Value of z {BasedOnField A5}",
                                                 "  1,                                      !- Maximum Value of z {BasedOnField A5}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                 "  Dimensionless,                          !- Input Unit Type for w",
                                                 "  Dimensionless,                          !- Input Unit Type for x",
                                                 "  Dimensionless,                          !- Input Unit Type for y",
                                                 "  Dimensionless;                          !- Input Unit Type for z",
                                             }),
                                             "Minimum Value of y [0.80] > Maximum Value of y [0.50]"),
                             std::make_tuple("Curve:QuadLinear",
                                             "z",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:QuadLinear,",
                                                 "  QuadLinear,                             !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 w",
                                                 "  1,                                      !- Coefficient3 x",
                                                 "  1,                                      !- Coefficient4 y",
                                                 "  1,                                      !- Coefficient5 z",
                                                 "  0,                                      !- Minimum Value of w {BasedOnField A2}",
                                                 "  1,                                      !- Maximum Value of w {BasedOnField A2}",
                                                 "  0,                                      !- Minimum Value of x {BasedOnField A3}",
                                                 "  1,                                      !- Maximum Value of x {BasedOnField A3}",
                                                 "  0,                                      !- Minimum Value of y {BasedOnField A4}",
                                                 "  1,                                      !- Maximum Value of y {BasedOnField A4}",
                                                 "  0.8,                                    !- Minimum Value of z {BasedOnField A5}",
                                                 "  0.5,                                    !- Maximum Value of z {BasedOnField A5}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                 "  Dimensionless,                          !- Input Unit Type for w",
                                                 "  Dimensionless,                          !- Input Unit Type for x",
                                                 "  Dimensionless,                          !- Input Unit Type for y",
                                                 "  Dimensionless;                          !- Input Unit Type for z",
                                             }),
                                             "Minimum Value of z [0.80] > Maximum Value of z [0.50]"),

                             // Curve:QuintLinear: 5 dimensions
                             std::make_tuple("Curve:QuintLinear",
                                             "v",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:QuintLinear,",
                                                 "  QuintLinear,                            !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 v",
                                                 "  1,                                      !- Coefficient3 w",
                                                 "  1,                                      !- Coefficient4 x",
                                                 "  1,                                      !- Coefficient5 y",
                                                 "  1,                                      !- Coefficient6 z",
                                                 "  0.8,                                    !- Minimum Value of v {BasedOnField A2}",
                                                 "  0.5,                                    !- Maximum Value of v {BasedOnField A2}",
                                                 "  0,                                      !- Minimum Value of w {BasedOnField A2}",
                                                 "  1,                                      !- Maximum Value of w {BasedOnField A2}",
                                                 "  0,                                      !- Minimum Value of x {BasedOnField A3}",
                                                 "  1,                                      !- Maximum Value of x {BasedOnField A3}",
                                                 "  0,                                      !- Minimum Value of y {BasedOnField A4}",
                                                 "  1,                                      !- Maximum Value of y {BasedOnField A4}",
                                                 "  0,                                      !- Minimum Value of z {BasedOnField A5}",
                                                 "  1,                                      !- Maximum Value of z {BasedOnField A5}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                 "  Dimensionless,                          !- Input Unit Type for v",
                                                 "  Dimensionless,                          !- Input Unit Type for w",
                                                 "  Dimensionless,                          !- Input Unit Type for x",
                                                 "  Dimensionless,                          !- Input Unit Type for y",
                                                 "  Dimensionless;                          !- Input Unit Type for z",
                                             }),
                                             "Minimum Value of v [0.80] > Maximum Value of v [0.50]"),
                             std::make_tuple("Curve:QuintLinear",
                                             "w",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:QuintLinear,",
                                                 "  QuintLinear,                            !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 v",
                                                 "  1,                                      !- Coefficient3 w",
                                                 "  1,                                      !- Coefficient4 x",
                                                 "  1,                                      !- Coefficient5 y",
                                                 "  1,                                      !- Coefficient6 z",
                                                 "  0,                                      !- Minimum Value of v {BasedOnField A2}",
                                                 "  1,                                      !- Maximum Value of v {BasedOnField A2}",
                                                 "  0.8,                                    !- Minimum Value of w {BasedOnField A2}",
                                                 "  0.5,                                    !- Maximum Value of w {BasedOnField A2}",
                                                 "  0,                                      !- Minimum Value of x {BasedOnField A3}",
                                                 "  1,                                      !- Maximum Value of x {BasedOnField A3}",
                                                 "  0,                                      !- Minimum Value of y {BasedOnField A4}",
                                                 "  1,                                      !- Maximum Value of y {BasedOnField A4}",
                                                 "  0,                                      !- Minimum Value of z {BasedOnField A5}",
                                                 "  1,                                      !- Maximum Value of z {BasedOnField A5}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                 "  Dimensionless,                          !- Input Unit Type for v",
                                                 "  Dimensionless,                          !- Input Unit Type for w",
                                                 "  Dimensionless,                          !- Input Unit Type for x",
                                                 "  Dimensionless,                          !- Input Unit Type for y",
                                                 "  Dimensionless;                          !- Input Unit Type for z",
                                             }),
                                             "Minimum Value of w [0.80] > Maximum Value of w [0.50]"),
                             std::make_tuple("Curve:QuintLinear",
                                             "x",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:QuintLinear,",
                                                 "  QuintLinear,                            !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 v",
                                                 "  1,                                      !- Coefficient3 w",
                                                 "  1,                                      !- Coefficient4 x",
                                                 "  1,                                      !- Coefficient5 y",
                                                 "  1,                                      !- Coefficient6 z",
                                                 "  0,                                      !- Minimum Value of v {BasedOnField A2}",
                                                 "  1,                                      !- Maximum Value of v {BasedOnField A2}",
                                                 "  0,                                      !- Minimum Value of w {BasedOnField A2}",
                                                 "  1,                                      !- Maximum Value of w {BasedOnField A2}",
                                                 "  0.8,                                    !- Minimum Value of x {BasedOnField A3}",
                                                 "  0.5,                                    !- Maximum Value of x {BasedOnField A3}",
                                                 "  0,                                      !- Minimum Value of y {BasedOnField A4}",
                                                 "  1,                                      !- Maximum Value of y {BasedOnField A4}",
                                                 "  0,                                      !- Minimum Value of z {BasedOnField A5}",
                                                 "  1,                                      !- Maximum Value of z {BasedOnField A5}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                 "  Dimensionless,                          !- Input Unit Type for v",
                                                 "  Dimensionless,                          !- Input Unit Type for w",
                                                 "  Dimensionless,                          !- Input Unit Type for x",
                                                 "  Dimensionless,                          !- Input Unit Type for y",
                                                 "  Dimensionless;                          !- Input Unit Type for z",
                                             }),
                                             "Minimum Value of x [0.80] > Maximum Value of x [0.50]"),
                             std::make_tuple("Curve:QuintLinear",
                                             "y",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:QuintLinear,",
                                                 "  QuintLinear,                            !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 v",
                                                 "  1,                                      !- Coefficient3 w",
                                                 "  1,                                      !- Coefficient4 x",
                                                 "  1,                                      !- Coefficient5 y",
                                                 "  1,                                      !- Coefficient6 z",
                                                 "  0,                                      !- Minimum Value of v {BasedOnField A2}",
                                                 "  1,                                      !- Maximum Value of v {BasedOnField A2}",
                                                 "  0,                                      !- Minimum Value of w {BasedOnField A2}",
                                                 "  1,                                      !- Maximum Value of w {BasedOnField A2}",
                                                 "  0,                                      !- Minimum Value of x {BasedOnField A3}",
                                                 "  1,                                      !- Maximum Value of x {BasedOnField A3}",
                                                 "  0.8,                                    !- Minimum Value of y {BasedOnField A4}",
                                                 "  0.5,                                    !- Maximum Value of y {BasedOnField A4}",
                                                 "  0,                                      !- Minimum Value of z {BasedOnField A5}",
                                                 "  1,                                      !- Maximum Value of z {BasedOnField A5}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                 "  Dimensionless,                          !- Input Unit Type for v",
                                                 "  Dimensionless,                          !- Input Unit Type for w",
                                                 "  Dimensionless,                          !- Input Unit Type for x",
                                                 "  Dimensionless,                          !- Input Unit Type for y",
                                                 "  Dimensionless;                          !- Input Unit Type for z",
                                             }),
                                             "Minimum Value of y [0.80] > Maximum Value of y [0.50]"),
                             std::make_tuple("Curve:QuintLinear",
                                             "z",
                                             CurveManagerValidationFixture::delimited_string({
                                                 "Curve:QuintLinear,",
                                                 "  QuintLinear,                            !- Name",
                                                 "  1,                                      !- Coefficient1 Constant",
                                                 "  1,                                      !- Coefficient2 v",
                                                 "  1,                                      !- Coefficient3 w",
                                                 "  1,                                      !- Coefficient4 x",
                                                 "  1,                                      !- Coefficient5 y",
                                                 "  1,                                      !- Coefficient6 z",
                                                 "  0,                                      !- Minimum Value of v {BasedOnField A2}",
                                                 "  1,                                      !- Maximum Value of v {BasedOnField A2}",
                                                 "  0,                                      !- Minimum Value of w {BasedOnField A2}",
                                                 "  1,                                      !- Maximum Value of w {BasedOnField A2}",
                                                 "  0,                                      !- Minimum Value of x {BasedOnField A3}",
                                                 "  1,                                      !- Maximum Value of x {BasedOnField A3}",
                                                 "  0,                                      !- Minimum Value of y {BasedOnField A4}",
                                                 "  1,                                      !- Maximum Value of y {BasedOnField A4}",
                                                 "  0.8,                                    !- Minimum Value of z {BasedOnField A5}",
                                                 "  0.5,                                    !- Maximum Value of z {BasedOnField A5}",
                                                 "  ,                                       !- Minimum Curve Output {BasedOnField A4}",
                                                 "  ,                                       !- Maximum Curve Output {BasedOnField A4}",
                                                 "  Dimensionless,                          !- Input Unit Type for v",
                                                 "  Dimensionless,                          !- Input Unit Type for w",
                                                 "  Dimensionless,                          !- Input Unit Type for x",
                                                 "  Dimensionless,                          !- Input Unit Type for y",
                                                 "  Dimensionless;                          !- Input Unit Type for z",
                                             }),
                                             "Minimum Value of z [0.80] > Maximum Value of z [0.50]")),
                         [](const testing::TestParamInfo<CurveManagerValidationFixture::ParamType> &info) -> std::string {
                             auto object_name = std::get<0>(info.param);
                             std::replace_if(object_name.begin(), object_name.end(), [](char c) { return !std::isalnum(c); }, '_');
                             return object_name + "_" + std::get<1>(info.param);
                         });

