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
class ParseRenderTest :public ::testing::TestWithParam<std::string> {

  void SetUp() override {
    // Make the output directory for these tests
    std::filesystem::create_directory(fmt::format("{}/output", SRC_DIR));
    std::filesystem::create_directory(fmt::format("{}/output/tests", SRC_DIR));
  }

protected:

  void printVtkPolyData(vtkPolyData *poly) {
    std::string points_str = "";
    for(auto k=0; k < poly->GetNumberOfPoints(); ++k) {
      double coords[3];
      poly->GetPoint(k, coords);
      points_str = fmt::format("{}    [{}] = [{}, {}, {}]\n", points_str, k, coords[0], coords[1], coords[2]);
    }

    fmt::print(
      R"STR(
> GetPointData()->GetNumberOfArrays() => {}
> GetNumberOfPoints() => {}
> GetLines()->GetData()->GetName() => {}
> Points => [
  {}
  ]
)STR",
      poly->GetPointData()->GetNumberOfArrays(),
      poly->GetNumberOfPoints(),
      poly->GetNumberOfPoints(),
      Util::formatNullable(poly->GetLines()->GetData()->GetName()),
      points_str
    );
  }

  void assertPolyEquals(vtkPolyData *actual, vtkPolyData *expected) {
    fmt::print("Actual vtkPolyData\n---------------\n");
    printVtkPolyData(actual);
    fmt::print("Expected vtkPolyData\n---------------\n");
    printVtkPolyData(expected);

    // Assertions required to test vt-tv meshaes
    // Number of point data should be ranks
    ASSERT_EQ(actual->GetPointData()->GetNumberOfArrays(), expected->GetPointData()->GetNumberOfArrays());

    // Validate points
    ASSERT_EQ(actual->GetNumberOfPoints(), expected->GetNumberOfPoints());
    for(auto k=0; k < actual->GetNumberOfPoints(); ++k) {
      double actualCoords[3];
      double expectedCoords[3];
      actual->GetPoint(k, actualCoords);
      expected->GetPoint(k, expectedCoords);

      ASSERT_EQ(actualCoords[0], expectedCoords[0]);
      ASSERT_EQ(actualCoords[1], expectedCoords[1]);
      ASSERT_EQ(actualCoords[2], expectedCoords[2]);
    }

    // Validate lines
    ASSERT_EQ(actual->GetLines()->GetData()->GetName(), expected->GetLines()->GetData()->GetName());
    // TODO: browse lines
  }
};

/**
 * Test ParseRender:parseAndRender correcty run the different configuration files
 */
