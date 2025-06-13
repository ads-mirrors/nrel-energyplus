#!/usr/bin/env python
# EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University
# of Illinois, The Regents of the University of California, through Lawrence
# Berkeley National Laboratory (subject to receipt of any required approvals
# from the U.S. Dept. of Energy), Oak Ridge National Laboratory, managed by UT-
# Battelle, Alliance for Sustainable Energy, LLC, and other contributors. All
# rights reserved.
#
# NOTICE: This Software was developed under funding from the U.S. Department of
# Energy and the U.S. Government consequently retains certain rights. As such,
# the U.S. Government has been granted for itself and others acting on its
# behalf a paid-up, nonexclusive, irrevocable, worldwide license in the
# Software to reproduce, distribute copies to the public, prepare derivative
# works, and perform publicly and display publicly, and to permit others to do
# so.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# (1) Redistributions of source code must retain the above copyright notice,
#     this list of conditions and the following disclaimer.
#
# (2) Redistributions in binary form must reproduce the above copyright notice,
#     this list of conditions and the following disclaimer in the documentation
#     and/or other materials provided with the distribution.
#
# (3) Neither the name of the University of California, Lawrence Berkeley
#     National Laboratory, the University of Illinois, U.S. Dept. of Energy nor
#     the names of its contributors may be used to endorse or promote products
#     derived from this software without specific prior written permission.
#
# (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in
#     stand-alone form without changes from the version obtained under this
#     License, or (ii) Licensee makes a reference solely to the software
#     portion of its product, Licensee must refer to the software as
#     "EnergyPlus version X" software, where "X" is the version number Licensee
#     obtained under this License and may not use a different name for the
#     software. Except as specifically required in this Section (4), Licensee
#     shall not use in a company name, a product name, in advertising,
#     publicity, or other promotional activities any name, trade name,
#     trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or
#     confusingly similar designation, without the U.S. Department of Energy's
#     prior written consent.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
from collections import defaultdict
import csv
from datetime import datetime, UTC
import json
from shutil import copy
from pathlib import Path
import sys
from shutil import rmtree
from traceback import print_exc
from zoneinfo import ZoneInfo

from energyplus_regressions.builds.base import BuildTree
from energyplus_regressions.runtests import SuiteRunner
from energyplus_regressions.structures import TextDifferences, TestEntry, EndErrSummary


