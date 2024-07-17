/*
//@HEADER
// *****************************************************************************
//
//                           test_info.cc
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

#include <vt-tv/api/info.h>
#include <vt-tv/api/phase_work.h>

#include <fmt-vt/format.h>

#include <string>
#include <filesystem>
#include <iostream>
#include <variant>
#include <set>

#include "../generator.h"


namespace vt::tv::tests::unit::api {

struct InfoTestParam
{
  public:

  InfoTestParam(size_t num_objects, size_t num_ranks, size_t num_phases):
    num_objects(num_objects), num_ranks(num_ranks), num_phases(num_phases) {

  }

  friend std::ostream& operator<< (std::ostream& stream, const InfoTestParam& param) {
    stream << "{num_objects: " << std::to_string(param.num_objects) << ", num_ranks: " << std::to_string(param.num_ranks) << ", num_phases: " << std::to_string(param.num_phases) << "}";
    return stream;
  }

  const size_t num_objects;
  const size_t num_ranks;
  const size_t num_phases;
};

/**
 * Provides unit tests for the vt::tv::api::Info class
 */
class InfoTest :public ::testing::TestWithParam<InfoTestParam> { };

/**
 * Test Info:getNumRanks returns same number of ranks as defined in the sample
 */
TEST_P(InfoTest, test_get_num_ranks) {
  InfoTestParam const & param = GetParam();
  Info info = Generator::makeInfo(param.num_objects, param.num_ranks, param.num_phases);
  EXPECT_EQ(info.getNumRanks(), param.num_ranks);
}

/**
 * Test Info:getNumPhases
 */
TEST_P(InfoTest, test_get_num_phases) {
  InfoTestParam const & param = GetParam();
  Info info = Generator::makeInfo(param.num_objects, param.num_ranks, param.num_phases);

  EXPECT_EQ(info.getNumPhases(), param.num_phases);
}

/**
 * Test Info:getRankIDs
 */
TEST_P(InfoTest, test_get_rank_ids) {
  InfoTestParam const & param = GetParam();
  Info info = Generator::makeInfo(param.num_objects, param.num_ranks, param.num_phases);
  fmt::print("{}={}", param.num_phases, info.getNumPhases());

  auto rank_ids = std::vector<NodeType>();
  for (NodeType rank_id = 0; rank_id< param.num_ranks; rank_id++) {
    rank_ids.push_back(rank_id);
  }

  ASSERT_THAT(info.getRankIDs(), ::testing::UnorderedElementsAreArray(rank_ids));
}

/**
 * Test Info:getObjectQoiGetter
 */
TEST_P(InfoTest, test_get_object_qoi_getter) {
  InfoTestParam const & param = GetParam();
  Info info = Generator::makeInfo(param.num_objects, param.num_ranks, param.num_phases);

  auto qoi_list = std::vector<std::string>({"load", "received_volume", "sent_volume", "max_volume", "id", "rank_id", "non-existent"});
  for (auto const qoi: qoi_list) {
    auto qoi_getter = info.getObjectQoiGetter(qoi);
  }
}

/**
 * Test Info:getObjectQoi
 */
TEST_P(InfoTest, test_get_object_qoi) {
  InfoTestParam const & param = GetParam();

  ObjectWork object_0 = ObjectWork(0, 2.0, {}, {}, {});

  Info info = Info(
    Generator::makeObjectInfoMap({{ 0, object_0 }}), // 1 object
    Generator::makeRanks({{ 0, object_0 }}, 1, 1) // 1 phase 1 rank
  );
  auto qoi_list = std::vector<std::string>({"load", "received_volume", "sent_volume", "max_volume", "id", "rank_id", "non-existent"});
  for (auto const qoi: qoi_list) {
    if (qoi == "non-existent") {
      EXPECT_THROW(info.getObjectQoi(0, 0, qoi), std::runtime_error);
    } else {
      auto qoi_value = info.getObjectQoi(0, 0, qoi);
    }
  }
}

/**
 * Test Info:getAllObjectIDs returns same items as defined in the sample
 */
TEST_P(InfoTest, test_get_all_object_ids) {
  InfoTestParam const & param = GetParam();

  auto objects = Generator::makeObjects(param.num_objects);
  auto ranks = Generator::makeRanks(objects, param.num_ranks, param.num_phases);
  auto object_info_map = Generator::makeObjectInfoMap(objects);
  auto info = Info(object_info_map, ranks);

  auto const& expected = object_info_map;
  auto const& actual = info.getAllObjectIDs();

  EXPECT_EQ(expected.size(), actual.size());
  for (auto& pair: expected) {
    auto object_id = pair.first;
    EXPECT_NE(actual.find(pair.first), actual.end()) << "Cannot find element with id " << object_id;
  }
}