TEST_P(ParseRenderTest, test_parse_config_and_render_output) {
  std::string const & config_file = GetParam();
  auto parse_render = ParseRender(fmt::format("{}/tests/config/{}", SRC_DIR, config_file));
  ASSERT_NO_THROW(parse_render.parseAndRender());

  YAML::Node config = YAML::LoadFile(fmt::format("{}/tests/config/{}", SRC_DIR, config_file));
  Info info = Generator::loadInfoFromConfig(config);

  auto output_dir = Util::resolveDir(SRC_DIR, config["output"]["directory"].as<std::string>(), true);
  auto output_file_stem = config["output"]["file_stem"].as<std::string>();
  // auto n_ranks = config["input"]["n_ranks"].as<int64_t>();

  for (uint64_t i = 0; i<info.getNumPhases(); i++) {
    auto rank_mesh_file = fmt::format("{}{}_rank_mesh_{}.vtp", output_dir, output_file_stem, i);
    auto object_mesh_file = fmt::format("{}{}_object_mesh_{}.vtp", output_dir, output_file_stem, i);
    auto png_file = fmt::format("{}{}{}.png", output_dir, output_file_stem, i);
    auto expected_rank_mesh_file = fmt::format("{}/tests/expected/{}/{}_rank_mesh_{}.vtp",
                                                SRC_DIR, output_file_stem, output_file_stem, i);
    auto expected_object_mesh_file = fmt::format("{}/tests/expected/{}/{}_object_mesh_{}.vtp",
                                                SRC_DIR, output_file_stem, output_file_stem, i);

    fmt::format("----- Test phase {} -----", i);

    // 1. test files exist: rank mesh, object mesh, png
    ASSERT_TRUE(
      std::filesystem::exists(rank_mesh_file)
    ) << fmt::format("Error: rank mesh not generated at {}", object_mesh_file);
    ASSERT_TRUE(
      std::filesystem::exists(object_mesh_file)
    ) << fmt::format("Error: object mesh not generated at {}", object_mesh_file);
    ASSERT_TRUE(
      std::filesystem::exists(png_file)
    ) << fmt::format("Error: PNG image not generated at {}", png_file);

    // 2. test PNG with tolerance
    fmt::print("-- Test PNG --\n");
    auto expected_png_file = fmt::format("{}/tests/expected/{}/{}{}.png", SRC_DIR, output_file_stem, output_file_stem, i);
    std::vector<std::string> cmd_vars = {
      fmt::format("ACTUAL={}", png_file),
      fmt::format("EXPECTED={}", expected_png_file),
      "TOLERANCE=2.0",
    };
    auto cmd = fmt::format("{} {}/tests/test_image.sh",
      fmt::join(cmd_vars, " "),
      SRC_DIR
    );
    const auto [status, output] = Util::exec(cmd.c_str());
    ASSERT_EQ(status, EXIT_SUCCESS) << fmt::format("Error: {}", output);

    // TODO: testing mesh files cannot be a simple diff as below because each run generates some different data.
    //       The future test should test XML Nodes
    // 3. test vtp's file content
    // 3.1 rank mesh file
    fmt::print("-- Test rank mesh --\n");

    // auto rank_mesh_content = Util::getFileContent(rank_mesh_file);
    // auto expected_rank_mesh_content = Util::getFileContent(expected_rank_mesh_file);
    // ASSERT_EQ(expected_rank_mesh_content, rank_mesh_content) << fmt::format("rank mesh file content differs from expected at phase {}", i);

    vtkNew<vtkXMLPolyDataReader> expected_rank_mesh_reader;
    expected_rank_mesh_reader->SetFileName(expected_rank_mesh_file.c_str());
    expected_rank_mesh_reader->Update();
    vtkPolyData *expected_rank_mesh = expected_rank_mesh_reader->GetOutput();

    vtkNew<vtkXMLPolyDataReader> rank_mesh_reader;
    rank_mesh_reader->SetFileName(rank_mesh_file.c_str());
    rank_mesh_reader->Update();
    vtkPolyData *rank_mesh = rank_mesh_reader->GetOutput();

    this->assertPolyEquals(rank_mesh, expected_rank_mesh);

    // 3.2 object mesh file
    fmt::print("-- Test object mesh --\n");

    // auto object_mesh_content = Util::getFileContent(object_mesh_file);
    // auto expected_object_mesh_content = Util::getFileContent(expected_object_mesh_file);
    // ASSERT_EQ(expected_object_mesh_content, object_mesh_content) << fmt::format("object mesh file content differs from expected at phase {}", i);

    vtkNew<vtkXMLPolyDataReader> expected_object_mesh_reader;
    expected_object_mesh_reader->SetFileName(expected_object_mesh_file.c_str());
    expected_object_mesh_reader->Update();
    vtkPolyData *expected_object_mesh = expected_object_mesh_reader->GetOutput();

    vtkNew<vtkXMLPolyDataReader> object_mesh_reader;
    object_mesh_reader->SetFileName(object_mesh_file.c_str());
    object_mesh_reader->Update();
    vtkPolyData *object_mesh = object_mesh_reader->GetOutput();

    this->assertPolyEquals(object_mesh, expected_object_mesh);

    } // end phases loop
}


/* Run with different configuration files */
INSTANTIATE_TEST_SUITE_P(
    ParseRenderTests,
    ParseRenderTest,
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

} // end namespace vt::tv::tests::unit
