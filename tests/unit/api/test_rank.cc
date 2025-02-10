/*
//@HEADER
// *****************************************************************************
//
//                                 test_rank.cc
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

#include <vt-tv/api/rank.h>

#include "../generator.h"

namespace vt::tv::tests::unit::api {

/**
 * Provides unit tests for the vt::tv::api::Rank class
 */
struct RankTest : public ::testing::Test {
public:
  // 6 objects with 2.0 load
  std::unordered_map<ElementIDType, ObjectWork> objects =
    Generator::makeObjects(6, 2.0);

  // 3 phases (2 objects per phase) (default load = 2.0 per object)
  std::unordered_map<ElementIDType, ObjectWork> objects_0 = {
    {0, objects.at(0)}, {1, objects.at(1)}};
  std::unordered_map<ElementIDType, ObjectWork> objects_1 = {
    {2, objects.at(2)}, {3, objects.at(3)}};
  std::unordered_map<ElementIDType, ObjectWork> objects_2 = {
    {4, objects.at(4)}, {5, objects.at(5)}};
  PhaseWork phase_0 = Generator::makePhase(0, objects_0);
  PhaseWork phase_1 = Generator::makePhase(1, objects_1);
  PhaseWork phase_2 = Generator::makePhase(2, objects_2);

  // rank
  std::unordered_map<PhaseType, PhaseWork> phase_info_0 = {
    {0, phase_0},
    {1, phase_1},
    {2, phase_2},
  };
  Rank rank_0 = Rank(2, phase_info_0, Generator::makeQOIVariants(10));
};

/**
 * Test Rank initial state
 */
TEST_F(RankTest, test_initial_state) {
  // Assertions for rank_0
  EXPECT_EQ(rank_0.getRankID(), 2);
  EXPECT_EQ(rank_0.getNumPhases(), 3);
  EXPECT_EQ(rank_0.getLoad(0, no_lb_iter), phase_0.getLoad());
  EXPECT_EQ(rank_0.getLoad(1, no_lb_iter), phase_1.getLoad());
  EXPECT_EQ(rank_0.getLoad(2, no_lb_iter), phase_2.getLoad());
  EXPECT_EQ(rank_0.getNumObjects(0, no_lb_iter), phase_0.getObjectWork().size());
  EXPECT_EQ(rank_0.getNumObjects(1, no_lb_iter), phase_1.getObjectWork().size());
  EXPECT_EQ(rank_0.getNumObjects(2, no_lb_iter), phase_2.getObjectWork().size());
  EXPECT_EQ(rank_0.getPhaseWork().size(), phase_info_0.size());
  EXPECT_EQ(rank_0.getPhaseWork().at(0).getPhase(), 0);
  EXPECT_EQ(rank_0.getPhaseWork().at(1).getPhase(), 1);
  EXPECT_EQ(rank_0.getPhaseWork().at(2).getPhase(), 2);
}

/**
 * Test Rank communication methods
 */
TEST_F(RankTest, test_communications_and_get_max_volume) {
  // object 0 not in phase 2 objects must fire an error
  ASSERT_THROW(
    { rank_0.addObjectReceivedCommunicationAtPhase(2, 0, 3, 12.0); },
    std::out_of_range);

  // Phase 1, object 2 receives from object 4 (phase 2) a 12.0 load.
  rank_0.addObjectReceivedCommunicationAtPhase(1, 2, 4, 3.5);
  ASSERT_EQ(rank_0.getPhaseWork().at(1).getMaxVolume(), 3.5);

  // send volume of 1000.0 from object to another object in same phase
  rank_0.addObjectSentCommunicationAtPhase(0, 0, 1, 6.0);
  ASSERT_EQ(rank_0.getPhaseWork().at(0).getMaxVolume(), 6.0);
  // send load=1.0 from object to another object in another phase
  rank_0.addObjectSentCommunicationAtPhase(0, 0, 5, 7.0);
  ASSERT_EQ(rank_0.getPhaseWork().at(0).getMaxVolume(), 7.0);
}

} // namespace vt::tv::tests::unit::api
