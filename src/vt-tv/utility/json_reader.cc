/*
//@HEADER
// *****************************************************************************
//
//                              json_reader.cc
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
#include "vt-tv/utility/json_reader.h"
#include "vt-tv/utility/decompression_input_container.h"
#include "vt-tv/utility/input_iterator.h"
#include "vt-tv/utility/qoi_serializer.h"

#include <nlohmann/json.hpp>
#include <fmt-vt/core.h>
#include <fmt-vt/format.h>

#include <fstream>

namespace vt::tv::utility {

bool JSONReader::isCompressed(std::string const& in_filename) const {
  bool compressed = true;

  // determine if the file is compressed or not
  std::ifstream is(in_filename);
  if (not is.good()) {
    auto str = fmt::format("Filename is not valid: {}", in_filename);
    fmt::print(str);
    assert(false && "Invalid file");
    return false;
  }
  char f = '\0';
  while (is.good()) {
    f = is.get();
    if (f == ' ' or f == '\t' or f == '\n') {
      continue;
    } else {
      break;
    }
  }
  if (f == '{') {
    compressed = false;
  }
  is.close();

  return compressed;
}

void JSONReader::readFile(std::string const& in_filename) {
  using json = nlohmann::json;

  if (isCompressed(in_filename)) {
    DecompressionInputContainer c(in_filename);
    json j = json::parse(c);
    json_ = std::make_unique<json>(std::move(j));
  } else {
    std::ifstream is(in_filename, std::ios::binary);
    assert(is.good() && "File must be good");
    json j = json::parse(is);
    is.close();
    json_ = std::make_unique<json>(std::move(j));
  }
}

void JSONReader::readString(std::string const& in_json_string) {
  using json = nlohmann::json;
  json j = json::parse(in_json_string);
  json_ = std::make_unique<json>(std::move(j));
}

std::unique_ptr<Info> JSONReader::parse() {
  using json = nlohmann::json;

  assert(json_ != nullptr && "Must have valid json");

  std::unordered_map<ElementIDType, ObjectInfo> object_info;
  std::unordered_map<PhaseType, PhaseWork> phase_info;

  json j = std::move(*json_);

  auto phases = j["phases"];
  if (phases.is_array()) {
    for (auto const& phase : phases) {
      auto id = phase["id"];
      auto tasks = phase["tasks"];

      std::unordered_map<ElementIDType, ObjectWork> objects;

      if (tasks.is_array()) {
        for (auto const& task : tasks) {
          auto node = task["node"];
          auto time = task["time"];
          auto etype = task["entity"]["type"];
          assert(time.is_number());
          assert(node.is_number());

          if (etype == "object") {
            auto object = task["entity"]["id"];
            auto home = task["entity"]["home"];
            bool migratable = task["entity"]["migratable"];
            assert(object.is_number());
            assert(home.is_number());

            std::vector<UniqueIndexBitType> index_arr;

            if (
              task["entity"].find("collection_id") != task["entity"].end() and
              task["entity"].find("index") != task["entity"].end()
            ) {
              auto cid = task["entity"]["collection_id"];
              auto idx = task["entity"]["index"];
              if (cid.is_number() && idx.is_array()) {
                std::vector<UniqueIndexBitType> arr = idx;
                index_arr = std::move(arr);
              }
            }

            ObjectInfo oi{object, home, migratable, std::move(index_arr)};

            if (task["entity"].find("collection_id") != task["entity"].end()) {
              oi.setIsCollection(true);
              oi.setMetaID(task["entity"]["collection_id"]);
            }

            if (task["entity"].find("objgroup_id") != task["entity"].end()) {
              oi.setIsObjGroup(true);
              oi.setMetaID(task["entity"]["objgroup_id"]);
            }

            object_info.try_emplace(object, std::move(oi));

            std::unordered_map<SubphaseType, TimeType> subphase_loads;

            if (task.find("subphases") != task.end()) {
              auto subphases = task["subphases"];
              if (subphases.is_array()) {
                for (auto const& s : subphases) {
                  auto sid = s["id"];
                  auto stime = s["time"];

                  assert(sid.is_number());
                  assert(stime.is_number());

                  subphase_loads[sid] = stime;
                }
              }
            }

            std::unordered_map<std::string, QOIVariantTypes> readed_user_defined;
            if (task.find("user_defined") != task.end()) {
              auto user_defined = task["user_defined"];
              if (user_defined.is_object()) {
                for (auto& [key, value] : user_defined.items()) {
                  readed_user_defined[key] = value;
                }
              }
            }

            std::unordered_map<std::string, QOIVariantTypes> readed_attributes;
            if (task.find("attributes") != task.end()) {
              auto attributes = task["attributes"];
              if (attributes.is_object()) {
                for (auto& [key, value] : attributes.items()) {
                  readed_attributes[key] = value;
                }
              }
            }

            // fmt::print(" Add object {}\n", (ElementIDType)object);
            objects.try_emplace(
              object,
              ObjectWork{
                object, time, std::move(subphase_loads), std::move(readed_user_defined), std::move(readed_attributes)
              }
            );
          }
        }
      }

      auto communications = phase["communications"];
      if (communications.is_array()) {
        for (auto const& comm : communications) {
          auto type = comm["type"];
          if (type == "SendRecv") {
            auto bytes = comm["bytes"];
            auto messages = comm["messages"];

            auto from = comm["from"];
            auto to = comm["to"];

            ElementIDType from_id = from["id"];
            ElementIDType to_id = to["id"];

            assert(bytes.is_number());
            // assert(from.is_number());
            // assert(to.is_number());

            // fmt::print(" From: {}, to: {}\n", from_id, to_id);

            // Object on this rank sent data
            auto from_it = objects.find(from_id);
            if (from_it != objects.end()) {
              from_it->second.addSentCommunications(to_id, bytes);
            } else {
              auto to_it = objects.find(to_id);
              if (to_it != objects.end()) {
                to_it->second.addReceivedCommunications(from_id, bytes);
              } else {
                fmt::print("Warning: Communication {} -> {}: neither sender nor recipient was found in objects.\n", from_id, to_id);
              }
            }
          }
        }
      }
      phase_info.try_emplace(id, PhaseWork{id, std::move(objects)});
    }
  }

  std::unordered_map<std::string, QOIVariantTypes> readed_metadata;
  if (j.find("metadata") != j.end()) {
    auto metadata = j["metadata"];
    if (metadata.find("attributes") != metadata.end()) {
      auto attributes = metadata["attributes"];
      if (attributes.is_object()) {
        for (auto& [key, value] : attributes.items()) {
          readed_metadata[key] = value;
        }
      }
    }
  }

  Rank r{rank_, std::move(phase_info), std::move(readed_metadata)};

  std::unordered_map<NodeType, Rank> rank_info;
  rank_info.try_emplace(rank_, std::move(r));

  return std::make_unique<Info>(std::move(object_info), std::move(rank_info));
}

} /* end namespace vt::tv::utility */