class RegressionManager:

    def __init__(self):
        self.root_index_files_no_diff = []
        self.root_index_files_diffs = []
        self.root_index_files_failed = []
        self.diffs_by_idf = defaultdict(list)
        self.diffs_by_type = defaultdict(list)
        self.summary_results = {}
        self.num_idf_inspected = 0
        # self.all_files_compared = []  TODO: need to get this from regression runner
        import energyplus_regressions
        self.threshold_file = str(Path(energyplus_regressions.__file__).parent / 'diffs' / 'math_diff.config')

    def single_file_regressions(self, baseline: Path, modified: Path) -> [TestEntry, bool]:

        idf = baseline.name
        self.num_idf_inspected += 1
        this_file_diffs = []

        entry = TestEntry(idf, "")
        b1 = BuildTree()
        b1.build_dir = baseline
        b2 = BuildTree()
        b2.build_dir = modified
        entry, message = SuiteRunner.process_diffs_for_one_case(
            entry, b1, b2,"", self.threshold_file, ci_mode=True
        )  # returns an updated entry
        self.summary_results[idf] = entry.summary_result

        has_diffs = False

        text_diff_results = {
            "Audit": entry.aud_diffs,
            "BND": entry.bnd_diffs,
            "DELightIn": entry.dl_in_diffs,
            "DELightOut": entry.dl_out_diffs,
            "DXF": entry.dxf_diffs,
            "EIO": entry.eio_diffs,
            "ERR": entry.err_diffs,
            "Readvars_Audit": entry.readvars_audit_diffs,
            "EDD": entry.edd_diffs,
            "WRL": entry.wrl_diffs,
            "SLN": entry.sln_diffs,
            "SCI": entry.sci_diffs,
            "MAP": entry.map_diffs,
            "DFS": entry.dfs_diffs,
            "SCREEN": entry.screen_diffs,
            "GLHE": entry.glhe_diffs,
            "MDD": entry.mdd_diffs,
            "MTD": entry.mtd_diffs,
            "RDD": entry.rdd_diffs,
            "SHD": entry.shd_diffs,
            "PERF_LOG": entry.perf_log_diffs,
            "IDF": entry.idf_diffs,
            "StdOut": entry.stdout_diffs,
            "StdErr": entry.stderr_diffs,
        }
        for diff_type, diffs in text_diff_results.items():
            if diffs is None:
                continue
            if diffs.diff_type != TextDifferences.EQUAL:
                has_diffs = True
                this_file_diffs.append(diff_type)
                self.diffs_by_type[diff_type].append(idf)
                self.diffs_by_idf[idf].append(diff_type)

        numeric_diff_results = {
            "ESO": entry.eso_diffs,
            "MTR": entry.mtr_diffs,
            "SSZ": entry.ssz_diffs,
            "ZSZ": entry.zsz_diffs,
            "JSON": entry.json_diffs,
        }
        for diff_type, diffs in numeric_diff_results.items():
            if diffs is None:
                continue
            if diffs.diff_type == 'Big Diffs':
                has_diffs = True
                this_file_diffs.append(f"{diff_type} Big Diffs")
                self.diffs_by_type[f"{diff_type} Big Diffs"].append(idf)
                self.diffs_by_idf[idf].append(f"{diff_type} Big Diffs")
            elif diffs.diff_type == 'Small Diffs':
                has_diffs = True
                this_file_diffs.append(f"{diff_type} Small Diffs")
                self.diffs_by_type[f"{diff_type} Small Diffs"].append(idf)
                self.diffs_by_idf[idf].append(f"{diff_type} Small Diffs")

        if entry.table_diffs:
            if entry.table_diffs.big_diff_count > 0:
                has_diffs = True
                this_file_diffs.append("Table Big Diffs")
                self.diffs_by_type["Table Big Diffs"].append(idf)
                self.diffs_by_idf[idf].append("Table Big Diffs")
            elif entry.table_diffs.small_diff_count > 0:
                has_diffs = True
                this_file_diffs.append("Table Small Diffs")
                self.diffs_by_type["Table Small Diffs"].append(idf)
                self.diffs_by_idf[idf].append("Table Small Diffs")
            if entry.table_diffs.string_diff_count > 1:  # There's always one...the time stamp
                has_diffs = True
                this_file_diffs.append("Table String Diffs")
                self.diffs_by_type["Table String Diffs"].append(idf)
                self.diffs_by_idf[idf].append("Table String Diffs")

        return entry, has_diffs

    @staticmethod
    def single_diff_html(contents: str) -> str:
        return f"""
<!doctype html>
<html>
 <head>
  <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.6.0/styles/default.min.css">
  <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.6.0/highlight.min.js"></script>
  <script src="https://cdnjs.cloudflare.com/ajax/libs/highlight.js/11.6.0/languages/diff.min.js"></script>
 </head>
 <body>
  <pre>
   <code class="diff">
    {contents}
   </code>
  </pre>
  <script>hljs.highlightAll();</script>
 </body>
</html>"""

    @staticmethod
    def regression_row_in_single_test_case_html(diff_file_name: str) -> str:
        return f"""
   <tr>
    <td>{diff_file_name}</td>
    <td><a href='{diff_file_name}' download='{diff_file_name}'>download</a></td>
    <td><a href='{diff_file_name}.html'>view</a></td>
   </tr>"""

    @staticmethod
    def single_test_case_html(contents: str) -> str:
        return f"""
<!doctype html>
<html>
 <head>
  <link rel="stylesheet" href="https://netdna.bootstrapcdn.com/bootstrap/3.1.1/css/bootstrap.min.css">
  <link rel="stylesheet" href="https://netdna.bootstrapcdn.com/bootstrap/3.1.1/css/bootstrap-theme.min.css">
  <script src="https://code.jquery.com/jquery-2.1.1.min.js"></script>
  <script src="https://code.jquery.com/jquery-migrate-1.2.1.min.js"></script>
  <script src="https://netdna.bootstrapcdn.com/bootstrap/3.1.1/js/bootstrap.min.js"></script>
 </head>
 <body>
  <table class='table table-hover'>
   <tr>
    <th>filename</th>
    <th></th>
    <th></th>
   </tr>
{contents}
  </table>
 </body>
</html>"""

    def bundle_root_index_html(self, header_info: list[str]) -> str:
        # set up header table
        header_content = ""
        for hi in header_info:
            header_content += f"""<li class="list-group-item">{hi}</li>\n"""

        # set up diff summary listings
        num_no_diff = len(self.root_index_files_no_diff)
        nds = 's' if num_no_diff == 0 or num_no_diff > 1 else ''
        no_diff_content = ""
        for nd in self.root_index_files_no_diff:
            no_diff_content += f"""<li class="list-group-item">{nd}</li>\n"""
        num_diff = len(self.root_index_files_diffs)
        ds = 's' if num_diff == 0 or num_diff > 1 else ''
        diff_content = ""
        for d in self.root_index_files_diffs:
            diff_content += f"""<a href="{d}/index.html" class="list-group-item list-group-item-action">{d}</a>\n"""
        num_failed = len(self.root_index_files_failed)
        nfs = 's' if num_failed == 0 or num_failed > 1 else ''
        failed_content = ""
        for nf in self.root_index_files_failed:
            failed_content += f"""<li class="list-group-item">{nf}</li>\n"""

        # set up diff type listing
        diff_type_keys = sorted(self.diffs_by_type.keys())
        num_diff_types = len(diff_type_keys)
        dt = 's' if num_diff_types == 0 or num_diff_types > 1 else ''
        diff_type_content = ""
        if num_diff_types > 0:
            for k in diff_type_keys:
                nice_type_key = k.lower().replace(' ', '')
                diffs_this_type = self.diffs_by_type[k]
                num_files_this_type = len(diffs_this_type)
                dtt = 's' if num_diff_types == 0 or num_diff_types > 1 else ''
                this_diff_type_list = ""
                for idf in diffs_this_type:
                    this_diff_type_list += f"""<a href="{idf}/index.html" class="list-group-item list-group-item-action">{idf}</a>\n"""
                diff_type_content += f"""
   <div class="panel-group">
    <div class="panel panel-default">
     <div class="panel-heading">
      <h4 class="panel-title">
       <a data-toggle="collapse" href="#{nice_type_key}">{k}: {num_files_this_type} File{dtt}</a>
      </h4>
     </div>
     <div id="{nice_type_key}" class="panel-collapse collapse">
      <div class="panel-body">
       <ul class="list-group">
{this_diff_type_list}
       </ul>
      </div>
     </div>
    </div>
   </div>"""

        # set up runtime results table
        run_time_rows_text = ""
        sum_base_seconds = 0
        sum_branch_seconds = 0
        sorted_idf_keys = sorted(self.summary_results.keys())
        for idf in sorted_idf_keys:
            summary = self.summary_results[idf]
            case_1_success = summary.simulation_status_case1 == EndErrSummary.STATUS_SUCCESS
            case_2_success = summary.simulation_status_case2 == EndErrSummary.STATUS_SUCCESS
            if case_1_success:
                base_time = summary.run_time_seconds_case1
            else:
                base_time = "N/A"
            if case_1_success:
                branch_time = summary.run_time_seconds_case2
            else:
                branch_time = "N/A"
            if case_1_success and case_2_success:
                sum_base_seconds += base_time
                sum_branch_seconds += branch_time

            run_time_rows_text += f"""<tr><td><a href='{idf}/index.html'>{idf}</a></td><td>{base_time}</td><td>{branch_time}</td></tr>"""
        run_time_rows_text += f"""<tr><td>Runtime Total (Successes)</td><td>{sum_base_seconds:.1f}</td><td>{sum_branch_seconds:.1f}</td></tr>"""

        return f"""
<!doctype html>
<html>
 <head>
  <link rel="stylesheet" href="https://netdna.bootstrapcdn.com/bootstrap/3.1.1/css/bootstrap.min.css">
  <link rel="stylesheet" href="https://netdna.bootstrapcdn.com/bootstrap/3.1.1/css/bootstrap-theme.min.css">
  <script src="https://code.jquery.com/jquery-2.1.1.min.js"></script>
  <script src="https://code.jquery.com/jquery-migrate-1.2.1.min.js"></script>
  <script src="https://netdna.bootstrapcdn.com/bootstrap/3.1.1/js/bootstrap.min.js"></script>
 </head>
 <body>
  <div class="container-fluid">
 
   <h1>EnergyPlus Regressions</h1>
  
   <div class="panel-group" id="accordion_header">
    <div class="panel panel-default">
     <div class="panel-heading">
      <h4 class="panel-title">
       <a data-toggle="collapse" data-parent="#accordion_header" href="#header">Header Information</a>
      </h4>
     </div>
     <div id="header" class="panel-collapse collapse">
      <div class="panel-body">
       <ul class="list-group">
{header_content}
       </ul>
      </div>
     </div>
    </div>
   </div>

   <hr>
  
   <h2>Summary by File</h1>
   
   <div class="panel-group">
    <div class="panel panel-default">
     <div class="panel-heading">
      <h4 class="panel-title">
       <a data-toggle="collapse" href="#no_diffs">{num_no_diff} File{nds} with No Diffs</a>
      </h4>
     </div>
     <div id="no_diffs" class="panel-collapse collapse">
      <div class="panel-body">
       <ul class="list-group">
{no_diff_content}
       </ul>
      </div>
     </div>
    </div>
   </div>
  
   <div class="panel-group">
    <div class="panel panel-default">
     <div class="panel-heading">
      <h4 class="panel-title">
       <a data-toggle="collapse" href="#diffs">{num_diff} File{ds} with Diffs</a>
      </h4>
     </div>
     <div id="diffs" class="panel-collapse collapse">
      <div class="panel-body">
       <ul class="list-group">
{diff_content}
       </ul>
      </div>
     </div>
    </div>
   </div>
 
 
   <div class="panel-group">
    <div class="panel panel-default">
     <div class="panel-heading">
      <h4 class="panel-title">
       <a data-toggle="collapse" href="#failed">{num_failed} File{nfs} Failed During Regression Processing</a>
      </h4>
     </div>
     <div id="failed" class="panel-collapse collapse">
      <div class="panel-body">
       <ul class="list-group">
{failed_content}
       </ul>
      </div>
     </div>
    </div>
   </div>
 
   <hr>
  
   <h2>Summary by Diff Type</h1>
   
   <div class="panel-group">
    <div class="panel panel-default">
     <div class="panel-heading">
      <h4 class="panel-title">
       <a data-toggle="collapse" href="#by_diff_type">{num_diff_types} Diff Type{dt} Encountered</a>
      </h4>
     </div>
     <div id="by_diff_type" class="panel-collapse collapse">
      <div class="panel-body">
       <ul class="list-group">
{diff_type_content}
       </ul>
      </div>
     </div>
    </div>
   </div>
  
   <hr>
  
   <h2>Run Times</h2>
  
   <div class="panel-group">
    <div class="panel panel-default">
     <div class="panel-heading">
      <h4 class="panel-title">
       <a data-toggle="collapse" href="#run_times">Runtime Results Table</a>
      </h4>
     </div>
     <div id="run_times" class="panel-collapse collapse">
      <div class="panel-body">
       <table class='table table-hover'>
        <tr>
         <th>Filename</th>
         <th>Base Case Runtime (seconds)</th>
         <th>Branch Case Runtime (seconds)</th>
        </tr>
{run_time_rows_text}
       </table>
      </div>
     </div>
    </div>
   </div>
   
  </div>
 </body>
</html>
"""

    def generate_markdown_summary(self, bundle_root: Path):
        diff_lines = ""
        for diff_type, idfs in self.diffs_by_type.items():
            diff_lines += f"  - {diff_type}: {len(idfs)}\n"
        content = f"""
<details>
  <summary>Regression Summary</summary>

{diff_lines}
</details>"""
        (bundle_root / 'summary.md').write_text(content)

    @staticmethod
    def read_csv_to_columns(file_path: Path) -> dict[str, list[str | float]]:
        columns = {}
        with open(file_path, 'r', newline='') as file:
            reader = csv.reader(file)
            header = next(reader)  # Read header row
            for i, col_name in enumerate(header):
                columns[col_name.strip()] = []
            for row in reader:
                for i, value in enumerate(row):
                    value = value.strip()
                    variable = header[i].strip()
                    try:
                        v = float(value)
                        columns[variable].append(v)
                    except ValueError:  # just take the string timestamp
                        columns[variable].append(value)
        return columns

    @staticmethod
    def generate_regression_plotter(bundle_root: Path, metadata_object: dict, results_object: dict, limit: bool):
        metadata_string = json.dumps(metadata_object, indent=4)
        results_string = json.dumps(results_object, indent=4)
        diagnostics_string = ""
        limit_string = "" if not limit else r"""<div id="size-alert" class="alert alert-warning" role="alert">The regressions hit a size limit and were truncated, not all files may be represented here. Double check the full regression package.</div>"""
        content = r"""
<!DOCTYPE html>
<html lang="en">
<head>
    <title>EnergyPlus Regression Viewer</title>
    <script src="https://cdn.plot.ly/plotly-3.0.1.min.js"></script>
    <link href="https://cdn.jsdelivr.net/npm/bootstrap@5.3.3/dist/css/bootstrap.min.css" rel="stylesheet">
    <link rel="icon" type="image/png"
          href="data:image/png;base64,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">
</head>
<body class="p-4">
<div class="container">
    <h2 class="mb-4">EnergyPlus Regression Quick Plotter</h2>
    <div class="mb-3">
        <label for="model-select" class="form-label">Select a Building Model</label>
        <select id="model-select" class="form-select"></select>
    </div>
    <div class="mb-3">
        <label for="variable-select" class="form-label">Select an Output Variable</label>
        <select id="variable-select" class="form-select"></select>
    </div>
    <button id="generate-button" class="btn btn-warning btn-lg mb-4" disabled>Regenerate Plot</button>
    <div id="plot-alert" class="alert alert-info" role="alert"></div>
    {{LIMIT}}
    <div id="plot" class="container"></div>
    <div id="plot2" class="container"></div>
</div>
<script>
    const metadata = {{METADATA}};
    const results = {{RESULTS}};

    const modelSelector = document.getElementById('model-select');
    const variableSelector = document.getElementById('variable-select');
    const generateButton = document.getElementById('generate-button');
    const plot2Div = document.getElementById('plot2');
    const plotAlert = document.getElementById('plot-alert');

    function fixTimestamp(ts) {
        const [datePart, timePart] = ts.trim().split(/\s+/); // "01/21", "1:00:00"
        const [month, day] = datePart.split('/').map(Number);
        let [hour, minute, second] = timePart.split(':').map(Number);
        const currentYear = new Date().getFullYear();

        // Fix hour=24 to hour=0 and increment day
        if (hour === 24) {
            hour = 0;
            return new Date(currentYear, month - 1, day + 1, hour, minute, second); // JavaScript handles overflow
        }

        return new Date(currentYear, month - 1, day, hour, minute, second);
    }

    function handlePageInit() {
        let modelCounter = 0;
        modelSelector.innerHTML = '';
        for (const model in results) {
            const opt = document.createElement('option');
            opt.value = model;
            opt.innerText = model;
            modelSelector.appendChild(opt);
            if (modelCounter === 0) {
                opt.selected = true;
                handleModelChange();
            }
            modelCounter++;
        }
        plot();
    }

    function setGenerateButtonStatus(enabled) {
        generateButton.disabled = !enabled;
        if (enabled) {
            generateButton.classList.add('btn-warning');
        } else {
            generateButton.classList.remove('btn-warning');
        }
    }

    function handleModelChange() {
        // Handles the user changing which building model is selected
        // Primary jobs are to refresh the variable selector and set the currentModel variable
        if (modelSelector.value === "") return;
        let variableCounter = 0;
        variableSelector.innerHTML = '';
        for (const variable in results[modelSelector.value]) {
            if (variable === "timestamps") continue; // Skip timestamps
            const opt = document.createElement('option');
            opt.value = variable;
            opt.innerText = variable;
            variableSelector.appendChild(opt);
            if (variableCounter === 0) {
                opt.selected = true;
            }
            variableCounter++;
        }
        setGenerateButtonStatus(true);
    }

    function handleVariableChange() {
        // Handles the user changing which output variable is selected
        // Primary job is to simply set the generate button to signal the user needs to regenerate the plot
        setGenerateButtonStatus(true);
    }

    function plot() {
        // Handles the user changing which output variable is selected
        // Primary job is to simply re-plot the graph with the new variable
        const model = modelSelector.value;
        const variable = variableSelector.value;
        const dev = results[model][variable].develop[0];
        const feat = results[model][variable].branch[0];
        const timestampsBaseline = results[model].timestamps.develop[0];
        const timestampsBranch = results[model].timestamps.branch[0];
        const xTimesBaseline = timestampsBaseline.map(fixTimestamp);
        const xTimesBranch = timestampsBranch.map(fixTimestamp);
        const layout = {
            title: {
                text: `${model} - ${variable}`,
                subtitle: {text: `${metadata.baseline} vs ${metadata.branch}@${metadata.branchSha}`}
            },
            autosize: true,
            yaxis: {automargin: true, rangemode: 'tozero'},
        };
        const config = {
            responsive: true,
        };
        Plotly.newPlot(
            'plot',
            [
                {x: xTimesBaseline, y: dev, name: `${metadata.baseline}`, mode: 'lines+markers', type: 'scatter'},
                {x: xTimesBranch, y: feat, name: `${metadata.branch}`, mode: 'lines+markers', type: 'scatter'}
            ],
            layout,
            config
        );
        if (results[model][variable].develop.length === 1) {
            plotAlert.innerText = "This run contained n != 2 design days, or timestamps changed between runs, so only one plot is shown.";
            plot2Div.hidden = true;
        } else if (results[model][variable].develop.length === 2) {
            plotAlert.innerText = "Two plots are shown, one for each design day";
            plot2Div.hidden = false;
            const dev2 = results[model][variable].develop[1];
            const feat2 = results[model][variable].branch[1];
            const timestampsBaseline2 = results[model].timestamps.develop[1];
            const timestampsBranch2 = results[model].timestamps.branch[1];
            const xTimesBaseline2 = timestampsBaseline2.map(fixTimestamp);
            const xTimesBranch2 = timestampsBranch2.map(fixTimestamp);
            Plotly.newPlot(
                'plot2',
                [
                    {x: xTimesBaseline2, y: dev2, name: `${metadata.baseline}`, mode: 'lines+markers', type: 'scatter'},
                    {x: xTimesBranch2, y: feat2, name: `${metadata.branch}`, mode: 'lines+markers', type: 'scatter'}
                ],
                layout,
                config
            );
        }
        setGenerateButtonStatus(false);
    }

    window.onload = function() {
        handlePageInit(); // call this *before* establishing event listeners
        modelSelector.onchange = () => handleModelChange();
        variableSelector.onchange = () => handleVariableChange();
        generateButton.onclick = () => plot();
    }
</script>
</body>
</html>
        """
        patches = {
            '{{METADATA}}': metadata_string,
            '{{RESULTS}}': results_string,
            '{{LIMIT}}': limit_string
        }
        for key, value in patches.items():
            content = content.replace(key, value)
        (bundle_root / 'regression_plotter.html').write_text(content)

    def check_all_regressions(self, base_testfiles: Path, mod_testfiles: Path, bundle_root: Path) -> bool:
        any_diffs = False
        bundle_root.mkdir(exist_ok=True)
        entries = sorted(base_testfiles.iterdir())
        backtrace_shown = False

        # temporarily hardcoding metadata stuff
        baseline_name = "baseline"
        branch_name = "branch"
        branch_sha = "abc123de"
        metadata_object = {
            "baseline": baseline_name, "branch": branch_name, "branchSha": branch_sha
        }
        results_object = {}
        hit_max_limit = False
        for entry_num, baseline in enumerate(entries):
            if not baseline.is_dir():
                continue
            if baseline.name == 'CMakeFiles':  # add more ignore dirs here
                continue
            modified = mod_testfiles / baseline.name
            if not modified.exists():
                continue  # TODO: Should we warn that it is missing?
            try:
                entry, diffs = self.single_file_regressions(baseline, modified)
                if diffs:
                    self.root_index_files_diffs.append(baseline.name)
                    any_diffs = True
                    potential_diff_files = baseline.glob("*.*.*")  # TODO: Could try to get this from the regression tool
                    target_dir_for_this_file_diffs = bundle_root / baseline.name
                    if potential_diff_files:
                        if target_dir_for_this_file_diffs.exists():
                            rmtree(target_dir_for_this_file_diffs)
                        target_dir_for_this_file_diffs.mkdir()
                        index_contents_this_file = ""
                        for potential_diff_file in potential_diff_files:
                            copy(potential_diff_file, target_dir_for_this_file_diffs)
                            diff_file_with_html = target_dir_for_this_file_diffs / (potential_diff_file.name + '.html')
                            if potential_diff_file.name.endswith('.htm'):
                                # already a html file, just upload the raw contents but renamed as ...htm.html
                                copy(potential_diff_file, diff_file_with_html)
                            else:
                                # it's not an HTML file, wrap it inside an HTML wrapper in a temp file and send it
                                contents = potential_diff_file.read_text()
                                wrapped_contents = self.single_diff_html(contents)
                                diff_file_with_html.write_text(wrapped_contents)
                                if potential_diff_file.name == "eplusout.csv.absdiff.csv" and not hit_max_limit:
                                    # get data from the csvs, for now just all columns
                                    base_csv = potential_diff_file.parent / "eplusout.csv"
                                    base_column_data = self.read_csv_to_columns(base_csv)
                                    base_timestamps = base_column_data['Date/Time']
                                    mod_csv = mod_testfiles / baseline.name / "eplusout.csv"
                                    mod_column_data = self.read_csv_to_columns(mod_csv)
                                    mod_timestamps = mod_column_data['Date/Time']
                                    # but then downselect only the columns that math-diff provided in the summary
                                    abs_diff_columns = self.read_csv_to_columns(potential_diff_file)
                                    variables_with_diffs = abs_diff_columns.keys()
                                    # Alright so here's the trick.  If we can tell for sure that these results are
                                    # just two design days only, split evenly, then we will split it into two
                                    # plots.  If not, we'll just do one big one
                                    # at this point, we know the time series match between base and branch
                                    do_multi_plot = False
                                    mid_point = 0
                                    base_times = [base_timestamps]
                                    mod_times = [mod_timestamps]
                                    if len(base_timestamps) % 2 == 0:
                                        mid_point = len(base_timestamps) // 2
                                        one = base_timestamps[:mid_point]
                                        one_prefix = one[0][0:6]
                                        one_all_same_prefix = all([x.startswith(one_prefix) for x in one])
                                        two = base_timestamps[mid_point:]
                                        two_prefix = two[0][0:6]
                                        two_all_same_prefix = all([x.startswith(two_prefix) for x in two])
                                        if one_all_same_prefix and two_all_same_prefix:
                                            do_multi_plot = True
                                            base_times = [one, two]
                                            mod_times = [one, two]
                                    results_object[baseline.name] = {
                                        "timestamps": {"develop": base_times, "branch": mod_times}
                                    }
                                    for v in variables_with_diffs:
                                        if v == 'Date/Time':
                                            continue
                                        base_data = [base_column_data[v]]
                                        mod_data = [mod_column_data[v]]
                                        if do_multi_plot:
                                            plot_one_base_data = base_column_data[v][:mid_point]
                                            plot_one_mod_data = mod_column_data[v][:mid_point]
                                            plot_two_base_data = base_column_data[v][mid_point:]
                                            plot_two_mod_data = mod_column_data[v][mid_point:]
                                            base_data = [plot_one_base_data, plot_two_base_data]
                                            mod_data = [plot_one_mod_data, plot_two_mod_data]
                                        # temporarily hardcode the units and quantity
                                        results_object[baseline.name][v] = {
                                            "develop": base_data,
                                            "branch": mod_data
                                        }
                                    # check if we are at the max limit to avoid making the page way too heavy to load
                                    max_size_megabytes = 75  # 75 MB is probably a good cutoff
                                    max_size_bytes = max_size_megabytes * 1024 * 1024
                                    estimated_size = len(json.dumps(results_object).encode('utf-8'))
                                    if estimated_size > max_size_bytes:
                                        hit_max_limit = True
                            index_contents_this_file += self.regression_row_in_single_test_case_html(
                                potential_diff_file.name
                            )
                        index_file = target_dir_for_this_file_diffs / 'index.html'
                        index_this_file = self.single_test_case_html(index_contents_this_file)
                        index_file.write_text(index_this_file)
                else:
                    self.root_index_files_no_diff.append(baseline.name)
                so_far = ' Diffs! ' if any_diffs else 'No diffs'
                if entry_num % 40 == 0:
                    print(f"On file #{entry_num}/{len(entries)} ({baseline.name}), Diff status so far: {so_far}")
            except Exception as e:
                any_diffs = True
                print(f"Regression run *failed* trying to process file: {baseline.name}; reason: {e}")
                if not backtrace_shown:
                    print("Traceback shown once:")
                    print_exc()
                    backtrace_shown = True
                self.root_index_files_failed.append(baseline.name)
        meta_data = [
            f"Regression time stamp in UTC: {datetime.now(UTC)}",
            f"Regression time stamp in Central Time: {datetime.now(ZoneInfo('America/Chicago'))}",
            f"Number of input files evaluated: {self.num_idf_inspected}",
        ]
        bundle_root_index_file_path = bundle_root / 'index.html'
        bundle_root_index_content = self.bundle_root_index_html(meta_data)
        bundle_root_index_file_path.write_text(bundle_root_index_content)
        print()
        print(f"* Files with Diffs *:\n{"\n ".join(self.root_index_files_diffs)}\n")
        print(f"* Diffs by File *:\n{json.dumps(self.diffs_by_idf, indent=2, sort_keys=True)}\n")
        print(f"* Diffs by Type *:\n{json.dumps(self.diffs_by_type, indent=2, sort_keys=True)}\n")
        if any_diffs:
            self.generate_markdown_summary(bundle_root)
            self.generate_regression_plotter(
                bundle_root, metadata_object, results_object, hit_max_limit
            )
            self.generate_regression_plotter(
                bundle_root.parent, metadata_object, results_object, hit_max_limit
            )
            # print("::warning title=Regressions::Diffs Detected")
        return any_diffs


if __name__ == "__main__":  # pragma: no cover - testing function, not the __main__ entry point

    if len(sys.argv) != 4:
        print("syntax: %s base_dir mod_dir regression_dir" % sys.argv[0])
        sys.exit(1)
    arg_base_dir = Path(sys.argv[1])
    arg_mod_dir = Path(sys.argv[2])
    arg_regression_dir = Path(sys.argv[3])
    rm = RegressionManager()
    response = rm.check_all_regressions(arg_base_dir, arg_mod_dir, arg_regression_dir)
    sys.exit(1 if response else 0)
