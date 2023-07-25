/*
//@HEADER
// *****************************************************************************
//
//                                example2.cc
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

#include <vt-tv/api/info.h>
#include <vt-tv/utility/json_reader.h>
#include <vt-tv/render/render.h>
#include "../tests/unit/cmake_config.h"

#include <fmt-vt/format.h>
#include <CLI/CLI11.hpp>

#include <filesystem>

#include <yaml-cpp/yaml.h>

int main(int argc, char** argv) {
  using namespace vt;
  using namespace tv;

  CLI::App app{"VT-TV task visualizer"};

  std::string default_config_file = "config/conf.yaml";
  app.add_option("-c,--conf", default_config_file, "Input configuration file")->required();

  CLI11_PARSE(app, argc, argv);

  std::string yaml_file = app.get_option("-c")->as<std::string>();

  std::filesystem::path config_file_path(yaml_file);
  // If it's a relative path, prepend the SRC_DIR
  if(config_file_path.is_relative()) {
      config_file_path = std::filesystem::path(SRC_DIR) / config_file_path;
  }
  yaml_file = config_file_path.string();

  fmt::print("Input configuration file={}\n", yaml_file);

  try {
    // Load the yaml file
    YAML::Node config = YAML::LoadFile(yaml_file);

    std::string input_dir = config["input"]["directory"].as<std::string>();
    std::filesystem::path input_path(input_dir);
    // If it's a relative path, prepend the SRC_DIR
    if(input_path.is_relative()) {
        input_path = std::filesystem::path(SRC_DIR) / input_path;
    }
    input_dir = input_path.string();
    // append / to avoid problems with file stems
    if (!input_dir.empty() && input_dir.back() != '/')
      input_dir += '/';

    uint64_t n_ranks = config["input"]["n_ranks"].as<uint64_t>();

    std::array<std::string, 3> qoi_request = { config["viz"]["rank_qoi"].as<std::string>(), "", config["viz"]["object_qoi"].as<std::string>()};

    bool continuous_object_qoi = config["viz"]["force_continuous_object_qoi"].as<bool>();

    std::array<uint64_t, 3> grid_size = { config["viz"]["x_ranks"].as<uint64_t>(), config["viz"]["y_ranks"].as<uint64_t>(), config["viz"]["z_ranks"].as<uint64_t>() };

    double object_jitter = config["viz"]["object_jitter"].as<double>();

    std::string output_dir = config["output"]["directory"].as<std::string>();
    std::filesystem::path output_path(output_dir);
    // If it's a relative path, prepend the SRC_DIR
    if(output_path.is_relative()) {
        output_path = std::filesystem::path(SRC_DIR) / output_path;
    }
    output_dir = output_path.string();
    // append / to avoid problems with file stems
    if (!output_dir.empty() && output_dir.back() != '/')
      output_dir += '/';

    std::string output_file_stem = config["output"]["file_stem"].as<std::string>();

    // Read JSON file and input data
    std::filesystem::path p = input_dir;
    std::string path = std::filesystem::absolute(p).string();

    std::unique_ptr<Info> info = std::make_unique<Info>();

    for (NodeType rank = 0; rank < n_ranks; rank++) {
      utility::JSONReader reader{rank, input_dir + "data." + std::to_string(rank) + ".json"};
      reader.readFile();
      auto tmpInfo = reader.parseFile();
      info->addInfo(tmpInfo->getObjectInfo(), tmpInfo->getRank(rank));
    }

    fmt::print("Num ranks={}\n", info->getNumRanks());

    // Instantiate render
    auto r = Render(qoi_request, continuous_object_qoi, *info, grid_size, object_jitter, output_dir, output_file_stem, 1.0);
    r.generate();

  } catch (const std::exception& e) {
    std::cout << "Error reading the configuration file: " << e.what() << std::endl;
    return 1; // Return error code
  }

  return 0;
}
