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

#include <fmt-vt/format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <vt-tv/api/phase_work.h>

#include <filesystem>
#include <iostream>
#include <numeric>
#include <set>
#include <string>
#include <variant>

#include "helper.h"

namespace vt::tv::tests::unit::api {

/**
 * Provides unit tests for the vt::tv::api::ObjectWork class
 */
class PhaseWorkTestFixture : public ::testing::Test {
 public:
  PhaseWorkTestFixture() {
    objects_0 = Helper::make_objects(10);
    phase_0 = PhaseWork(
      11, // phase id
      objects_0 // map of ObjectWork
    );
  }

  std::unordered_map<ElementIDType, ObjectWork> objects_0;
  PhaseWork phase_0;
};

/**
 * Test PhaseWork contructor and getters
 */
TEST_F(PhaseWorkTestFixture, test_initializer) {
  // Assertions for phase_0
  EXPECT_EQ(phase_0.getPhase(), 11);
  EXPECT_EQ(
      phase_0.getLoad(),
      std::accumulate(objects_0.begin(), objects_0.end(), 0,
                      [](double value, const std::pair<ElementIDType, ObjectWork>& p) {
                        return value + p.second.getLoad();
                      }));
}

TEST_F(PhaseWorkTestFixture, test_communications) {
  // Assertions for phase_0

  // create communicator for object with id 2
  auto object_id = 2;
  ObjectCommunicator communicator = ObjectCommunicator(214);
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
}

}  // namespace vt::tv::tests::unit::api
