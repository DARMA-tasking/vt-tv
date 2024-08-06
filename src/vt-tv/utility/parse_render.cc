/*
//@HEADER
// *****************************************************************************
//
//                               parse_render.cc
//             DARMA/vt-tv => Virtual Transport -- Task Visualizer
//
// Copyright 2019 National Technology & Engineering Solutions of Sandia, LLC
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

#include "../tests/unit/cmake_config.h"

#include <filesystem>

namespace vt::tv::utility {

void ParseRender::parseAndRender(PhaseType phase_id, std::unique_ptr<Info> info) {
  try {
    // Load the yaml file
    YAML::Node config = YAML::LoadFile(filename_);

    if (info == nullptr) {
      std::string input_dir = config["input"]["directory"].as<std::string>();
      std::filesystem::path input_path(input_dir);

      // If it's a relative path, prepend the SRC_DIR
      if (input_path.is_relative()) {
        input_path = std::filesystem::path(SRC_DIR) / input_path;
      }
      input_dir = input_path.string();

      // append / to avoid problems with file stems
      if (!input_dir.empty() && input_dir.back() != '/') {
        input_dir += '/';
      }

      int64_t n_ranks = config["input"]["n_ranks"].as<int64_t>(); // signed for omp parallel for

      // Read JSON file and input data
      std::filesystem::path p = input_dir;
      std::string path = std::filesystem::absolute(p).string();

      info = std::make_unique<Info>();

      #ifdef VT_TV_OPENMP_ENABLED
      #if VT_TV_OPENMP_ENABLED
        #ifdef VT_TV_N_THREADS
        const int threads = VT_TV_N_THREADS;
        #else
        const int threads = 2;
        #endif // VT_TV_N_THREADS
        omp_set_num_threads(threads);
        fmt::print("vt-tv: Using {} threads\n", threads);
        # pragma omp parallel for
      #endif
      #endif // VT_TV_OPENMP_ENABLED
        for (int64_t rank = 0; rank < n_ranks; rank++) {
          fmt::print("Reading file for rank {}\n", rank);
          utility::JSONReader reader{static_cast<NodeType>(rank)};
          reader.readFile(input_dir + "data." + std::to_string(rank) + ".json");
          auto tmpInfo = reader.parse();
          #ifdef VT_TV_OPENMP_ENABLED
          #if VT_TV_OPENMP_ENABLED
            #pragma omp critical
          #endif
          #endif
          {
          info->addInfo(tmpInfo->getObjectInfo(), tmpInfo->getRank(rank));
          }
        }
    }

    std::array<std::string, 3> qoi_request = {
      config["viz"]["rank_qoi"].as<std::string>(),
      "",
      config["viz"]["object_qoi"].as<std::string>()
    };

    bool save_meshes = config["viz"]["save_meshes"].as<bool>();
    bool save_pngs = config["viz"]["save_pngs"].as<bool>();
    bool continuous_object_qoi = config["viz"]["force_continuous_object_qoi"].as<bool>();

    std::array<uint64_t, 3> grid_size = {
      config["viz"]["x_ranks"].as<uint64_t>(),
      config["viz"]["y_ranks"].as<uint64_t>(),
      config["viz"]["z_ranks"].as<uint64_t>()
    };

    double object_jitter = config["viz"]["object_jitter"].as<double>();

    std::string output_dir;
    std::filesystem::path output_path;
    std::string output_file_stem;
    uint64_t win_size;
    uint64_t font_size;

    if (save_meshes || save_pngs) {
      output_dir = config["output"]["directory"].as<std::string>();
      output_path = output_dir;

      // If it's a relative path, prepend the SRC_DIR
      if (output_path.is_relative()) {
        output_path = std::filesystem::path(SRC_DIR) / output_path;
      }
      output_dir = output_path.string();

      // append / to avoid problems with file stems
      if (!output_dir.empty() && output_dir.back() != '/') {
        output_dir += '/';
      }

      output_file_stem = config["output"]["file_stem"].as<std::string>();

      fmt::print("Num ranks={}\n", info->getNumRanks());

      win_size = 2000;
      if (config["output"]["window_size"]) {
        win_size = config["output"]["window_size"].as<uint64_t>();
      }

      // Use automatic font size if not defined by user
      // 0.025 is the factor of the window size determined to be ideal for the font size
      font_size = 0.025 * win_size;
      if (config["output"]["font_size"]) {
        font_size = config["output"]["font_size"].as<uint64_t>();
      }
    } else {
      fmt::print("Warning: save_pngs and save_meshes are both False "
                 "(no visualization will be generated).\n");
    }

    // Instantiate render
    Render r(
      qoi_request, continuous_object_qoi, *std::move(info), grid_size, object_jitter,
      output_dir, output_file_stem, 1.0, save_meshes, save_pngs, phase_id
    );

    if (save_meshes || save_pngs) {
      r.generate(font_size, win_size);
    }

  } catch (std::exception const& e) {
    std::cout << "Error reading the configuration file: " << e.what() << std::endl;
  }
}

} /* end namespace vt::tv::utility */
