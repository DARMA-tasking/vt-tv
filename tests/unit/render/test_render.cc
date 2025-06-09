/*
//@HEADER
// *****************************************************************************
//
//                                test_render.cc
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

#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkPolyDataReader.h>
#include <vtkXMLPolyDataReader.h>

#include <vt-tv/render/render.h>
#include <vt-tv/utility/json_reader.h>
#include <vt-tv/utility/parse_render.h>

#include <set>
#include <regex>

#include "../generator.h"
#include "../util.h"

namespace vt::tv::tests::unit::render {

using Util = vt::tv::tests::unit::Util;

/**
 * Provides unit tests for the vt::tv::render::Render class to test with config file input
 * Note: PNG output is turned off for these tests.
 */
class RenderTest : public ::testing::TestWithParam<std::string> {
protected:
  Render createRender(YAML::Node config, Info info, std::string& output_dir) {
    // change output directory to use specific one for these tests
    output_dir = Util::resolveDir(
      SRC_DIR, config["output"]["directory"].as<std::string>(), true);
    // Logic copied from the ParseRender class here
    return Render(
      {config["viz"]["rank_qoi"].as<std::string>(),
       "",
       config["viz"]["object_qoi"].as<std::string>()},
      config["viz"]["force_continuous_object_qoi"].as<bool>(),
      info,
      {config["viz"]["x_ranks"].as<uint64_t>(),
       config["viz"]["y_ranks"].as<uint64_t>(),
       config["viz"]["z_ranks"].as<uint64_t>()},
      0.0,
      output_dir,
      config["output"]["file_stem"].as<std::string>(),
      1.0,
      config["viz"]["save_meshes"].as<bool>(),
      config["viz"]["save_pngs"].as<bool>(),
      std::numeric_limits<PhaseType>::max());
  }

  void printVtkPolyData(vtkPolyData* poly) {
    std::string points_str = "";
    for (auto k = 0; k < poly->GetNumberOfPoints(); ++k) {
      double* coords = poly->GetPoint(k);
      points_str = fmt::format(
        "{}    [{}] = [{}, {}, {}]\n",
        points_str,
        k,
        coords[0],
        coords[1],
        coords[2]);
    }

    std::string point_array_str = "";
    for (auto k = 0; k < poly->GetPointData()->GetNumberOfArrays(); ++k) {
      auto array_name = poly->GetPointData()->GetArrayName(k);
      auto array = poly->GetPointData()->GetArray(array_name);

      std::string components_str = "";
      for (auto tupleIdx = 0; tupleIdx < array->GetNumberOfTuples();
           ++tupleIdx) {
        for (auto compIdx = 0; compIdx < array->GetNumberOfComponents();
             ++compIdx) {
          components_str = fmt::format(
            "{}    [{}] = [TupleId={}, ComponentId={}]\n",
            components_str,
            k,
            tupleIdx,
            compIdx);
        }
      }

      point_array_str = fmt::format(
        "{}    [{}] = [Name=\"{}\", Type={}, Size={}, Components={}]\n",
        point_array_str,
        k,
        array_name,
        array->GetArrayType(),
        array->GetSize(),
        components_str);
    }

    fmt::print(
      R"STR(
> PointData.NumberOfArrays => {}
> NumberOfPoints => {}
> Lines.Data.Name => {}
> Points => [
  {}
  ]
  PointData.Arrays() => [
  {}
  ]
)STR",
      poly->GetPointData()->GetNumberOfArrays(),
      poly->GetNumberOfPoints(),
      Util::formatNullable(poly->GetLines()->GetData()->GetName()),
      points_str,
      point_array_str);
  }

  void assertPolyEquals(vtkPolyData* actual, vtkPolyData* expected) {
    // fmt::print("Actual vtkPolyData:\n");
    // printVtkPolyData(actual);
    // fmt::print("Expected vtkPolyData:\n");
    // printVtkPolyData(expected);

    // Assertions required to test vt-tv meshaes
    // Number of point data should be ranks
    ASSERT_EQ(
      actual->GetPointData()->GetNumberOfArrays(),
      expected->GetPointData()->GetNumberOfArrays());

    // Validate points
    ASSERT_EQ(actual->GetNumberOfPoints(), expected->GetNumberOfPoints());

    for (auto k = 0; k < actual->GetNumberOfPoints(); ++k) {
      double actualCoords[3];
      double expectedCoords[3];
      actual->GetPoint(k, actualCoords);
      expected->GetPoint(k, expectedCoords);

      ASSERT_EQ(actualCoords[0], expectedCoords[0])
        << "Invalid point X coordinate at index " << k
        << "diff=" << (expectedCoords[0] - actualCoords[0]);
      ASSERT_EQ(actualCoords[1], expectedCoords[1])
        << "Invalid point Y coordinate at index " << k
        << "diff=" << (expectedCoords[1] - actualCoords[1]);
      ASSERT_EQ(actualCoords[2], expectedCoords[2])
        << "Invalid point Z coordinate at index " << k
        << "diff=" << (expectedCoords[2] - actualCoords[2]);
    }

    // Validate lines
    ASSERT_EQ(
      actual->GetLines()->GetData()->GetName(),
      expected->GetLines()->GetData()->GetName());
    // TODO: browse lines
  }
};

