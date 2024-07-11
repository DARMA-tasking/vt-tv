/*
//@HEADER
// *****************************************************************************
//
//                           test_json_reader.cc
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

#include "../generator.h"

namespace vt::tv::tests::unit::render {

/**
 * Provides unit tests for the vt::tv::render::Render class to test with config file input
 */
class RenderTest :public ::testing::TestWithParam<std::string> {

  virtual void SetUp() {
    // Make the output directory for these tests
    std::filesystem::create_directory(fmt::format("{}/output", SRC_DIR));
    std::filesystem::create_directory(fmt::format("{}/output/tests", SRC_DIR));
  }

  protected:
    Render createRender(YAML::Node config, std::unique_ptr<Info> info) {

      std::string output_dir = config["output"]["directory"].as<std::string>();
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
        *std::move(info),
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
        config["viz"]["save_pngs"].as<bool>(),
        std::numeric_limits<PhaseType>::max()
      );
    }
};

/**
 * Test Render:generate correcty run the different configuration files
 */
TEST_P(RenderTest, test_render_from_config) {
  std::string const & config_file = GetParam();
  YAML::Node config = YAML::LoadFile(fmt::format("{}/tests/config/{}", SRC_DIR, config_file));
  std::unique_ptr<Info> info = Generator::loadInfoFromConfig(config);

  uint64_t win_size = 2000;
  uint64_t font_size = 50;

  if (config["viz"]["object_qoi"].as<std::string>() == "shared_block_id") {
    // Temporary: this case must be removed as soon as the `shared_block_id` QOI becomes supported.
    EXPECT_THROW(createRender(config, std::move(info)), std::runtime_error); // "Invalid Object QOI: shared_block_id"
  } else {
    Render render = createRender(config, std::move(info));
    render.generate(font_size, win_size);
  }
}

/* Run with different configuration files */
INSTANTIATE_TEST_SUITE_P(
    RenderTests,
    RenderTest,
    ::testing::Values<std::string>(
        "conf.yaml",
        "ccm-example.yaml"
        //"test-vt-tv.yaml" // TODO: PB SegFault
    )
);

} // end namespace vt::tv::tests::unit
