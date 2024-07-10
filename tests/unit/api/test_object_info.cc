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
#include <gmock/gmock.h>

#include <vt-tv/api/object_info.h>

#include <fmt-vt/format.h>

#include <string>
#include <filesystem>
#include <iostream>
#include <variant>
#include <set>


namespace vt::tv::tests::unit::api {

/**
 * Provides unit tests for the vt::tv::api::ObjectInfo class
 */
class ObjectInfoTest :public ::testing::Test {
  public:
    ObjectInfo object_0 = ObjectInfo(
      6, // id
      2, // home
      false, // migratable
      std::vector<size_t>({ 0, 1, 2})
    );

    ObjectInfo object_1 = ObjectInfo(
      7, // id
      1, // home
      true, // migratable
      std::vector<size_t>({ 3, 5, 6})
    );
};


/**
 * Test ObjectWork:ObjectWork() and getters
 */
TEST_F(ObjectInfoTest, test_initializer) {
  // Assertions for object_0
  EXPECT_EQ(object_0.getID(), 6);
  EXPECT_EQ(object_0.getHome(), 2);
  EXPECT_EQ(object_0.getIndexArray().size(), 3);
  ASSERT_THAT(object_0.getIndexArray(), ::testing::ElementsAre(0, 1, 2));
  EXPECT_FALSE(object_0.isMigratable());

  // Assertions for object_1
  EXPECT_EQ(object_1.getID(), 7);
  EXPECT_EQ(object_1.getHome(), 1);
  EXPECT_EQ(object_1.getIndexArray().size(), 3);
  ASSERT_THAT(object_1.getIndexArray(), ::testing::ElementsAre(3, 5, 6));
  EXPECT_TRUE(object_1.isMigratable());

  // Post-modifiers assertions on object_0
  object_0.setMetaID(static_cast<vt::tv::CollectionObjGroupIDType>(2));
  EXPECT_EQ(object_0.getMetaID(), 2);

  object_0.setIsObjGroup(true);
  EXPECT_TRUE(object_0.getIsObjGroup());

  object_0.setIsObjGroup(false);
  EXPECT_FALSE(object_0.getIsObjGroup());
}

} // end namespace vt::tv::tests::unit