/* Run Unit tests using different data sets as Tests params */
INSTANTIATE_TEST_SUITE_P(
    InfoTests,
    InfoTest,
    ::testing::Values<InfoTestParam>(
        // num_objects, num_ranks, num_phases
        InfoTestParam(0,0,0), // empty case
        InfoTestParam(2,5,1),
        InfoTestParam(6,1,1)
    ),
    [](const testing::TestParamInfo<InfoTest::ParamType>& info) {
      // test suffix
      return std::to_string(info.param.num_objects) + "_" +
             std::to_string(info.param.num_ranks) + "_" +
             std::to_string(info.param.num_phases);
    }
);

/**
 * Test Info::addInfo
 */
TEST_F(InfoTest, test_add_info) {

  Info info = Info();

  std::vector<size_t> idx;

  // Create object info and add to a map
  ObjectInfo oInfo = ObjectInfo(0, 0, true, idx);
  auto object_info_map = std::unordered_map<ElementIDType, ObjectInfo>();
  object_info_map.insert(std::make_pair(oInfo.getID(), oInfo));

  // Create PhaseWork including one object work
  auto object_work_objects = std::unordered_map<PhaseType, ObjectWork>();
  PhaseWork phase = PhaseWork(0, object_work_objects);

  // Create PhaseWork map
  auto phase_object_map = std::unordered_map<PhaseType, PhaseWork>();
  phase_object_map.insert(std::make_pair(0, phase));

  // Create a rank with id 0 and the PhaseWork map instance
  Rank rank = Rank(0, phase_object_map);

  EXPECT_EQ(info.getNumRanks(), 0);
  info.addInfo(object_info_map, rank);

  EXPECT_EQ(info.getNumRanks(), 1);
  EXPECT_EQ(info.getRank(0).getPhaseWork().size(), 1);

  // Rank already added. Expected assertion error (DEBUG).
  ASSERT_DEBUG_DEATH({ info.addInfo(object_info_map, rank); }, "Rank must not exist");
}

/**
 * Test Info:getNumPhases with inconstent rank phases.
 */
TEST_F(InfoTest, test_inconsistent_number_of_phases_across_ranks_throws_error) {
  Rank rank_0 = Rank(0, {{0, {}}}, {}); // Rank with 1 phase
  Rank rank_1 = Rank(1, {{0, {}}, {1, {}}}, {}); // Rank with 2 phases
  Info info = Info(Generator::makeObjectInfoMap(Generator::makeObjects(10)), { {0, rank_0}, {1, rank_1} } );
  EXPECT_THROW(info.getNumPhases(), std::runtime_error);
}

