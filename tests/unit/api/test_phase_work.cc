/*
//@HEADER
// *****************************************************************************
//
//                              test_phase_work.cc
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

#include <vt-tv/api/phase_work.h>

#include "../util.h"
#include "../generator.h"
#include "basic_serializer.h"

namespace vt::tv::tests::unit::api {

using ObjectWorkMap = std::unordered_map<ElementIDType, ObjectWork>;

/**
 * Provides unit tests for the vt::tv::api::PhaseWork class
 */
struct PhaseWorkTest : public ::testing::Test {
public:
  PhaseWorkTest() {
    objects_0 = Generator::makeObjects(10);
    phase_0 = PhaseWork(
      11,       // phase id
      objects_0 // map of ObjectWork
    );
  }

  ObjectWorkMap objects_0;
  PhaseWork phase_0;
};

/**
 * Test PhaseWork initial state
 */
TEST_F(PhaseWorkTest, test_initial_state) {
  // Assertions for phase_0
  EXPECT_EQ(phase_0.getPhase(), 11);
  EXPECT_EQ(
    phase_0.getLoad(),
    std::accumulate(
      objects_0.begin(), objects_0.end(), 0, [](double value, const auto& p) {
        return value + p.second.getLoad();
      }));
  EXPECT_EQ(phase_0.getMaxVolume(), 0.0);
}

/**
 * Test PhaseWork communications methods:
 * addObjectSentCommunication, addObjectReceivedCommunication, getMaxVolume before and after some communications
 */
TEST_F(PhaseWorkTest, test_communications) {
  // Assertions for phase_0

  // change some object communicator
  auto object_id = 2;
  ObjectCommunicator communicator = ObjectCommunicator(3);

  // object id (2) != communicator object id (3). Expected assertion error (DEBUG).
  ASSERT_DEBUG_DEATH(
    { phase_0.setCommunications(object_id, communicator); }, "");

  object_id = 3;
  phase_0.setCommunications(object_id, communicator);

  EXPECT_EQ(phase_0.getObjectWork().at(object_id).getSent().size(), 0);
  EXPECT_EQ(phase_0.getObjectWork().at(object_id).getSentVolume(), 0.0);

  // scenario: move 25 bytes from task 2 to task 3.
  auto from_id = 2;
  auto to_id = 3;
  phase_0.addObjectSentCommunication(object_id, to_id, 25.0);
  phase_0.addObjectReceivedCommunication(object_id, from_id, 25.0);
  EXPECT_EQ(phase_0.getObjectWork().at(object_id).getSent().size(), 1);
  EXPECT_EQ(phase_0.getObjectWork().at(object_id).getSentVolume(), 25.0);
  EXPECT_EQ(phase_0.getObjectWork().at(object_id).getReceived().size(), 1);
  EXPECT_EQ(phase_0.getObjectWork().at(object_id).getReceivedVolume(), 25.0);

  EXPECT_EQ(phase_0.getMaxVolume(), 25.0);
  phase_0.addObjectReceivedCommunication(object_id, from_id, 40.0);
  EXPECT_EQ(phase_0.getMaxVolume(), 40.0);
}

} // namespace vt::tv::tests::unit::api
