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

#include <vt-tv/api/info.h>
#include <vt-tv/utility/json_reader.h>

#include <fmt-vt/format.h>

#include <string>

namespace vt::tv::tests::unit {

struct TestJSONReader : TestHarness { };

TEST_F(TestJSONReader, test_json_reader_1) {
  // @todo: fix this path
  std::string path = "/Users/jliffla/codes/vt/vt-tv/tests/unit/lb_test_data";

  NodeType rank = 0;
  utility::JSONReader reader{rank, path + "/data.0.json"};
  reader.readFile();
  auto info = reader.parseFile();

  auto const& obj_info = info->getObjectInfo();

  fmt::print("Object info size={}\n", obj_info.size());
  fmt::print("Num ranks={}\n", info->getNumRanks());

  EXPECT_EQ(info->getNumRanks(), 1);

  for (auto const& [elm_id, oi] : obj_info) {
    fmt::print(
      "elm_id={:x}, home={}, migratable={}, index_array size={}\n",
      elm_id, oi.getHome(), oi.isMigratable(), oi.getIndexArray().size()
    );
    EXPECT_EQ(elm_id, oi.getID());

    // for this dataset, no migrations happen so all objects should be on home
    EXPECT_EQ(oi.getHome(), rank);
  }

  auto& rank_info = info->getRank(rank);
  EXPECT_EQ(rank_info.getRankID(), rank);

  auto& phases = rank_info.getPhaseWork();

  // for this dataset, expect that all phases have the same objects
  std::set<ElementIDType> phase_0_objects;
  // and we should have a phase 0
  auto const& phase_0_object_work = phases.find(0)->second.getObjectWork();

  for (auto const& [elm_id, _] : phase_0_object_work) {
    phase_0_objects.insert(elm_id);
  }

  for (auto const& [phase, phase_work] : phases) {
    fmt::print("phase={}\n", phase);
    // sizes should be consistent
    EXPECT_EQ(phase_work.getObjectWork().size(), phase_0_objects.size());
    for (auto const& [elm_id, work] : phase_work.getObjectWork()) {
      // object should be found in phase 0
      EXPECT_TRUE(phase_0_objects.find(elm_id) != phase_0_objects.end());
      fmt::print("\t elm_id={:x}: load={}\n", elm_id, work.getLoad());
    }
  }
}

} // end namespace vt::tv::tests::unit
