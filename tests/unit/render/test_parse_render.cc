/*
//@HEADER
// *****************************************************************************
//
//                             test_parse_render.cc
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

#include <yaml-cpp/yaml.h>

#include <vt-tv/utility/parse_render.h>

#include <regex>

#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkXMLPolyDataReader.h>

#include "../generator.h"
#include "../util.h"

namespace vt::tv::tests::unit::render {

using ParseRender = vt::tv::utility::ParseRender;
using Util = vt::tv::tests::unit::Util;

/**
 * Provides unit tests for the vt::tv::utility::ParseRender class to test with config file input
 */
class ParseRenderTest : public ::testing::TestWithParam<std::string> { };

/**
 * Test ParseRender:parseAndRender correcty run the different configuration files
 */
TEST_P(ParseRenderTest, test_parse_render_no_png_generates_mesh_files_only) {
  std::string const& config_file = GetParam();
  auto parse_render =
    ParseRender(fmt::format("{}/tests/config/{}", SRC_DIR, config_file));
  YAML::Node config =
    YAML::LoadFile(fmt::format("{}/tests/config/{}", SRC_DIR, config_file));
  Info info = Generator::loadInfoFromConfig(config);

  auto output_dir = Util::resolveDir(
    SRC_DIR, config["output"]["directory"].as<std::string>(), true);
  std::filesystem::create_directories(output_dir);

  auto output_file_stem = config["output"]["file_stem"].as<std::string>();

  // generate file containing object jitter dimensions for output trace and input usage
  // this is needed for image and mesh tests
  ASSERT_NO_THROW(parse_render.parseAndRender());

  for (uint64_t i = 0; i < info.getNumPhases(); i++) {
    auto rank_mesh_file =
      fmt::format("{}{}_rank_mesh_{}.vtp", output_dir, output_file_stem, i);
    auto object_mesh_file =
      fmt::format("{}{}_object_mesh_{}.vtp", output_dir, output_file_stem, i);
    auto png_file = fmt::format("{}{}{}.png", output_dir, output_file_stem, i);

    fmt::print("----- Test phase {} -----\n", i);
    // 1. test files exist: rank mesh, object mesh, png
    ASSERT_TRUE(std::filesystem::exists(rank_mesh_file))
      << fmt::format("Error: rank mesh not generated at {}", rank_mesh_file);
    ASSERT_TRUE(std::filesystem::exists(object_mesh_file)) << fmt::format(
      "Error: object mesh not generated at {}", object_mesh_file);
    ASSERT_FALSE(std::filesystem::exists(png_file))
      << fmt::format("Error: PNG image not generated at {}", png_file);

  } // end phases loop
}

/* Run with different configuration files */
INSTANTIATE_TEST_SUITE_P(
  ParseRenderTests,
  ParseRenderTest,
  ::testing::Values<std::string>("conf-no-png.yaml", "ccm-example-no-png.yaml"),
  [](const ::testing::TestParamInfo<std::string>& in_info) {
    // test suffix as slug
    auto suffix = std::regex_replace(in_info.param, std::regex("\\.yaml"), "");
    suffix = std::regex_replace(suffix, std::regex("-"), "_");
    return suffix;
  });

} // namespace vt::tv::tests::unit::render