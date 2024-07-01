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

struct TestInfo : TestHarness { };

/* 
  Common Assertions: EXPECT_EQ(a, b), EXPECT_TRUE(condition)
  Common print: fmt::print(
      format, var1, var2 etc.
    );
 */

TEST_F(TestInfo, test_info_get_num_ranks_empty) {
  std::unique_ptr<Info> info = std::make_unique<Info>();  
  EXPECT_EQ(info->getNumRanks(), 0);
}

TEST_F(TestInfo, test_info_get_num_ranks) {
  // Initialize a info object, that will hold data for all ranks for all phases
  auto num_ranks = 5;
  std::unordered_map<NodeType, Rank> in_ranks = std::unordered_map<NodeType, Rank>();
  for (NodeType rank_id = 0; rank_id < num_ranks; rank_id++) {
    auto rank = Rank();
    in_ranks.insert(std::make_pair(rank_id, rank));
  }

  std::unordered_map<ElementIDType, ObjectInfo> in_object_info;
  std::unique_ptr<Info> info = std::make_unique<Info>(Info(in_object_info, in_ranks));

  EXPECT_EQ(info->getNumRanks(), num_ranks);
}

} // end namespace vt::tv::tests::unit
