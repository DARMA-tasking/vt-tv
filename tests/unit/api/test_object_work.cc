/*
//@HEADER
// *****************************************************************************
//
//                             test_object_work.cc
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

#include <vt-tv/api/object_work.h>

#include "../util.h"
#include "../generator.h"

namespace vt::tv::tests::unit::api {

/**
 * Provides unit tests for the vt::tv::api::ObjectWork class
 */
class ObjectWorkTest :public ::testing::Test {
  public:
    ObjectWork object_0 = ObjectWork(
      12,
      10.0,
      {{ 3, 12.0 }},
      Generator::makeQOIVariants(3, "user_", "_value"),
      Generator::makeQOIVariants(2, "attr_", "_value")
    );
};

TEST_F(ObjectWorkTest, test_empty_constructor) {
  ObjectWork empty = ObjectWork();
  EXPECT_EQ(empty.getID(), 0);
  EXPECT_EQ(empty.getLoad(), 0.);
}

/**
 * Test ObjectWork initial state
 */
TEST_F(ObjectWorkTest, test_std_constructor_and_getters) {
  EXPECT_EQ(object_0.getID(), 12);
  EXPECT_EQ(object_0.getLoad(), 10.0);
  EXPECT_EQ(object_0.getSubphaseLoads().size(), 1);
  EXPECT_EQ(object_0.getSubphaseLoads().at(3), static_cast<TimeType>(12.0));

  EXPECT_EQ(object_0.getUserDefined().size(), 3);
  EXPECT_EQ(object_0.getUserDefined().at("user_0"), static_cast<QOIVariantTypes>("user_0_value"));
  EXPECT_EQ(object_0.getUserDefined().at("user_1"), static_cast<QOIVariantTypes>("user_1_value"));
  EXPECT_EQ(object_0.getUserDefined().at("user_2"), static_cast<QOIVariantTypes>("user_2_value"));

  EXPECT_EQ(object_0.getAttributes().size(), 2);
  EXPECT_EQ(object_0.getAttributes().at("attr_0"), static_cast<QOIVariantTypes>("attr_0_value"));
  EXPECT_EQ(object_0.getAttributes().at("attr_1"), static_cast<QOIVariantTypes>("attr_1_value"));

  EXPECT_EQ(object_0.getReceivedVolume(), 0.0);
  EXPECT_EQ(object_0.getSentVolume(), 0.0);
}

TEST_F(ObjectWorkTest, test_add_and_get_received_volumes) {
  object_0.addReceivedCommunications(12, 56.0);
  EXPECT_EQ(object_0.getReceivedVolume(), 56.0);

  object_0.addReceivedCommunications(10, 22.5);
  EXPECT_EQ(object_0.getReceivedVolume(), 78.5);
}

TEST_F(ObjectWorkTest, test_add_and_get_sent_volumes) {
  object_0.addSentCommunications(10, 10.2);
  EXPECT_EQ(object_0.getSentVolume(), 10.2);

  object_0.addSentCommunications(10, 5.65);
  EXPECT_EQ(object_0.getSentVolume(), 15.85);
}

} // end namespace vt::tv::tests::unit
