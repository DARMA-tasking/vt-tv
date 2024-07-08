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

#include "helper.h"


namespace vt::tv::tests::unit::api {


/**
 * Provides unit tests for the vt::tv::api::ObjectWork class
 */
class ObjectWorkTestFixture :public ::testing::Test {
  public:
    ObjectWork object_0 = ObjectWork(
      12,
      10.0,
      {{ 3, 12.0 }},
      { 
        { "user_defined_1", "user_defined_1_value" },
        { "user_defined_2", "user_defined_2_value" },
        { "user_defined_3", "user_defined_3_value" }
      },
      { 
        { "attribute_1", "attribute_1_value" },
        { "attribute_2", "attribute_2_value" }
      }
    );
};


/**
 * Test ObjectWork:ObjectWork() and getters
 */
TEST_F(ObjectWorkTestFixture, test_initializer) {
  EXPECT_EQ(object_0.getID(), 12);
  EXPECT_EQ(object_0.getLoad(), 10.0);
  EXPECT_EQ(object_0.getSubphaseLoads().size(), 1);
  EXPECT_EQ(object_0.getSubphaseLoads().at(3), static_cast<TimeType>(12.0));

  EXPECT_EQ(object_0.getUserDefined().size(), 3);
  EXPECT_EQ(object_0.getUserDefined().at("user_defined_1"), static_cast<QOIVariantTypes>("user_defined_1_value"));
  EXPECT_EQ(object_0.getUserDefined().at("user_defined_2"), static_cast<QOIVariantTypes>("user_defined_2_value"));
  EXPECT_EQ(object_0.getUserDefined().at("user_defined_3"), static_cast<QOIVariantTypes>("user_defined_3_value"));

  EXPECT_EQ(object_0.getAttributes().size(), 2);
  EXPECT_EQ(object_0.getAttributes().at("attribute_1"), static_cast<QOIVariantTypes>("attribute_1_value"));
  EXPECT_EQ(object_0.getAttributes().at("attribute_2"), static_cast<QOIVariantTypes>("attribute_2_value"));

  EXPECT_EQ(object_0.getReceivedVolume(), 0.0);
  EXPECT_EQ(object_0.getSentVolume(), 0.0);
}

TEST_F(ObjectWorkTestFixture, test_received_volumes) {
  object_0.addReceivedCommunications(12, 56.0);
  EXPECT_EQ(object_0.getReceivedVolume(), 56.0);

  object_0.addReceivedCommunications(10, 22.5);
  EXPECT_EQ(object_0.getReceivedVolume(), 78.5);
}

TEST_F(ObjectWorkTestFixture, test_sent_volumes) {
  object_0.addSentCommunications(10, 10.2);
  EXPECT_EQ(object_0.getSentVolume(), 10.2);

  object_0.addSentCommunications(10, 5.65);
  EXPECT_EQ(object_0.getSentVolume(), 15.85);
}

} // end namespace vt::tv::tests::unit
