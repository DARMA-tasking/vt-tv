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
#include "test_harness.h"

#include "vt-tv/api/info.h"

#include <fmt-vt/format.h>

#include "cmake_config.h"

#include <string>
#include <filesystem>
#include <iostream>
#include <variant>

namespace vt::tv::tests::unit {

/**
 * Represents a sample set of data for the tests Info class tests
 */
struct Sample {

  public:
    std::unordered_map<NodeType, Rank> ranks;
    std::unordered_map<ElementIDType, ObjectWork> object_work_map;
    std::unordered_map<ElementIDType, ObjectInfo> object_info_map;
};

/**
 * Provides unit tests for the vttv.api.Info class
 */
struct TestInfo : TestHarness {

  /**
   * Generates sample data using a single phase and a default phase load
   */
  auto create_sample(int num_ranks, int num_objects_per_rank) {
    std::unordered_map<NodeType, Rank> ranks = std::unordered_map<NodeType, Rank>();
    std::unordered_map<ElementIDType, ObjectWork> object_work_map = std::unordered_map<ElementIDType, ObjectWork>();
    std::unordered_map<ElementIDType, ObjectInfo> object_info_map = std::unordered_map<ElementIDType, ObjectInfo>();
    std::vector<size_t> in_index;

    for (NodeType rank_id = 0; rank_id < num_ranks; rank_id++) {
      // Add some objects
      for (auto object_index = 0; object_index < num_objects_per_rank; object_index++) {
        auto object_id = rank_id * num_objects_per_rank + object_index;
        auto phase_load = 2.0;

        // object work
        std::unordered_map<SubphaseType, TimeType> sub_phase_loads = std::unordered_map<SubphaseType, TimeType>();
        auto object_work = ObjectWork(object_id, phase_load, sub_phase_loads);
        object_work_map.insert(std::make_pair(object_id, object_work));

        // object info
        ObjectInfo object_info = ObjectInfo(object_id, rank_id, true, in_index);
        object_info_map.insert(std::make_pair(object_id, object_info));
      }
    }

    // Add a phase (id 0)
    std::unordered_map<PhaseType, PhaseWork> phase_info = std::unordered_map<PhaseType, PhaseWork>();
    auto phase = PhaseWork(0, object_work_map);
    phase_info.insert(std::make_pair(0, phase));

    // Add some ranks
    for (NodeType rank_id = 0; rank_id < num_ranks; rank_id++) {
      auto rank = Rank(rank_id, phase_info);
      ranks.insert(std::make_pair(rank_id, rank));
    }

    // Return sample data
    auto sample = Sample();
    sample.ranks = ranks;
    sample.object_info_map = object_info_map;
    sample.object_work_map = object_work_map;

    return sample;
  }

  virtual void SetUp() {
    TestHarness::SetUp();
  }
};

/**
 * Test Info:get* methods
 */
TEST_F(TestInfo, test_info_getter_methods) {

  std::cout << "Testing empty" << std::endl;
  std::unique_ptr<Info> info_00 = std::make_unique<Info>();  
  EXPECT_EQ(info_00->getNumRanks(), 0);
  EXPECT_EQ(info_00->getAllObjectIDs().size(), 0) << "getAllObjectIDs() ok";

  auto sample_01 = create_sample(2, 5);
  std::unique_ptr<Info> info_01 = std::make_unique<Info>(sample_01.object_info_map, sample_01.ranks);
  std::cout << "Testing sample_01 (2 ranks, 5 objects per rank)" << std::endl;
  EXPECT_EQ(info_01->getNumRanks(), 2);
  EXPECT_EQ(info_01->getAllObjectIDs().size(), sample_01.object_work_map.size()) << "getAllObjectIDs() ok";

  auto sample_02 = create_sample(6, 1);
  printf("Test sample_01 (6 ranks, 1 object per rank)\n");
  std::unique_ptr<Info> info_02 = std::make_unique<Info>(sample_02.object_info_map, sample_02.ranks);
  EXPECT_EQ(info_02->getNumRanks(), 6);
  EXPECT_EQ(info_02->getAllObjectIDs().size(), sample_02.object_work_map.size()) << "getAllObjectIDs() ok";
}

} // end namespace vt::tv::tests::unit