/**
 * Test Render:generate correcty run the different configuration files and generates mesh files (.vtp)
 */
TEST_P(RenderTest, test_render_from_config_with_png) {
  std::string const& config_file = GetParam();
  YAML::Node config =
    YAML::LoadFile(fmt::format("{}/tests/config/{}", SRC_DIR, config_file));
  Info info = Generator::loadInfoFromConfig(config);

  uint64_t win_size = 2000;
  uint64_t font_size = 50;

  std::string output_dir;
  std::string output_file_stem =
    config["output"]["file_stem"].as<std::string>();

  // Render
  Render render = createRender(config, info, output_dir);
  std::filesystem::create_directories(output_dir);

  render.generate(font_size, win_size);

  // Check: that number of generated mesh files (*.vtp) correspond to the number of phases
  for (uint64_t i = 0; i < info.getNumPhases(); i++) {
    auto rank_mesh_file =
      fmt::format("{}{}_rank_mesh_{}.vtp", output_dir, output_file_stem, i);
    auto object_mesh_file =
      fmt::format("{}{}_object_mesh_{}.vtp", output_dir, output_file_stem, i);
    auto png_file = fmt::format("{}{}{}.png", output_dir, output_file_stem, i);
    auto expected_rank_mesh_file = fmt::format(
      "{}/tests/expected/{}/{}_rank_mesh_{}.vtp",
      SRC_DIR,
      output_file_stem,
      output_file_stem,
      i);
    auto expected_object_mesh_file = fmt::format(
      "{}/tests/expected/{}/{}_object_mesh_{}.vtp",
      SRC_DIR,
      output_file_stem,
      output_file_stem,
      i);

    fmt::print("----- Testing phase {} -----\n", i);
    // 1. test expected output files existence for phase: rank mesh, object mesh, png
    ASSERT_TRUE(std::filesystem::exists(rank_mesh_file))
      << fmt::format("Error: rank mesh not generated at {}", object_mesh_file);
    ASSERT_TRUE(std::filesystem::exists(object_mesh_file)) << fmt::format(
      "Error: object mesh not generated at {}", object_mesh_file);
    ASSERT_TRUE(std::filesystem::exists(png_file))
      << fmt::format("Error: PNG image not generated at {}", png_file);

    // 2. test PNG with tolerance
    fmt::print(
      "Testing png file {}\n",
      std::filesystem::path(png_file).filename().c_str());
    if (std::filesystem::exists(png_file)) {
      auto expected_png_file = fmt::format(
        "{}/tests/expected/{}/{}{}.png",
        SRC_DIR,
        output_file_stem,
        output_file_stem,
        i);
      auto cmd = fmt::format(
        "ACTUAL={} EXPECTED={} TOLERANCE=0.1 "
        "{}/tests/test_image.sh",
        png_file, expected_png_file, SRC_DIR
      );
      cout << cmd << endl;
      const auto [status, output] = Util::exec(cmd.c_str());
      cout << output << endl;
      ASSERT_EQ(status, EXIT_SUCCESS) << output;
    } else {
      ADD_FAILURE() << "Cannot test png file (not generated)";
    }

    // 3. test vtp's file content
    // 3.1 rank mesh file
    fmt::print(
      "Testing rank mesh file {}\n",
      std::filesystem::path(rank_mesh_file).filename().c_str());
    if (std::filesystem::exists(rank_mesh_file)) {
      // 3.1.1 Compare raw vtp files
      auto rank_mesh_content = Util::getFileContent(rank_mesh_file);
      auto expected_rank_mesh_content =
        Util::getFileContent(expected_rank_mesh_file);
      ASSERT_EQ(expected_rank_mesh_content, rank_mesh_content) << fmt::format(
        "rank mesh file content differs from expected at phase {}", i);

      // 3.1.2 Compare polydata using vtkXMLPolyDataReader
      vtkNew<vtkXMLPolyDataReader> expected_rank_mesh_reader;
      expected_rank_mesh_reader->SetFileName(expected_rank_mesh_file.c_str());
      expected_rank_mesh_reader->Update();
      vtkPolyData* expected_rank_mesh = expected_rank_mesh_reader->GetOutput();

      vtkNew<vtkXMLPolyDataReader> rank_mesh_reader;
      rank_mesh_reader->SetFileName(rank_mesh_file.c_str());
      rank_mesh_reader->Update();
      vtkPolyData* rank_mesh = rank_mesh_reader->GetOutput();

      this->assertPolyEquals(rank_mesh, expected_rank_mesh);
    } else {
      ADD_FAILURE() << "Cannot test rank mesh file (not generated)";
    }

    // 3.2 object mesh file
    fmt::print(
      "Testing object mesh file {}\n",
      std::filesystem::path(object_mesh_file).filename().c_str());
    if (std::filesystem::exists(object_mesh_file)) {
      // 3.2.1 Compare raw vtp files
      auto object_mesh_content = Util::getFileContent(object_mesh_file);
      auto expected_object_mesh_content =
        Util::getFileContent(expected_object_mesh_file);
      ASSERT_EQ(expected_object_mesh_content, object_mesh_content)
        << fmt::format(
             "object mesh file content differs from expected at phase {}", i);

      // 3.2.2 Compare polydata using vtkXMLPolyDataReader
      vtkNew<vtkXMLPolyDataReader> expected_object_mesh_reader;
      expected_object_mesh_reader->SetFileName(
        expected_object_mesh_file.c_str());
      expected_object_mesh_reader->Update();
      vtkPolyData* expected_object_mesh =
        expected_object_mesh_reader->GetOutput();

      vtkNew<vtkXMLPolyDataReader> object_mesh_reader;
      object_mesh_reader->SetFileName(object_mesh_file.c_str());
      object_mesh_reader->Update();
      vtkPolyData* object_mesh = object_mesh_reader->GetOutput();

      this->assertPolyEquals(object_mesh, expected_object_mesh);
    } else {
      ADD_FAILURE() << "Cannot test object mesh file (not generated)";
    }

    fmt::print("----- Finished testing phase {} -----\n", i);
  }
}

