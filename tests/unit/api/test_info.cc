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

#include <fmt-vt/format.h>

#include <string>
#include <filesystem>
#include <iostream>
#include <variant>

#include "test_sample.h"

namespace vt::tv::tests::unit::api {

/**
 * Provides unit tests for the vt::tv::api::Info class
 */
class SampleParametherizedTestFixture :public ::testing::TestWithParam<Sample> {};

/**
 * Test Info:getNumRanks returns same number of ranks as defined in the sample
 */
TEST_P(SampleParametherizedTestFixture, test_get_num_ranks) {
  Sample const & sample = GetParam();
  Info* info = new Info(sample.object_info_map, sample.ranks);
  EXPECT_EQ(info->getNumRanks(), sample.ranks.size());
  delete info;
}

/**
 * Test Info:getAllObjectIDs returns same items as defined in the sample
 */
TEST_P(SampleParametherizedTestFixture, test_get_all_object_ids) {
  Sample const & sample = GetParam();
  Info* info = new Info(sample.object_info_map, sample.ranks);
  EXPECT_EQ(info->getAllObjectIDs().size(),  sample.object_info_map.size());
  delete info;
}

/* Run Unit tests using different data sets as Tests params */
INSTANTIATE_TEST_SUITE_P(
    TestInfo,
    SampleParametherizedTestFixture,
    ::testing::Values<Sample>(
        SampleFactory::create_one_phase_sample(0, 0),
        SampleFactory::create_one_phase_sample(2, 5),
        SampleFactory::create_one_phase_sample(6, 1)
    ),
    [](const testing::TestParamInfo<SampleParametherizedTestFixture::ParamType>& info) {
      // test suffix
      return info.param.get_slug();
    }
);

} // end namespace vt::tv::tests::unit
