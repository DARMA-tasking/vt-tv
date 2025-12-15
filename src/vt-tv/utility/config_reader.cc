/*
//@HEADER
// *****************************************************************************
//
//                               config_reader.cc
//             DARMA/vt-tv => Virtual Transport -- Task Visualizer
//
// Copyright 2019-2024 National Technology & Engineering Solutions of Sandia, LLC
// (NTESS). Under the terms of Contract DE-NA0003525 with NTESS, the U.S.
// Government retains certain rights in this software.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// * Redistributions of source code must retain the above copyright notice,
//   this list of conditions and the following disclaimer.
//
// * Redistributions in binary form must reproduce the above copyright notice,
//   this list of conditions and the following disclaimer in the documentation
//   and/or other materials provided with the distribution.
//
// * Neither the name of the copyright holder nor the names of its
//   contributors may be used to endorse or promote products derived from this
//   software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
// AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
// LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
// SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
// INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
// CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
// ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//
// Questions? Contact darma@sandia.gov
//
// *****************************************************************************
//@HEADER
*/
#include "config_reader.h"

namespace vt::tv::utility {

ConfigReader ConfigReader::from_file(const std::string& filename) {
  YAML::Node root;
  try { root = YAML::LoadFile(filename); }
  catch (const std::exception& e) {
    throw ValidationError(std::string("Failed to load YAML: ") + e.what());
  }
  ConfigValidator::validate(root); // check structure + types only
  ConfigReader cfg = parse(root);  // parse into typed fields
  cfg.compute_grid();              // fill/derive x,y,z based on n_ranks + any hints
  return cfg;
}

ConfigReader ConfigReader::from_binding_inputs(
    const std::string& viz_yaml_fragment,
    uint64_t num_ranks)
{
  YAML::Node root_in;
  try {
    root_in = YAML::Load(viz_yaml_fragment);
  } catch (const std::exception& e) {
    throw ValidationError(std::string("Failed to parse binding YAML: ") + e.what());
  }

  YAML::Node synthetic;
  // input
  synthetic["input"]["directory"] = ""; // dummy, not used (no filesystem discovery)
  synthetic["input"]["n_ranks"]   = num_ranks;
  // no file_stem in this mode

  // viz
  if (root_in["x_ranks"])  synthetic["viz"]["x_ranks"]  = root_in["x_ranks"];
  if (root_in["y_ranks"])  synthetic["viz"]["y_ranks"]  = root_in["y_ranks"];
  if (root_in["z_ranks"])  synthetic["viz"]["z_ranks"]  = root_in["z_ranks"];
  if (root_in["object_jitter"]) synthetic["viz"]["object_jitter"] = root_in["object_jitter"];
  if (root_in["rank_qoi"])      synthetic["viz"]["rank_qoi"]      = root_in["rank_qoi"];
  if (root_in["object_qoi"])    synthetic["viz"]["object_qoi"]    = root_in["object_qoi"];
  if (root_in["save_meshes"])   synthetic["viz"]["save_meshes"]   = root_in["save_meshes"];
  // binding always renders PNGs
  synthetic["viz"]["save_pngs"] = true;
  if (root_in["force_continuous_object_qoi"]) {
    synthetic["viz"]["force_continuous_object_qoi"] =
      root_in["force_continuous_object_qoi"];
  }

  // output
  // required in binding mode:
  if (!root_in["output_visualization_dir"] ||
      !root_in["output_visualization_dir"].IsScalar()) {
    throw ValidationError("Binding config missing required 'output_visualization_dir'.");
  }
  if (!root_in["output_visualization_file_stem"] ||
      !root_in["output_visualization_file_stem"].IsScalar()) {
    throw ValidationError("Binding config missing required 'output_visualization_file_stem'.");
  }

  synthetic["output"]["directory"] =
    root_in["output_visualization_dir"].as<std::string>();
  synthetic["output"]["file_stem"] =
    root_in["output_visualization_file_stem"].as<std::string>();

  if (root_in["window_size"]) synthetic["output"]["window_size"] = root_in["window_size"];
  if (root_in["font_size"])   synthetic["output"]["font_size"]   = root_in["font_size"];

  // In binding mode we require absolute output directory
  {
    std::string outdir = synthetic["output"]["directory"].as<std::string>();
    std::filesystem::path p(outdir);
    if (!p.is_absolute()) {
      throw SemanticError("Visualization output directory must be absolute: " + outdir);
    }
    // We don't prepend SRC_DIR in this mode
  }

  // Give adapted config to regular config parser
  ConfigReader cfg = ConfigReader::parse(synthetic);
  cfg.compute_grid();
  return cfg;
}

ConfigReader ConfigReader::parse(const YAML::Node& root) {
  ConfigReader cfg;

  auto in = root["input"];
  cfg.input.directory = in["directory"].as<std::string>();
  cfg.input.n_ranks   = in["n_ranks"].as<uint64_t>();
  if (auto n = in["file_stem"]) cfg.input.file_stem = n.as<std::string>();

  if (auto vz = root["viz"]) {
    if (auto n = vz["x_ranks"])       cfg.viz.x_ranks       = n.as<uint64_t>();
    if (auto n = vz["y_ranks"])       cfg.viz.y_ranks       = n.as<uint64_t>();
    if (auto n = vz["object_jitter"]) cfg.viz.object_jitter = n.as<double>();
    if (auto n = vz["rank_qoi"])      cfg.viz.rank_qoi      = n.as<std::string>();
    if (auto n = vz["object_qoi"])    cfg.viz.object_qoi    = n.as<std::string>();
    if (auto n = vz["save_meshes"])   cfg.viz.save_meshes   = n.as<bool>();
    if (auto n = vz["save_pngs"])     cfg.viz.save_pngs     = n.as<bool>();
    if (auto n = vz["force_continuous_object_qoi"]) cfg.viz.force_continuous_object_qoi = n.as<bool>();
  }

  if (auto out = root["output"]) {
    if (auto n = out["directory"])   cfg.output.directory   = n.as<std::string>();
    if (auto n = out["file_stem"])   cfg.output.file_stem   = n.as<std::string>();
    if (auto n = out["window_size"]) cfg.output.window_size = n.as<uint64_t>();
    if (auto n = out["font_size"])   cfg.output.font_size   = n.as<uint64_t>();
  }

  return cfg;
}

void ConfigReader::compute_grid() {
  const uint64_t n = input.n_ranks;
  grid.x = viz.x_ranks.value_or(0);
  grid.y = viz.y_ranks.value_or(0);

  // Case A: none of x/y provided -> pick closest square layout
  if (!grid.x && !grid.y) {
    if (n <= 0) throw SemanticError("Number of ranks is negative or zero.");

    uint64_t k = static_cast<uint64_t>(std::ceil(std::sqrt(static_cast<double>(n)))); // closest square
    uint64_t m = static_cast<uint64_t>(std::floor(std::sqrt(static_cast<double>(n)))); // factor below

    // Square grid candidate
    uint64_t x_sq = k, y_sq = k;
    uint64_t empty_sq = x_sq * y_sq - n;

    // Rectangle candidate
    uint64_t x_rect = std::min(m, static_cast<uint64_t>(std::ceil(n / static_cast<double>(m))));
    uint64_t y_rect = std::max(m, static_cast<uint64_t>(std::ceil(n / static_cast<double>(m))));
    // Enforce n_x >= n_y
    if (x_rect < y_rect) std::swap(x_rect, y_rect);
    uint64_t empty_rect = x_rect * y_rect - n;

    // Prefer square unless it wastes more cells than the rectangle
    if (empty_sq <= empty_rect) {
      grid.x = x_sq;
      grid.y = y_sq;
    } else {
      grid.x = x_rect;
      grid.y = y_rect;
    }
  }

  // Case B: exactly one of x/y is missing -> infer the other
  else if (!grid.x && grid.y) {
    grid.x = std::ceil(n / static_cast<double>(grid.y));
  } else if (grid.x && !grid.y) {
    grid.y = std::ceil(n / static_cast<double>(grid.x));
  }

  // Case C: both are provided -> nothing to do

  // Sanity checks
  if (grid.x * grid.y < n) {
    throw SemanticError("Grid size (" + std::to_string(grid.x) + ", " + std::to_string(grid.y) +
                        ") is too small for input.n_ranks (" + std::to_string(n) + ").");
  }
  if (grid.x == 0 || grid.y == 0) {
    throw SemanticError("Computed grid contains zero dimension.");
  }
}

} /* end namespace vt::tv::utility */
