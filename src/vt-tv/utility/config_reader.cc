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
