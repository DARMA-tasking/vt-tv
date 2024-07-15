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

} // end namespace vt::tv::tests::unit
