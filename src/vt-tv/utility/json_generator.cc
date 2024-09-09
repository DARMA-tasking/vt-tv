/*
//@HEADER
// *****************************************************************************
//
//                              json_generator.cc
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

#include "vt-tv/utility/json_generator.h"

#include <nlohmann/json.hpp>

namespace vt::tv::utility {

std::unique_ptr<nlohmann::json> JSONGenerator::generateJSON() const {
  using json = nlohmann::json;

  auto const& rank_info = info_.getRank(rank_);
  auto const& phases = rank_info.getPhaseWork();
  assert(phases.find(phase_) != phases.end() and "Must have phase");
  auto const& phase_work = phases.at(phase_);
  auto const& object_work = phase_work.getObjectWork();

  json j;
  j["id"] = phase_;

  std::size_t task_index = 0;
  for (auto const& [elm_id, work] : object_work) {
    auto load = work.getLoad();
    auto const& subphase_loads = work.getSubphaseLoads();

    // assume resource is cpu for now (we have not added others yet)
    j["tasks"][task_index]["resource"] = "cpu";
    j["tasks"][task_index]["node"] = rank_;
    j["tasks"][task_index]["time"] = load;

    outputObjectMetaData(j["tasks"][task_index]["entity"], elm_id);

    if (subphase_loads.size() > 0) {
      for (std::size_t i = 0; i < subphase_loads.size(); i++) {
        j["tasks"][task_index]["subphases"][i]["id"] = i;
        j["tasks"][task_index]["subphases"][i]["time"] = subphase_loads.at(i);
      }
    }

    auto const& user_defined = work.getUserDefined();
    for (auto const& [key, val] : user_defined) {
      // can't capture structured binding in C++17 (wait for 20!)
      auto const& key2 = key;
      std::visit(
        [&](auto&& arg) { j["tasks"][task_index]["user_defined"][key2] = arg; },
        val);
    }

    // @todo: add communications
  }

  return std::make_unique<json>(std::move(j));
}

void JSONGenerator::outputObjectMetaData(
  nlohmann::json& j, ElementIDType id) const {
  auto const& object_info_map = info_.getObjectInfo();

  assert(
    object_info_map.find(id) != object_info_map.end() && "Object must exist");
  auto const& object_info = object_info_map.find(id)->second;

  j["type"] = "object";
  j["id"] = id;
  j["home"] = object_info.getHome();
  j["migratable"] = object_info.isMigratable();

  auto const& idx_array = object_info.getIndexArray();
  if (idx_array.size() > 0) {
    for (std::size_t i = 0; i < idx_array.size(); i++) {
      j["index"][i] = idx_array[i];
    }
  }

  if (object_info.getIsCollection()) {
    j["collection_id"] = object_info.getMetaID();
  } else if (object_info.getIsObjGroup()) {
    j["objgroup_id"] = object_info.getMetaID();
  }
}

} /* end namespace vt::tv::utility */
