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
#include "vt-tv/api/info.h"
#include <string>

namespace vt::tv::tests::unit::api {

/**
 * Helper class that provide useful static methods to be used by the different unit tests classes related to the api.
 */
class Helper {
    public:  

        /**
         * Make a map of new objects
         */
        static const std::unordered_map<ElementIDType, ObjectWork> make_objects(const int num_objects = rand() % 10) {
            auto object_work_map = std::unordered_map<ElementIDType, ObjectWork>();
            for (auto object_id = 0; object_id < num_objects; object_id++) {
                // Make some ObjectWork instance
                TimeType whole_phase_load = 2.0;
                auto subphase_loads = std::unordered_map<SubphaseType, TimeType>();
                auto user_defined = std::unordered_map<std::string, QOIVariantTypes>();
                auto attributes = std::unordered_map<std::string, QOIVariantTypes>();
                auto object_work = ObjectWork(object_id, whole_phase_load, subphase_loads, user_defined, attributes);
                object_work_map.insert(std::make_pair(object_id, object_work));
            }
            return object_work_map;    
        }

        /**
         * Make a new phase
         */
        static const PhaseWork make_phase(PhaseType phase_id, std::unordered_map<ElementIDType, ObjectWork> objects) {
            PhaseWork phase = PhaseWork(0, objects);
            return phase;
        }

        /**
         * Make a map of new phases
         */
        static const std::unordered_map<PhaseType, PhaseWork> make_phases(std::unordered_map<ElementIDType, ObjectWork> objects, const int num_phases = rand() % 10) {
            auto phase_info_map = std::unordered_map<PhaseType, PhaseWork>();
            for (PhaseType phase_id = 0; phase_id < num_phases; phase_id++) {
                phase_info_map.insert(std::make_pair(phase_id, make_phase(phase_id, objects)));
            }
            return phase_info_map;
        }

        /**
         * Make a map of new ranks
         */
        static const std::unordered_map<NodeType, Rank> make_ranks(std::unordered_map<ElementIDType, ObjectWork> objects, int num_ranks = rand() % 10, int num_phases = rand() % 10) {
            auto rank_map = std::unordered_map<NodeType, Rank>();      
            for (NodeType rank_id = 0; rank_id < num_ranks; rank_id++) {
                auto rank = Rank(rank_id, make_phases(objects, num_phases));
                rank_map.insert(std::make_pair(rank_id, rank));
            }
            return rank_map;
        }

        /**
         * Make a map object info from an object map
         */
        static std::unordered_map<ElementIDType, ObjectInfo> make_object_info_map (const std::unordered_map<ElementIDType, ObjectWork> object_work_map) {
            auto object_info_map = std::unordered_map<ElementIDType, ObjectInfo>();
            std::vector<size_t> idx;
            for (auto& it: object_work_map) {
                ObjectInfo object_info = ObjectInfo(it.first, 0, true, idx);
                object_info_map.insert(std::make_pair(it.first, object_info));
            }
            return object_info_map;

        }

        /**
         * Make an Info instance
         */
        static const Info info(int num_objects = rand() % 100, int num_ranks = rand() % 10, int num_phases = 10) {  
            auto objects = make_objects(num_objects);
            auto ranks = make_ranks(objects, num_ranks, num_phases);
            return Info(make_object_info_map(objects), ranks);
        }
};

}