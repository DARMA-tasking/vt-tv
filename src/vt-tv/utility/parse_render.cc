/*
//@HEADER
// *****************************************************************************
//
//                               parse_render.cc
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

#include "vt-tv/utility/parse_render.h"
#include "vt-tv/utility/json_reader.h"
#include "vt-tv/render/render.h"
#include "vt-tv/api/info.h"

#include <filesystem>
#include <regex>

namespace vt::tv::utility {

void ParseRender::parseAndRender(
  PhaseType phase_id, std::unique_ptr<Info> external_info) {
  try {
    // Load configuration
    ConfigReader cfg =
      (mode_ == RunMode::Standalone)
        ? ConfigReader::from_file(filename_)
        : ConfigReader::from_binding_inputs(
            binding_yaml_fragment_,
            binding_num_ranks_);

    std::unique_ptr<Info> info = std::move(external_info);

    if (info == nullptr) {
      if (mode_ == RunMode::Standalone) {
        // Discover input files
        std::filesystem::path input_path(cfg.input.directory);
        // If it's a relative path, prepend the SRC_DIR
        if (input_path.is_relative()) {
          input_path = std::filesystem::path(SRC_DIR) / input_path;
        }
        const std::string input_dir_abs = std::filesystem::absolute(input_path).string();

        const std::string stem = cfg.input.file_stem.value_or("data");
        std::regex pattern(stem + R"(\.\d+\.json(\.br)?)");

        std::vector<std::filesystem::path> data_files;
        for (const auto& entry : std::filesystem::directory_iterator(input_dir_abs)) {
          if (entry.is_regular_file()) {
            const std::string filename = entry.path().filename().string();
            if (std::regex_match(filename, pattern)) {
              data_files.push_back(entry.path());
            }
          }
        }

        if (data_files.size() != cfg.input.n_ranks) {
          throw SemanticError(
            "Found " + std::to_string(data_files.size()) + " data files in '" + input_dir_abs +
            "', but input.n_ranks is " + std::to_string(cfg.input.n_ranks) + "."
          );
        }

        info = std::make_unique<Info>();

#if VT_TV_OPENMP_ENABLED
        const int threads = VT_TV_N_THREADS;
        omp_set_num_threads(threads);
        fmt::print("vt-tv: Using {} threads\n", threads);
#pragma omp parallel for
#endif // VT_TV_OPENMP_ENABLED
        for (uint64_t i = 0; i < data_files.size(); ++i) {
          const auto filepath = data_files[i].string();
          const auto filename = data_files[i].filename().string();

          auto first = filename.find('.');
          auto next  = filename.find('.', first + 1);
          auto rank  = std::stoll(filename.substr(first + 1, next - first - 1));

          fmt::print("Reading file for rank {}\n", rank);
          utility::JSONReader reader{static_cast<NodeType>(rank)};
          if (!reader.validate_datafile(filepath)) {
            throw std::runtime_error("JSON data file is invalid: " + filepath);
          }
          reader.readFile(filepath);
          auto tmpInfo = reader.parse();
#if VT_TV_OPENMP_ENABLED
#pragma omp critical
#endif
          { info->addInfo(tmpInfo->getObjectInfo(), tmpInfo->getRank(rank)); }
        }

        if (info->getNumRanks() != cfg.input.n_ranks) {
          throw SemanticError("Parsed ranks do not match n_ranks.");
        }

      } else { // RunMode::Binding
        if (binding_json_per_rank_.size() != binding_num_ranks_) {
          throw SemanticError("Must have same number of rank JSON blobs as num_ranks.");
        }

        info = std::make_unique<Info>();

#if VT_TV_OPENMP_ENABLED
        const int threads = VT_TV_N_THREADS;
        omp_set_num_threads(threads);
        fmt::print("vt-tv: Using {} threads\n", threads);
#pragma omp parallel for
#endif // VT_TV_OPENMP_ENABLED
        for (uint64_t rank_id = 0; rank_id < binding_num_ranks_; ++rank_id) {
          fmt::print("Reading file for rank {}\n", rank_id);
          const std::string& rank_json_str = binding_json_per_rank_[rank_id];
          utility::JSONReader reader{static_cast<NodeType>(rank_id)};
          reader.readString(rank_json_str);
          auto tmpInfo = reader.parse();
#if VT_TV_OPENMP_ENABLED
#pragma omp critical
#endif
          { info->addInfo(tmpInfo->getObjectInfo(), tmpInfo->getRank(rank_id)); }
        }

        if (info->getNumRanks() != binding_num_ranks_) {
          throw SemanticError("Parsed ranks do not match num_ranks (binding).");
        }
      }
    }

    // Prepare rendering parameters
    std::array<std::string, 3> qoi_request = {
      cfg.viz.rank_qoi.value_or("load"),
      "",
      cfg.viz.object_qoi.value_or("load")
    };
    const bool save_meshes = cfg.viz.save_meshes.value_or(true);
    const bool save_pngs   = cfg.viz.save_pngs.value_or(true);
    const bool continuous_object_qoi =
      cfg.viz.force_continuous_object_qoi.value_or(true);


    std::array<std::uint64_t, 3> grid_size = { cfg.grid.x, cfg.grid.y, 1 }; // hard setting z to 1 in grid
    const double object_jitter = cfg.viz.object_jitter.value_or(0.5);

    std::string output_dir;
    std::string output_file_stem = cfg.output.file_stem.value_or("vttv");
    uint64_t win_size = cfg.output.window_size.value_or(2000);
    // Use automatic font size if not defined by user
    // 0.025 is the factor of the window size determined to be ideal for the font size
    uint64_t font_size = cfg.output.font_size.value_or(static_cast<uint64_t>(0.025 * win_size));

    if (save_meshes || save_pngs) {
      std::filesystem::path output_path = cfg.output.directory.value_or("output");
      // If it's a relative path, prepend the SRC_DIR
      if (output_path.is_relative()) {
        output_path = std::filesystem::path(SRC_DIR) / output_path;
      }
      // Create the output directory if it does not already exist
      std::filesystem::create_directory(output_path);
      output_dir = output_path.string();
      // append / to avoid problems with file stems
      if (!output_dir.empty() && output_dir.back() != '/') {
        output_dir += '/';
      }
    } else {
      fmt::print("Warning: save_pngs and save_meshes are both False "
                  "(no visualization will be generated).\n");
    }

    // Instantiate render
    Render r(
      qoi_request,
      continuous_object_qoi,
      *std::move(info),
      grid_size,
      object_jitter,
      output_dir,
      output_file_stem,
      1.0,
      save_meshes,
      save_pngs,
      phase_id);

    if (save_meshes || save_pngs) {
      r.generate(font_size, win_size);
    }

  } catch (const ValidationError& e) {
    std::cerr << "Config schema error: " << e.what() << "\n";
  } catch (const SemanticError& e) {
    std::cerr << "Config semantic error: " << e.what() << "\n";
  } catch (const std::exception& e) {
    std::cerr << "Error: " << e.what() << "\n";
  }
}

} /* end namespace vt::tv::utility */
