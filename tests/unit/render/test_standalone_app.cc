/*
//@HEADER
// *****************************************************************************
//
//                           test_standalone_app.cc
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
#include <gmock/gmock.h>

#include <yaml-cpp/yaml.h>

#include <vt-tv/utility/parse_render.h>

#include <string>
#include <filesystem>
#include <iostream>
#include <variant>
#include <set>
#include <regex>

namespace vt::tv::tests::unit::render {

/**
 * Provides unit tests for the standalone vt-tv app.
 * It is similar to the ParseRender tests except it runs as a separate process.
 */
class StandaloneAppTest :public ::testing::TestWithParam<std::tuple<std::string, int>> {
  virtual void SetUp() {
    // Make the output directory for these tests
    std::filesystem::create_directory(fmt::format("{}/output", SRC_DIR));
    std::filesystem::create_directory(fmt::format("{}/output/tests", SRC_DIR));
  }

  protected:
    std::string exec(const char* cmd) {
        std::array<char, 128> buffer;
        std::string result;
        std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);
        if (!pipe) {
            throw std::runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    }
};

/**
 * Test standalone app run with different configuration files
 */
TEST_P(StandaloneAppTest, test_run) {
    std::string config_file = fmt::format("{}/tests/config/{}", SRC_DIR, std::get<0>(GetParam()));
    int expected_phases = std::get<1>(GetParam());

    // Run vt-tv_standalone process
    auto cmd = fmt::format("{}/apps/vt-tv_standalone --conf={}", BUILD_DIR, config_file);
    auto output = exec(cmd.c_str());
    fmt::print(output);

    // Load config for some checks
    auto config = YAML::LoadFile(config_file);
    std::string output_dir = config["output"]["directory"].as<std::string>();
    std::filesystem::path output_path(output_dir);
    // If it's a relative path, prepend the SRC_DIR
    if (output_path.is_relative()) {
      output_path = std::filesystem::path(SRC_DIR) / output_path;
    }
    output_dir = output_path.string();

    // append / to avoid problems with file stems
    if (!output_dir.empty() && output_dir.back() != '/') {
      output_dir += '/';
    }
    std::string output_file_stem = config["output"]["file_stem"].as<std::string>();

    if (config["viz"]["object_qoi"].as<std::string>() == "shared_block_id") {
      // Temporary: this case must be removed as soon as the `shared_block_id` QOI becomes supported.
      EXPECT_THAT(output, ::testing::HasSubstr("Error reading the configuration file: Invalid Object QOI: shared_block_id"));
    } else {
      // Expect 1 output file per phase for both rank meshes and object meshes
      for (int64_t i = 0; i<expected_phases; i++) {
        ASSERT_TRUE(std::filesystem::exists(fmt::format("{}{}_rank_mesh_{}.vtp", output_dir, output_file_stem, i))) << fmt::format("{}{}_rank_mesh_{}.vtp", output_dir, output_file_stem, i);
        ASSERT_TRUE(std::filesystem::exists(fmt::format("{}{}_object_mesh_{}.vtp", output_dir, output_file_stem, i))) << fmt::format("{}{}_rank_mesh_{}.vtp", output_dir, output_file_stem, i);
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
        std::make_tuple<std::string, int>("ccm-example.yaml", 1)
        // std::make_tuple<std::string, int>("test-vt-tv.yaml", 1)
    ),
    [](const ::testing::TestParamInfo<std::tuple<std::string, int>>& info) {
      // test suffix as slug
      auto suffix = std::regex_replace(std::get<0>(info.param), std::regex("\\.yaml"), "");
      suffix = std::regex_replace(suffix, std::regex("-"), "_");
      return suffix;
    }
);

} // end namespace vt::tv::tests::unit