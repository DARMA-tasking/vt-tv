/*
//@HEADER
// *****************************************************************************
//
//                           test_object_communicator.cc
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

#include <vt-tv/api/object_communicator.h>

#include <string>
#include <filesystem>
#include <iostream>
#include <variant>
#include <set>

#include "basic_serializer.h"

namespace vt::tv::tests::unit::api {

/**
 * Provides unit tests for the vt::tv::api::ObjectCommunicator class
 */
class ObjectCommunicatorTest :public ::testing::Test {
  public:
    ObjectCommunicator comm_0 = ObjectCommunicator(0);

    ObjectCommunicator comm_1 = ObjectCommunicator(
        1,
        // recv
        {
            { 1, 25.5 }, { 2, 12.0 }
        },
        // sent
        {
            { 1, 29.5 },  { 2, 10.2 },  { 3, 10.3 }
        }
    );
};

/**
 * Test empty ObjectCommunicator initial state
 */
TEST_F(ObjectCommunicatorTest, test_empty_communicator_getters) {
  // Assertions for comm_0
  EXPECT_EQ(comm_0.getObjectId(), 0);
  EXPECT_EQ(comm_0.getMaxVolume(), 0.0);
  EXPECT_EQ(comm_0.getReceived().size(), 0);
  EXPECT_EQ(comm_0.getTotalReceivedVolume(), 0.0);
  EXPECT_EQ(comm_0.getSent().size(), 0);
  EXPECT_EQ(comm_0.getTotalSentVolume(), 0.0);
}

/**
 * Test ObjectCommunicator initial state
 */
TEST_F(ObjectCommunicatorTest, test_sample_communicator_and_getters) {
  // Assertions for comm_1
  EXPECT_EQ(comm_1.getObjectId(), 1);
  EXPECT_EQ(comm_1.getMaxVolume(), 29.5);
  EXPECT_EQ(comm_1.getReceived().size(), 2);
  EXPECT_EQ(comm_1.getTotalReceivedVolume(), 37.5);
  EXPECT_EQ(comm_1.getSent().size(), 3);
  EXPECT_EQ(comm_1.getTotalSentVolume(), 50.0);
}

/**
 * Test ObjectCommunicator initial state
 */
TEST_F(ObjectCommunicatorTest, test_initial_state) {
  // Assertions for comm_0
  EXPECT_EQ(comm_0.getObjectId(), 0);
  EXPECT_EQ(comm_0.getMaxVolume(), 0.0);
  EXPECT_EQ(comm_0.getReceived().size(), 0);
  EXPECT_EQ(comm_0.getTotalReceivedVolume(), 0.0);
  EXPECT_EQ(comm_0.getSent().size(), 0);
  EXPECT_EQ(comm_0.getTotalSentVolume(), 0.0);

  // Assertions for comm_1
  EXPECT_EQ(comm_1.getObjectId(), 1);
  EXPECT_EQ(comm_1.getMaxVolume(), 29.5);
  EXPECT_EQ(comm_1.getReceived().size(), 2);
  EXPECT_EQ(comm_1.getTotalReceivedVolume(), 37.5);
  EXPECT_EQ(comm_1.getSent().size(), 3);
  EXPECT_EQ(comm_1.getTotalSentVolume(), 50.0);
}

/**
 * Test ObjectCommunicator initial state
 */
TEST_F(ObjectCommunicatorTest, test_summarize_communications_count_empty_communicator) {
  // std::make_pair(w_sent, w_recv);
  auto summary = comm_0.summarize();
  EXPECT_EQ(summary.first.size(), 0);
  EXPECT_EQ(summary.second.size(), 0);
}

/**
 * Test ObjectCommunicator initial state
 */
TEST_F(ObjectCommunicatorTest, test_summarize_communications_count) {
  GTEST_SKIP() << "ObjectCommunicator::summarize seems in WIP and is never called";

  // std::make_pair(w_sent, w_recv);
  auto summary = comm_0.summarize();
  EXPECT_EQ(summary.first.size(), 3);
  EXPECT_EQ(summary.second.size(), 2);
}

TEST_F(ObjectCommunicatorTest, test_serialization) {
  BasicSerializer<std::variant<ElementIDType,std::multimap<ElementIDType, double>>> s = BasicSerializer<std::variant<ElementIDType,std::multimap<ElementIDType, double>>>();

  comm_1.serialize(s);
  EXPECT_EQ(s.items.size(), 3); // object_id_, received_, sent_

  auto actual_object_id = std::get<PhaseType>(s.items[0]);
  EXPECT_EQ(actual_object_id, comm_1.getObjectId()); // object id

  std::multimap<ElementIDType, double> actual_received = std::get<std::multimap<ElementIDType, double>>(s.items[1]);
  std::multimap<ElementIDType, double> actual_sent = std::get<std::multimap<ElementIDType, double>>(s.items[2]);

  bool any_failure = false;
  for (auto const& [object_id, received_volume] : comm_1.getReceived()) {
    if (actual_received.find(object_id) == actual_received.cend()) {
      fmt::print("Missing received volume {} from object {} in serialized communicator data", received_volume, object_id);
      any_failure = true;
      ADD_FAILURE();
    } else if (actual_received.count(object_id) != comm_1.getReceived().count(object_id)) {
      fmt::print("Different count of received volume from object {} in serialized communicator data", object_id);
      any_failure = true;
      ADD_FAILURE();
    }

    auto object_received = actual_received.equal_range(object_id);
    // check for some missing sent volumes in serialized data
    bool found = false;
    for (auto it = object_received.first; it != object_received.second; ++it) {
      if (it->second == received_volume) {
        found = true;
      }
    }

    if (!found) {
      any_failure = true;
      fmt::print("Missing received volume {} from object {} in serialized communicator data", received_volume, object_id);
      ADD_FAILURE();
    }
  }

  for (auto const& [object_id, sent_volume] : comm_1.getSent()) {
    if (actual_sent.find(object_id) == actual_sent.cend()) {
      fmt::print("Missing sent volume {} from object {} in serialized communicator data", sent_volume, object_id);
      any_failure = true;
      ADD_FAILURE();
    } else if (actual_sent.count(object_id) != comm_1.getSent().count(object_id)) {
      fmt::print("Different count of sent volume from object {} in serialized communicator data", object_id);
      any_failure = true;
      ADD_FAILURE();
    }

    auto object_sent = actual_sent.equal_range(object_id);
    // check for some missing sent volumes in serialized data
    bool found = false;
    for (auto it = object_sent.first; it != object_sent.second; ++it) {
      if (it->second == sent_volume) {
        found = true;
      }
    }

    if (!found) {
      any_failure = true;
      fmt::print("Missing received volume {} from object {} in serialized communicator data", sent_volume, object_id);
      ADD_FAILURE();
    }
  }

  if (any_failure) {
    FAIL();
  }
}

} // end namespace vt::tv::tests::unit
