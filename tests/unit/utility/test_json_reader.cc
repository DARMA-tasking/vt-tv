/*
//@HEADER
// *****************************************************************************
//
//                             test_json_reader.cc
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

#include <vt-tv/api/info.h>
#include <vt-tv/utility/json_reader.h>
#include <vt-tv/utility/qoi_serializer.h>

#include "../util.h"

namespace vt::tv::tests::unit::utility {

using JSONReader = vt::tv::utility::JSONReader;

/**
 * Provides unit tests for the vt::tv::utility::JSONReader class
 */
struct JSONReaderTest :public ::testing::Test {};

TEST_F(JSONReaderTest, test_json_reader_1) {
  std::filesystem::path p = std::filesystem::path(SRC_DIR) / "data/lb_test_data" ;
  std::string path = std::filesystem::absolute(p).string();

  NodeType rank = 0;
  JSONReader reader{rank};
  reader.readFile(path + "/data.0.json");
  auto info = reader.parse();

  auto const& obj_info = info->getObjectInfo();

  fmt::print("Object info size={}\n", obj_info.size());
  fmt::print("Num ranks={}\n", info->getNumRanks());

  EXPECT_EQ(info->getNumRanks(), 1);

  for (auto const& [elm_id, oi] : obj_info) {
    fmt::print(
      "elm_id={:x}, home={}, migratable={}, index_array size={}\n",
      elm_id, oi.getHome(), oi.isMigratable(), oi.getIndexArray().size()
    );
    EXPECT_EQ(elm_id, oi.getID());
    fmt::print("elm_id: {}, oi.getID: {}\n", elm_id, oi.getID());

    // for this dataset, no migrations happen so all objects should be on home
    EXPECT_EQ(oi.getHome(), rank);
  }

  auto& rank_info = info->getRank(rank);
  EXPECT_EQ(rank_info.getRankID(), rank);

  auto& phases = rank_info.getPhaseWork();

  // for this dataset, expect that all phases have the same objects
  std::set<ElementIDType> phase_0_objects;
  // and we should have a phase 0
  auto const& phase_0 = phases.find(0);
  fmt::print("phase_0 type: {}", typeid(phase_0).name());
  auto const& phase_0_object_work = phase_0->second.getObjectWork();

  for (auto const& [elm_id, _] : phase_0_object_work) {
    phase_0_objects.insert(elm_id);
  }

  for (auto const& [phase, phase_work] : phases) {
    fmt::print("phase={}\n", phase);
    // sizes should be consistent
    EXPECT_EQ(phase_work.getObjectWork().size(), phase_0_objects.size());
    for (auto const& [elm_id, work] : phase_work.getObjectWork()) {
      // object should be found in phase 0
      EXPECT_TRUE(phase_0_objects.find(elm_id) != phase_0_objects.end());
      fmt::print("\t elm_id={:x}: load={}\n", elm_id, work.getLoad());
    }
  }
}

TEST_F(JSONReaderTest, test_json_reader_metadata_attributes) {
  std::filesystem::path p = std::filesystem::path(SRC_DIR) / "data/lb_test_data" ;
  std::string path = std::filesystem::absolute(p).string();

  NodeType rank = 0;
  JSONReader reader{rank};

  reader.readFile(path + "/reader_test_data.json");
  auto info = reader.parse();
  auto& rank_info = info->getRank(rank);
  EXPECT_EQ(rank_info.getRankID(), rank);

  auto& rank_attributes = rank_info.getAttributes();
  EXPECT_TRUE(rank_attributes.find("intSample") != rank_attributes.end());
  EXPECT_EQ(1, std::get<int>(rank_attributes.at("intSample")));

  EXPECT_TRUE(rank_attributes.find("doubleSample") != rank_attributes.end());
  EXPECT_EQ(2.213, std::get<double>(rank_attributes.at("doubleSample")));

  EXPECT_TRUE(rank_attributes.find("stringSample") != rank_attributes.end());
  EXPECT_EQ("abc", std::get<std::string>(rank_attributes.at("stringSample")));
}

TEST_F(JSONReaderTest, test_json_reader_object_info_attributes) {
  std::filesystem::path p = std::filesystem::path(SRC_DIR) / "data/lb_test_data" ;
  std::string path = std::filesystem::absolute(p).string();

  NodeType rank = 0;
  JSONReader reader{rank};

  reader.readFile(path + "/reader_test_data.json");
  auto info = reader.parse();
  auto& rank_info = info->getRank(rank);
  EXPECT_EQ(rank_info.getRankID(), rank);

  auto const& objects = info->getRankObjects(0, 0);
  auto const& object_work = objects.at(3407875);

  auto& object_attributes = object_work.getAttributes();
  EXPECT_TRUE(object_attributes.find("intSample") != object_attributes.end());
  EXPECT_EQ(-100, std::get<int>(object_attributes.at("intSample")));

  EXPECT_TRUE(object_attributes.find("doubleSample") != object_attributes.end());
  EXPECT_EQ(0, std::get<double>(object_attributes.at("doubleSample")));

  EXPECT_TRUE(object_attributes.find("stringSample") != object_attributes.end());
  EXPECT_EQ("", std::get<std::string>(object_attributes.at("stringSample")));
}

TEST_F(JSONReaderTest, test_json_reader_qoi_serializer) {
  using json = nlohmann::json;

  // int in json
  json int_json = 1;
  QOIVariantTypes int_variant = int_json.get<QOIVariantTypes>();
  EXPECT_TRUE(std::holds_alternative<int>(int_variant));
  EXPECT_EQ(1, std::get<int>(int_variant));

  // double in json
  json double_json = 123.456;
  QOIVariantTypes double_variant = double_json.get<QOIVariantTypes>();
  EXPECT_TRUE(std::holds_alternative<double>(double_variant));
  EXPECT_EQ(123.456, std::get<double>(double_variant));

  // std::string in json
  json string_json = "some data";
  QOIVariantTypes string_variant = string_json.get<QOIVariantTypes>();
  EXPECT_TRUE(std::holds_alternative<std::string>(string_variant));
  EXPECT_EQ("some data", std::get<std::string>(string_variant));
}

TEST_F(JSONReaderTest, test_json_reader_object_work_user_defined) {
  std::filesystem::path p = std::filesystem::path(SRC_DIR) / "data/lb_test_data" ;
  std::string path = std::filesystem::absolute(p).string();

  NodeType rank = 0;
  JSONReader reader{rank};

  reader.readFile(path + "/reader_test_data.json");
  auto info = reader.parse();
  auto& rank_info = info->getRank(rank);
  EXPECT_EQ(rank_info.getRankID(), rank);

  auto const& objects = info->getRankObjects(0, 0);
  auto const& object_info = objects.at(3407875);

  auto& user_defined = object_info.getUserDefined();
  EXPECT_FALSE(user_defined.empty());
  EXPECT_TRUE(user_defined.find("isSample") != user_defined.end());
  EXPECT_EQ(1, std::get<int>(user_defined.at("isSample")));
}

} // end namespace vt::tv::tests::unit
