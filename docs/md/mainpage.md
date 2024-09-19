\mainpage Introduction

\tableofcontents

\section vttv_what_is What is VT TV?

`vt-tv` provides visualizations of an application's work-to-rank mappings, communications, and memory usage.

Specifically, the task visualizer takes in JSON files that describe work as a series of phases and subphases that contain 1) tasks for each rank, 2) communications, and 3) other user-defined fields (such as memory usage).

Using this input data, the task visualizer produces Exodus meshes to describe the ranks and objects over time, which can be visualized using Paraview. Additionally, the task visualizer can produce PNGs directly using a VTK workflow to render a visualization of ranks and tasks over phases.

---

\section vttv_getting_started Getting Started

You will need the following dependencies:

1. A C++ compiler that supports C++17
2. [`cmake`](https://cmake.org/cmake/help/latest/) >= 3.17
3. [`VTK`](https://docs.vtk.org/en/latest/index.html) (build instructions [here](https://gitlab.kitware.com/vtk/vtk/-/blob/master/Documentation/docs/build_instructions/build.md))

Begin by cloning `vt-tv`:

```
git clone https://github.com/DARMA-tasking/vt-tv.git
```

_In future directions, we will assume that the `vt-tv` source is located in `${VTTV_SOURCE_DIR}`._

---

\section vttv_installation_and_usage Installation and Usage

`vt-tv` can installed as either a standalone C++ app or as a Python module.

\subsection vttv_standalone Standalone

\subsubsection vttv_standaone_build 1. Build

For the simplest build, run from `${VTTV_SOURCE_DIR}`:

```
VTK_DIR=/path/to/vtk/build ./build.sh
```

To build and run tests, add the `--tests-run` flag:

```
VTK_DIR=/path/to/vtk/build ./build.sh --tests-run
```

(More documentation for `build.sh` can be found within the script itself, including examples.)

Alternatively, for an interactive build process, run:

```
./interactive_build.sh
```

_In future directions, we will assume  that the `vt-tv` build is in `${VTTV_BUILD_DIR}`._

\subsubsection vttv_standalone_usage 2. Usage

`vt-tv` requires two inputs:

1. One or more JSON data files
2. A YAML configuration file (which contains the path to the JSON data files)

The basic call to `vt-tv` is:

```bash
${VTTV_BUILD_DIR}/apps/vt_standalone -c path/to/config
```

_Note: The_ `path/to/config` _argument should be relative to_ `${VTTV_SOURCE_DIR}` _(see example below)._

#### YAML Input

A sample YAML configuration file can be found in `${VTTV_SOURCE_DIR}/config/conf.yaml`. To use it, run

```bash
${VTTV_BUILD_DIR}/apps/vt_standalone -c config/conf.yaml
```

#### JSON Data Files

Sample JSON data files are provided in `${VTTV_SOURCE_DIR}/tests/unit/lb_test_data`.

Information regarding the JSON format can be found in vt's [documentation](https://darma-tasking.github.io/docs/html/node-lb-data.html); the JSON schema validator is located in the vt [repo](https://github.com/DARMA-tasking/vt/blob/develop/scripts/JSON_data_files_validator.py).

Additionally, DARMA-tasking's Load Balancing Analysis Framework (LBAF) provides a Python script ([lbsJSONDataFilesMaker.py](https://github.com/DARMA-tasking/LB-analysis-framework/blob/develop/src/lbaf/Utils/lbsJSONDataFilesMaker.py)) that may be used to generate JSON data files.

\subsection vttv_python_module Python Module

\subsubsection vttv_python_module_deps Dependencies

In addition to the basic `vt-tv` dependencies listed above, you will also need:

1. A Python version between 3.8 - 3.11
2. [`nanobind`](https://nanobind.readthedocs.io/en/latest/), which can be installed with:

```sh
pip install nanobind
```

\subsubsection vttv_python_module_install 1. Install

First, specify the location of your `VTK` build (see above) with:

```bash
export VTK_DIR=/path/to/vtk/build
```

Optional: To specify the number of parallel jobs to use during the build, you can set the `VT_TV_CMAKE_JOBS` environment variable:

```bash
export VT_TV_CMAKE_JOBS=8
```

Then install the binded `vt-tv` Python module with:

```bash
pip install ${VTTV_SOURCE_DIR}
```

_Note: Behind the scenes, the usual `cmake` and `make` commands are run. Depending on your system, this can cause the install process to be lengthy as it will be compiling the entire `vt-tv` library._

\subsubsection vttv_python_module_usage 2. Usage

Import the `vt-tv` module into your project using:

```python
import vttv
```

The only function you need is `vttv.tvFromJson`, which has the following (C++) function signature:

```cpp
void tvFromJson(
    const std::vector<std::string>& input_json_per_rank_list,
    const std::string& input_yaml_params_str,
    uint64_t num_ranks
)
```

The parameters are:
- `input_json_per_rank_list`: A list of the input JSON data strings (one string per rank). In the C++ standalone app, this equates to the input JSON data files.
- `input_yaml_params_str`: The visualization and output configuration data, formatted as a dictionary but exported as a string (see example below). This equates to the standalone app's input YAML configuration file.
- `num_ranks`: The number of ranks to be visualized by `vt-tv`.

As an example, here is the (emptied) code used by the [`Load Balancing Analysis Framework`](https://github.com/DARMA-tasking/LB-analysis-framework) to call `vt-tv`:

```python
import vttv

# Populate with the JSON data from each rank
ranks_json_str = []

# Populate with the desired configuration parameters
vttv_params = {
    "x_ranks": ,
    "y_ranks": ,
    "z_ranks": ,
    "object_jitter": ,
    "rank_qoi": ,
    "object_qoi": ,
    "save_meshes": ,
    "force_continuous_object_qoi": ,
    "output_visualization_dir": ,
    "output_visualization_file_stem":
}

# Populate with number of ranks used in the current problem
num_ranks =

# Call vt-tv
vttv.tvFromJson(ranks_json_str, str(vttv_params), num_ranks)
```

---

\section vttv_design Design Information

\subsection vttv_qoi 1. Quantities of Interest

`vt-tv` visualizes various Quantities of Interest (QOI) as requested by the user in the YAML configuration file:

```yaml
visualization:
    # Other parameters...
    rank_qoi:
    object_qoi:
```

While `vt-tv` natively supports a variety of QOI, such as the `load`, `id`, or `volume` of ranks and objects[^1], we also support user-defined QOI, called `attributes`.

\subsubsection vttv_rank_attributes Rank Attributes

Rank `attributes` are defined in the `metadata` field of the JSON data files. For example:

```json
{
    "metadata": {
        "rank": 0,
        "attributes": {
            "max_memory_usage": 8.0e+9
        }
    }
}
```
In this example, the user defines `max_memory_usage` as a rank attribute. This can then be specified as a `rank_qoi` in the YAML configuration file.

\subsubsection vttv_obj_attributes Object Attributes

Object `attributes` are defined in the `tasks` field of the JSON data files. For example:

```json
{
    "phases": [
        {
            "id": 0,
            "tasks": [
                {
                    "entity": {
                        "home": 0,
                        "id": 0,
                        "migratable": true,
                        "type": "object"
                    },
                    "node": 0,
                    "resource": "cpu",
                    "time": 2.0,
                    "attributes": {
                        "shared_bytes": 10000.0,
                        "shared_id": 0
                    }
                },
            ]
        }
    ]
}
```

In this case, the user has defined `shared_bytes` and `shared_id` as potential QOI.

In the YAML configuration file passed to `vt-tv`, they may specify either of these as their `object_qoi`.

\subsection vttv_structure 2. General Structure

`vt-tv` is designed according to the following hierarchy:

\dot
digraph G {
    rankdir=TD;
    Info -> ObjectInfo;
    Info -> Rank;
    Rank -> PhaseWork;
    PhaseWork -> ObjectWork;
    ObjectWork -> ObjectCommunicator;
}
\enddot

_Further information on each class, including methods and member variables, can be found in the documentation._

#### Navigating the Hierarchy

Users should interact mainly with the overarching `Info` class, which contains functions that drill down the hierarchy to get the desired information.

For example, an instance of `Info` holds getters to all object and rank QOI (including user_defined attributes):

```cpp
auto rank_qoi = info.getRankQOIAtPhase(rank_id, phase_id, qoi_string);
auto obj_qoi = info.getObjectQOIAtPhase(obj_id, phase_id, qoi_string);
```
where the `qoi_string` is the name of the desired QOI, like "load" or "id". This string can also be a user-defined attribute, as described above.

#### ObjectInfo vs. ObjectWork

There are two classes that hold object data: `ObjectInfo` and `ObjectWork`.

`ObjectInfo` holds information about a given object across all ranks and phases. This includes:
- the object's ID
- the object's home rank (where it originated)
- whether the object is migratable or sentinel (stays on the same rank)

`ObjectWork` holds information that may change as an object changes rank or phase, such as:
- the object's attributes
- the object's communications

_Tip: As discussed above, users should utilize the getters present in `Info` rather than directly calling these classes._

---

\section License

@m_class{m-note m-dim}

@parblock
Copyright 2021-2024 National Technology & Engineering Solutions of Sandia, LLC
(NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
Government retains certain rights in this software.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice,
  this list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

* Neither the name of the copyright holder nor the names of its
  contributors may be used to endorse or promote products derived from this
  software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
@endparblock