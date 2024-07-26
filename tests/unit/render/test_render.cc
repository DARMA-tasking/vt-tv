/*
//@HEADER
// *****************************************************************************
//
//                           test_render.cc
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

#include <gtest/gtest.h>

#include <yaml-cpp/yaml.h>

#include <vt-tv/render/render.h>
#include <vt-tv/utility/json_reader.h>
#include <vt-tv/utility/parse_render.h>

#include <string>
#include <filesystem>
#include <iostream>
#include <variant>
#include <set>
#include <regex>

#include "../generator.h"

using namespace vt::tv::tests::unit;

namespace vt::tv::tests::unit::render {

/**
 * Provides unit tests for the vt::tv::render::Render class to test with config file input
 */
class RenderTest :public ::testing::TestWithParam<std::string> {

  void SetUp() override {
    // Make the output directory for these tests
    std::filesystem::create_directory(fmt::format("{}/output", SRC_DIR));
    std::filesystem::create_directory(fmt::format("{}/output/tests", SRC_DIR));
    std::filesystem::create_directory(fmt::format("{}/output/tests/render", SRC_DIR));
  }

  protected:
    Render createRender(YAML::Node config, Info info, std::string &output_dir) {
      // change output directory to use specific one for these tests
      output_dir = "output/tests/render";

      std::filesystem::path output_path(output_dir);
      if (output_path.is_relative()) {
        output_path = std::filesystem::path(SRC_DIR) / output_path;
      }
      output_dir = output_path.string();

      // append / to avoid problems with file stems
      if (!output_dir.empty() && output_dir.back() != '/') {
        output_dir += '/';
      }

      return Render(
      {
          config["viz"]["rank_qoi"].as<std::string>(),
          "",
          config["viz"]["object_qoi"].as<std::string>()
        },
        config["viz"]["force_continuous_object_qoi"].as<bool>(),
        info,
        {
          config["viz"]["x_ranks"].as<uint64_t>(),
          config["viz"]["y_ranks"].as<uint64_t>(),
          config["viz"]["z_ranks"].as<uint64_t>()
        },
        config["viz"]["object_jitter"].as<double>(),
        output_dir,
        config["output"]["file_stem"].as<std::string>(),
        1.0,
        config["viz"]["save_meshes"].as<bool>(),
        false,
        std::numeric_limits<PhaseType>::max()
      );
    }
};

/**
 * Test Render:generate correcty run the different configuration files and generates mesh files (.vtp)
 */
TEST_P(RenderTest, test_render_from_config) {
  std::string const & config_file = GetParam();
  YAML::Node config = YAML::LoadFile(fmt::format("{}/tests/config/{}", SRC_DIR, config_file));
  Info info = Generator::loadInfoFromConfig(config);

  uint64_t win_size = 2000;
  uint64_t font_size = 50;

  std::string output_dir;

  // Render
  Render render = createRender(config, info, output_dir);
  render.generate(font_size, win_size);

  // Check: that number of generated mesh files (*.vtp) correspond to the number of phases
  std::string output_file_stem = config["output"]["file_stem"].as<std::string>();
  for (uint64_t i = 0; i<info.getNumPhases(); i++) {
    ASSERT_TRUE(
      std::filesystem::exists(fmt::format("{}{}_rank_mesh_{}.vtp", output_dir, output_file_stem, i))
    ) << fmt::format("{}{}_rank_mesh_{}.vtp", output_dir, output_file_stem, i);
    ASSERT_TRUE(
      std::filesystem::exists(fmt::format("{}{}_object_mesh_{}.vtp", output_dir, output_file_stem, i))
    ) << fmt::format("{}{}_rank_mesh_{}.vtp", output_dir, output_file_stem, i);
  }

  // Validate mesh files (*.vtp) content ?
  // Is it needed
}

/* Run with different configuration files */
INSTANTIATE_TEST_SUITE_P(
    RenderTests,
    RenderTest,
    ::testing::Values<std::string>(
        "conf.yaml",
        "ccm-example.yaml"
    ),
    [](const ::testing::TestParamInfo<std::string>& in_info) {
      // test suffix as slug
      auto suffix = std::regex_replace(in_info.param, std::regex("\\.yaml"), "");
      suffix = std::regex_replace(suffix, std::regex("-"), "_");
      return suffix;
    }
);

TEST_F(RenderTest, test_render_construct_from_info) {
  // old example2 now as a test
  using namespace vt;
  using namespace tv;
  // Read JSON file and input data

  std::filesystem::path p = std::filesystem::path(SRC_DIR) / "tests/data/lb_test_data/";
  std::string path = std::filesystem::absolute(p).string();

  NodeType n_ranks = 4;

  std::unique_ptr<Info> info = std::make_unique<Info>();

  for (NodeType rank = 0; rank < n_ranks; rank++) {
    utility::JSONReader reader{rank};
    reader.readFile(path + "/data." + std::to_string(rank) + ".json");
    auto tmpInfo = reader.parse();
    info->addInfo(tmpInfo->getObjectInfo(), tmpInfo->getRank(rank));
  }

  fmt::print("Num ranks={}\n", info->getNumRanks());

  // Instantiate render
  auto r = Render(*info);
  r.generate();

  SUCCEED();
}

} // end namespace vt::tv::tests::unit
