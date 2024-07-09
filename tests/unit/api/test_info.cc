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

#include <vt-tv/api/info.h>
#include <vt-tv/api/phase_work.h>

#include <fmt-vt/format.h>

#include <string>
#include <filesystem>
#include <iostream>
#include <variant>
#include <set>

#include "generator.h"


namespace vt::tv::tests::unit::api {

struct TestParam
{
  public:

  TestParam(size_t num_objects, size_t num_ranks, size_t num_phases):
    num_objects(num_objects), num_ranks(num_ranks), num_phases(num_phases) {

  }

  const size_t num_objects;
  const size_t num_ranks;
  const size_t num_phases;

  std::string get_slug() const
  {
    return "sample_" + std::to_string(num_objects) + "_objects_" + std::to_string(num_ranks) + "_ranks_" + std::to_string(num_phases) + "_phases";
  }
};

/**
 * Provides unit tests for the vt::tv::api::Info class
 */
class ParametherizedTestFixture :public ::testing::TestWithParam<TestParam> {};

/**
 * Test Info:getNumRanks returns same number of ranks as defined in the sample
 */
TEST_P(ParametherizedTestFixture, test_get_num_ranks) {
  TestParam const & param = GetParam();
  Info info = Generator::makeInfo(param.num_objects, param.num_ranks, param.num_phases);
  EXPECT_EQ(info.getNumRanks(), param.num_ranks);
}

/**
 * Test Info:getAllObjectIDs returns same items as defined in the sample
 */
TEST_P(ParametherizedTestFixture, test_get_all_object_ids) {
  TestParam const & param = GetParam();

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
    TestInfo,
    ParametherizedTestFixture,
    ::testing::Values<TestParam>(
        TestParam(0,0,1),
        TestParam(2,5,1),
        TestParam(6,1,1)
    ),
    [](const testing::TestParamInfo<ParametherizedTestFixture::ParamType>& info) {
      // test suffix
      return info.param.get_slug();
    }
);

/**
 * Test Info:addInfo does not add twice an already-added object info.
 */
TEST_F(ParametherizedTestFixture, test_add_info) {

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

  info.addInfo(object_info_map, rank);
  EXPECT_EQ(info.getRank(0).getPhaseWork().size(), 1) << "object info has already been added and must not be added to the map again";
}

} // end namespace vt::tv::tests::unit