TEST_F(InfoTest, test_get_phase_objects) {
  auto objects_15 = Generator::makeObjects(2, 1.5, 0);
  auto objects_18 = Generator::makeObjects(2, 1.8, 2);
  auto objects_20 = Generator::makeObjects(2, 2.0, 4);

  auto objects_15_info = Generator::makeObjectInfoMap(objects_15);
  auto objects_18_info = Generator::makeObjectInfoMap(objects_18);
  auto objects_20_info = Generator::makeObjectInfoMap(objects_20);

  Rank rank_0 = Rank(0, { { 0, PhaseWork(0, objects_15) }, { 1, PhaseWork(1, {}) } }, {});
  Rank rank_1 = Rank(1, { { 0, PhaseWork(0, {})         }, { 1, PhaseWork(1, objects_18) } }, {});
  Rank rank_2 = Rank(2, { { 0, PhaseWork(0, objects_20) }, { 1, PhaseWork(1, {}) } }, {});

  auto objects_info = std::unordered_map<ElementIDType,ObjectInfo>();
  objects_info.merge(objects_15_info);
  objects_info.merge(objects_18_info);
  objects_info.merge(objects_20_info);

  Info info = Info(objects_info, { {0, rank_0}, {1, rank_1} , {2, rank_2} } );

  auto phase_0_objects = info.getPhaseObjects(0);
  for (auto& [key, val] : phase_0_objects) {
    fmt::print("Phase 0 has object {} at key {} with load {}\n", val.getID(), key, val.getLoad());
  }
  ASSERT_EQ(phase_0_objects.size(), 4);
  std::vector<int> actual_phase_0_object_ids;
  for (const auto& [key, val]: phase_0_objects) {
    actual_phase_0_object_ids.push_back(key);
  }
  ASSERT_THAT(actual_phase_0_object_ids, ::testing::UnorderedElementsAre(0, 1, 4, 5));
  ASSERT_EQ(phase_0_objects.at(0).getLoad(), 1.5);
  ASSERT_EQ(phase_0_objects.at(1).getLoad(), 1.5);
  ASSERT_EQ(phase_0_objects.at(4).getLoad(), 2.0);
  ASSERT_EQ(phase_0_objects.at(5).getLoad(), 2.0);

  auto phase_1_objects = info.getPhaseObjects(1);
  for (auto& [key, val] : phase_1_objects) {
    fmt::print("Phase 1 has object {} at key {} with load {}\n", val.getID(), key, val.getLoad());
  }
  ASSERT_EQ(phase_1_objects.size(), 2);
  std::vector<int> actual_phase_1_object_ids;
  for (const auto& [key, val]: phase_1_objects) {
    actual_phase_1_object_ids.push_back(key);
  }
  ASSERT_THAT(actual_phase_1_object_ids, ::testing::ElementsAre(2, 3));
  ASSERT_EQ(phase_1_objects.at(2).getLoad(), 1.8);
  ASSERT_EQ(phase_1_objects.at(3).getLoad(), 1.8);
}

/**
 * Test Info:getMaxLoad is valid value after changing the selected phase
 */
TEST_F(InfoTest, test_get_max_load_after_changing_selected_phase) {
  auto objects_15 = Generator::makeObjects(2, 1.5, 0);
  auto objects_18 = Generator::makeObjects(2, 1.8, 2);
  auto objects_20 = Generator::makeObjects(2, 2.0, 4);

  auto objects_15_info = Generator::makeObjectInfoMap(objects_15);
  auto objects_18_info = Generator::makeObjectInfoMap(objects_18);
  auto objects_20_info = Generator::makeObjectInfoMap(objects_20);

  Rank rank_0 = Rank(0, { { 0, PhaseWork(0, objects_15) }, { 1, PhaseWork(1, {}) } }, {});
  Rank rank_1 = Rank(1, { { 0, PhaseWork(0, {})         }, { 1, PhaseWork(1, objects_18) } }, {});
  Rank rank_2 = Rank(2, { { 0, PhaseWork(0, objects_20) }, { 1, PhaseWork(1, {}) } }, {});

  auto objects_info = std::unordered_map<ElementIDType,ObjectInfo>();
  objects_info.merge(objects_15_info);
  objects_info.merge(objects_18_info);
  objects_info.merge(objects_20_info);

  Info info = Info(objects_info, { {0, rank_0}, {1, rank_1} , {2, rank_2} } );

  // max volume for all phases (initial state)
  EXPECT_EQ(info.getMaxLoad(), 2.0);

  info.setSelectedPhase(1);
  EXPECT_EQ(info.getMaxLoad(), 1.8);

  info.setSelectedPhase(0);
  EXPECT_EQ(info.getMaxLoad(), 2.0);

  // max volume for all phases
  info.setSelectedPhase(std::numeric_limits<PhaseType>::max());
  EXPECT_EQ(info.getMaxLoad(), 2.0);
}

/**
 * Test Info:getNumPhases with inconstent rank phases.
 */
TEST_F(InfoTest, test_get_max_volume_throws_out_of_range_after_set_invalid_phase) {
  Rank rank_0 = Rank(0, {{0, PhaseWork()}, {1, PhaseWork()}}, {});
  Rank rank_1 = Rank(1, {{0, PhaseWork()}, {1, PhaseWork()}}, {});
  Info info = Info(Generator::makeObjectInfoMap(Generator::makeObjects(10)), { {0, rank_0}, {1, rank_1} } );
  info.setSelectedPhase(2);
  EXPECT_THROW(info.getMaxVolume(), std::runtime_error);
}

TEST_F(InfoTest, test_convert_qoi_variant_type_to_double_throws_runtime_error_for_string) {
  auto info = Info();
  EXPECT_THROW(info.convertQOIVariantTypeToDouble_("some_string_value"), std::runtime_error);
}

} // end namespace vt::tv::tests::unit
