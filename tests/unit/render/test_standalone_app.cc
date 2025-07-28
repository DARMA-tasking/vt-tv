/*
//@HEADER
// *****************************************************************************
//
//                            test_standalone_app.cc
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

#include <vt-tv/utility/parse_render.h>

#include "../util.h"

namespace vt::tv::tests::unit::render {

/**
 * Provides unit tests for the standalone vt-tv app.
 * It is similar to the ParseRender tests except it runs as a separate process.
 */
class StandaloneAppTest
  : public ::testing::TestWithParam<std::tuple<std::string, int>> {
  void SetUp() override {
    // This test is not testing vt-tv src.
    // That's why it is skipped. But it might be useful locally.
    GTEST_SKIP() << "Skipping standalone app tests";
    return;

    // Make the output directory for these tests
    std::filesystem::create_directories(
      fmt::format("{}/output/tests", SRC_DIR));
  }
};

/**
 * Test standalone app run with different configuration files
 */
TEST_P(StandaloneAppTest, test_run) {
  std::string config_file =
    fmt::format("{}/tests/config/{}", SRC_DIR, std::get<0>(GetParam()));
  int expected_phases = std::get<1>(GetParam());

  // Run vttv process
  auto cmd =
    fmt::format("{}/apps/vttv --conf={}", BUILD_DIR, config_file);
  const auto [status, output] = Util::exec(cmd.c_str());
  fmt::print(output);

  // Load config for some checks
  auto config = YAML::LoadFile(config_file);
  std::string output_dir = Util::resolveDir(
    SRC_DIR, config["output"]["directory"].as<std::string>(), true);
  std::string output_file_stem =
    config["output"]["file_stem"].as<std::string>();

  if (config["viz"]["object_qoi"].as<std::string>() == "shared_block_id") {
    // Temporary: this case must be removed as soon as the `shared_block_id` QOI becomes supported.
    EXPECT_THAT(
      output,
      ::testing::HasSubstr("Error reading the configuration file: Invalid "
                           "Object QOI: shared_block_id"));
  } else {
    // Expect 1 output file per phase for both rank meshes and object meshes
    for (int64_t i = 0; i < expected_phases; i++) {
      ASSERT_TRUE(std::filesystem::exists(
        fmt::format("{}{}_rank_mesh_{}.vtp", output_dir, output_file_stem, i)))
        << fmt::format(
             "Rank mesh not generated at {}{}_rank_mesh_{}.vtp",
             output_dir,
             output_file_stem,
             i);
      ASSERT_TRUE(std::filesystem::exists(fmt::format(
        "{}{}_object_mesh_{}.vtp", output_dir, output_file_stem, i)))
        << fmt::format(
             "Object mesh not generated at {}{}_object_mesh_{}.vtp",
             output_dir,
             output_file_stem,
             i);
    }
  }
}

/* Run with different configuration files */
INSTANTIATE_TEST_SUITE_P(
  StandaloneAppTests,
  StandaloneAppTest,
  // config file and expected number of phases
  ::testing::Values(
    // std::make_tuple<std::string, int>("conf.yaml", 8),
    std::make_tuple<std::string, int>("ccm-example.yaml", 1)),
  [](const ::testing::TestParamInfo<std::tuple<std::string, int>>& in_info) {
    // test suffix as slug
    auto suffix =
      std::regex_replace(std::get<0>(in_info.param), std::regex("\\.yaml"), "");
    suffix = std::regex_replace(suffix, std::regex("-"), "_");
    return suffix;
  });

} // namespace vt::tv::tests::unit::render