/* Run with different configuration files */
INSTANTIATE_TEST_SUITE_P(
  RenderTests,
  RenderTest,
  ::testing::Values<std::string>("conf.yaml", "ccm-example.yaml", "ccm-example_phase_1.yaml"),
  [](const ::testing::TestParamInfo<std::string>& in_info) {
    // test suffix as slug
    auto suffix = std::regex_replace(in_info.param, std::regex("\\.yaml"), "");
    suffix = std::regex_replace(suffix, std::regex("-"), "_");
    return suffix;
  });

TEST_F(RenderTest, test_render_construct_from_info) {
  // old example2 now as a test
  using namespace vt;
  using namespace tv;
  // Read JSON file and input data

  std::filesystem::path p =
    std::filesystem::path(SRC_DIR) / "data/lb_test_data/";
  std::string path = std::filesystem::absolute(p).string();

  NodeType n_ranks = 4;

  std::unique_ptr<Info> info = std::make_unique<Info>();

  for (NodeType rank = 0; rank < n_ranks; rank++) {
    utility::JSONReader reader{rank};

    // Validate the JSON data file
    std::string data_file_path = path + "/data." + std::to_string(rank) + ".json";
    if (reader.validate_datafile(data_file_path)) {
      reader.readFile(data_file_path);
      auto tmpInfo = reader.parse();

      info->addInfo(tmpInfo->getObjectInfo(), tmpInfo->getRank(rank));
    } else {
      ADD_FAILURE() << "JSON data file is invalid: " + data_file_path;
    }

  }

  fmt::print("Num ranks={}\n", info->getNumRanks());

  // Instantiate render
  auto r = Render(*info);
  r.generate();

  SUCCEED();
}

} // namespace vt::tv::tests::unit::render
