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

#include "vt-tv/api/types.h"
#include "vt-tv/api/rank.h"
#include "vt-tv/api/object_work.h"
#include "vt-tv/api/object_info.h"
#include <string>

namespace vt::tv::tests::unit::api {

struct Sample {
  public:
    Sample(
      std::unordered_map<NodeType, Rank> &in_ranks,
      std::unordered_map<ElementIDType, ObjectWork> &in_object_work_map,
      std::unordered_map<ElementIDType, ObjectInfo> &in_object_info_map
    ): ranks(in_ranks), object_work_map(in_object_work_map), object_info_map(in_object_info_map) {

    }

    std::string get_slug() const
    {
      return "sample_" + std::to_string(ranks.size()) + "_ranks_" + std::to_string(object_info_map.size()) + "_objects";
    }

    std::unordered_map<NodeType, Rank> ranks;
    std::unordered_map<ElementIDType, ObjectWork> object_work_map;
    std::unordered_map<ElementIDType, ObjectInfo> object_info_map;
};

/**
 * Represents a sample set of data for the tests Info class tests
 */
class SampleFactory {

  public:

  /**
   * Generates sample data using a single phase and a default phase load
   */
  static const Sample create_one_phase_sample(int num_ranks, int num_objects_per_rank) {
    std::unordered_map<NodeType, Rank> ranks = std::unordered_map<NodeType, Rank>();
    std::unordered_map<ElementIDType, ObjectWork> object_work_map = std::unordered_map<ElementIDType, ObjectWork>();
    std::unordered_map<ElementIDType, ObjectInfo> object_info_map = std::unordered_map<ElementIDType, ObjectInfo>();
    std::vector<size_t> in_index;

    for (NodeType rank_id = 0; rank_id < num_ranks; rank_id++) {
      // Add some objects
      for (auto object_index = 0; object_index < num_objects_per_rank; object_index++) {
        auto object_id = rank_id * num_objects_per_rank + object_index;
        auto phase_load = 2.0;

        // object work
        std::unordered_map<SubphaseType, TimeType> sub_phase_loads = std::unordered_map<SubphaseType, TimeType>();
        auto object_work = ObjectWork(object_id, phase_load, sub_phase_loads);
        object_work_map.insert(std::make_pair(object_id, object_work));

        // object info
        ObjectInfo object_info = ObjectInfo(object_id, rank_id, true, in_index);
        object_info_map.insert(std::make_pair(object_id, object_info));
      }
    }

    // Add a phase (id 0)
    std::unordered_map<PhaseType, PhaseWork> phase_info = std::unordered_map<PhaseType, PhaseWork>();
    auto phase = PhaseWork(0, object_work_map);
    phase_info.insert(std::make_pair(0, phase));

    // Add some ranks
    for (NodeType rank_id = 0; rank_id < num_ranks; rank_id++) {
      auto rank = Rank(rank_id, phase_info);
      ranks.insert(std::make_pair(rank_id, rank));
    }

    // Return sample data
    auto sample = Sample(ranks, object_work_map, object_info_map);

    return sample;
  }
};

} // end namespace vt::tv::tests::unit
