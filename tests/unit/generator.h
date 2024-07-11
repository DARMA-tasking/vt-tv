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

#include <yaml-cpp/yaml.h>

#include <vt-tv/api/types.h>
#include <vt-tv/api/rank.h>
#include <vt-tv/api/object_work.h>
#include <vt-tv/api/object_info.h>
#include <vt-tv/api/info.h>

#include <vt-tv/utility/json_reader.h>

#include <string>

namespace vt::tv::tests::unit {

/**
 * Testing Helper class that provide useful static methods to be used by the different
 * unit tests classes related to the api to generate some data in memory.
 */
class Generator {
    public:

        /**
         * Make a map of new objects
         */
        static const std::unordered_map<ElementIDType, ObjectWork> makeObjects(const int num_objects = rand() % 10, TimeType load = 2.0) {
            auto object_work_map = std::unordered_map<ElementIDType, ObjectWork>();
            for (auto object_id = 0; object_id < num_objects; object_id++) {
                // Make some ObjectWork instance
                auto subphase_loads = std::unordered_map<SubphaseType, TimeType>();
                auto user_defined = makeQOIVariants(5, "user_");
                auto attributes = makeQOIVariants(5, "attr_");
                auto object_work = ObjectWork(object_id, load, subphase_loads, user_defined, attributes);
                object_work_map.insert(std::make_pair(object_id, object_work));
            }
            return object_work_map;
        }

        /**
         * Make a new phase
         */
        static const PhaseWork makePhase(PhaseType phase_id, std::unordered_map<ElementIDType, ObjectWork> objects) {
            PhaseWork phase = PhaseWork(phase_id, objects);
            return phase;
        }

        /**
         * Make a map of new phases
         */
        static const std::unordered_map<PhaseType, PhaseWork> makePhases(std::unordered_map<ElementIDType, ObjectWork> objects, const int num_phases = rand() % 10) {
            auto phase_info_map = std::unordered_map<PhaseType, PhaseWork>();
            for (PhaseType phase_id = 0; phase_id < num_phases; phase_id++) {
                phase_info_map.insert(std::make_pair(phase_id, makePhase(phase_id, objects)));
            }
            return phase_info_map;
        }

        /**
         * Make a map of new ranks
         */
        static const std::unordered_map<NodeType, Rank> makeRanks(std::unordered_map<ElementIDType, ObjectWork> objects, int num_ranks = rand() % 10, int num_phases = rand() % 10) {
            auto rank_map = std::unordered_map<NodeType, Rank>();
            for (NodeType rank_id = 0; rank_id < num_ranks; rank_id++) {
                auto rank = Rank(rank_id, makePhases(objects, num_phases));
                rank_map.insert(std::make_pair(rank_id, rank));
            }
            return rank_map;
        }

        /**
         * Make a map object info from an object map
         */
        static std::unordered_map<ElementIDType, ObjectInfo> makeObjectInfoMap (
            const std::unordered_map<ElementIDType, ObjectWork> object_work_map
        ) {
            auto object_info_map = std::unordered_map<ElementIDType, ObjectInfo>();
            std::vector<size_t> idx;
            for (auto& it: object_work_map) {
                ObjectInfo object_info = ObjectInfo(it.first, 0, true, idx);
                object_info_map.insert(std::make_pair(it.first, object_info));
            }
            return object_info_map;
        }

        static std::unordered_map<std::string, QOIVariantTypes> makeQOIVariants (
            int num = rand() % 10,
            std::string key_prefix = "attr_",
            std::string value_suffix = "_value"
        ) {
            auto qoi_map = std::unordered_map<std::string, QOIVariantTypes>();
            for (size_t i=0; i<num; i++) {
                qoi_map[key_prefix + std::to_string(i)] = key_prefix + std::to_string(i) + value_suffix;
            }
            return qoi_map;
        }

        /**
         * Make an Info instance
         */
        static const Info makeInfo(int num_objects = rand() % 100, int num_ranks = rand() % 10, int num_phases = 10) {
            auto objects = makeObjects(num_objects);
            auto ranks = makeRanks(objects, num_ranks, num_phases);
            return Info(makeObjectInfoMap(objects), ranks);
        }

        static std::unique_ptr<Info> loadInfoFromConfig(YAML::Node config) {
            using JSONReader = ::vt::tv::utility::JSONReader;

            std::string input_dir = config["input"]["directory"].as<std::string>();
            std::filesystem::path input_path(input_dir);
            // If it's a relative path, prepend the SRC_DIR
            if (input_path.is_relative()) {
                input_path = std::filesystem::path(SRC_DIR) / input_path;
            }
            input_dir = input_path.string();

            int64_t n_ranks = config["input"]["n_ranks"].as<int64_t>();

            // append / to avoid problems with file stems
            if (!input_dir.empty() && input_dir.back() != '/') {
                input_dir += '/';
            }

            std::unique_ptr<Info> info = std::make_unique<Info>();
            for (int64_t rank = 0; rank < n_ranks; rank++) {
                fmt::print("Reading file for rank {}\n", rank);
                JSONReader reader{static_cast<NodeType>(rank)};
                reader.readFile(input_dir + "data." + std::to_string(rank) + ".json");
                auto tmpInfo = reader.parse();
                #ifdef VT_TV_OPENMP_ENABLED
                #if VT_TV_OPENMP_ENABLED
                    #pragma omp critical
                #endif
                #endif
                {
                info->addInfo(tmpInfo->getObjectInfo(), tmpInfo->getRank(rank));
                }
            }
            return info;
        }
};

}