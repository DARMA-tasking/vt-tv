[![Build and Test Ubuntu 22.04 gcc 11 x64](https://github.com/DARMA-tasking/vt-tv/actions/workflows/build-and-test-vt-tv.yml/badge.svg)](https://github.com/DARMA-tasking/vt-tv/actions/workflows/build-and-test-vt-tv.yml)

# tv => task visualizer

The task visualizer takes as input JSON files that describe work as a
series of phases and subphases that contain tasks for each rank,
communications, and other user-defined fields (such as memory
usage). Documentation on the JSON format can be found [in vt's
documentation](https://darma-tasking.github.io/docs/html/node-lb-data.html)
and the JSON schema validator is located [in the
repository](https://github.com/DARMA-tasking/vt/blob/develop/scripts/JSON_data_files_validator.py).

The task visualizer, using this input, produces Exodus meshes to
describe the ranks and objects over time, which can be visualized
using Paraview. Additionally, the task visualizer can produce PNGs
directly using a VTK workflow to render a visualization of ranks and
tasks over phases.

![Example Output PNG](./docs/example-output-image.png)