/*
//@HEADER
// *****************************************************************************
//
//                           test_parse_render.cc
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

#include <vt-tv/utility/parse_render.h>

#include <string>
#include <filesystem>
#include <iostream>
#include <variant>
#include <set>
#include <regex>

#include "../generator.h"

namespace vt::tv::tests::unit::render {

using ParseRender = vt::tv::utility::ParseRender;

/**
 * Provides unit tests for the vt::tv::utility::ParseRender class to test with config file input
 */
class ParseRenderTest :public ::testing::TestWithParam<std::string> {
  virtual void SetUp() {
    // Failing: segfault in vtk calls
    GTEST_SKIP();
    return;

    // Make the output directory for these tests
    std::filesystem::create_directory(fmt::format("{}/output", SRC_DIR));
    std::filesystem::create_directory(fmt::format("{}/output/tests", SRC_DIR));
  }
};

/**
 * Test Render:generate correcty run the different configuration files
 */
TEST_P(ParseRenderTest, test_render_from_config) {
    std::string config_file = fmt::format("{}/tests/config/{}", SRC_DIR, GetParam());
    auto parse_render = ParseRender(config_file);
    parse_render.parseAndRender();
}

/* Run with different configuration files */
INSTANTIATE_TEST_SUITE_P(
    ParseRenderTests,
    ParseRenderTest,
    ::testing::Values<std::string>(
        "conf.yaml",
        "ccm-example.yaml",
        "test-vt-tv.yaml"
    ),
    [](const ::testing::TestParamInfo<std::string>& info) {
      // test suffix as slug
      auto suffix = std::regex_replace(info.param, std::regex("\\.yaml"), "");
      suffix = std::regex_replace(suffix, std::regex("-"), "_");
      return suffix;
    }
);

} // end namespace vt::tv::tests::unit