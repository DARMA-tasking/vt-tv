#include "tv.h"

namespace vt::tv::bindings::python {

void tv_from_json(const std::string& input_json_str, const std::string& input_yaml_params_str) {
  auto j = nlohmann::json::parse(input_json_str, const std::string& input_yaml_params_str);
  fmt::print("JSON: {}\n", j.dump(2));
  // Read the json file
  // using json = nlohmann::json;

  // assert(j != nullptr && "Must have valid json");

  // std::unordered_map<ElementIDType, ObjectInfo> object_info;
  // std::unordered_map<PhaseType, PhaseWork> phase_info;

  // auto phases = j["phases"];
  // if (phases.is_array()) {
  //   for (auto const& phase : phases) {
  //     auto id = phase["id"];
  //     auto tasks = phase["tasks"];

  //     std::unordered_map<ElementIDType, ObjectWork> objects;

  //     if (tasks.is_array()) {
  //       for (auto const& task : tasks) {
  //         auto node = task["node"];
  //         auto time = task["time"];
  //         auto etype = task["entity"]["type"];
  //         assert(time.is_number());
  //         assert(node.is_number());

  //         if (etype == "object") {
  //           auto object = task["entity"]["id"];
  //           auto home = task["entity"]["home"];
  //           bool migratable = task["entity"]["migratable"];
  //           assert(object.is_number());
  //           assert(home.is_number());

  //           std::vector<UniqueIndexBitType> index_arr;

  //           if (
  //             task["entity"].find("collection_id") != task["entity"].end() and
  //             task["entity"].find("index") != task["entity"].end()
  //           ) {
  //             auto cid = task["entity"]["collection_id"];
  //             auto idx = task["entity"]["index"];
  //             if (cid.is_number() && idx.is_array()) {
  //               std::vector<UniqueIndexBitType> arr = idx;
  //               index_arr = std::move(arr);
  //             }
  //           }

  //           ObjectInfo oi{object, home, migratable, std::move(index_arr)};

  //           if (task["entity"].find("collection_id") != task["entity"].end()) {
  //             oi.setIsCollection(true);
  //             oi.setMetaID(task["entity"]["collection_id"]);
  //           }

  //           if (task["entity"].find("objgroup_id") != task["entity"].end()) {
  //             oi.setIsObjGroup(true);
  //             oi.setMetaID(task["entity"]["objgroup_id"]);
  //           }

  //           object_info.try_emplace(object, std::move(oi));

  //           std::unordered_map<SubphaseType, TimeType> subphase_loads;

  //           if (task.find("subphases") != task.end()) {
  //             auto subphases = task["subphases"];
  //             if (subphases.is_array()) {
  //               for (auto const& s : subphases) {
  //                 auto sid = s["id"];
  //                 auto stime = s["time"];

  //                 assert(sid.is_number());
  //                 assert(stime.is_number());

  //                 subphase_loads[sid] = stime;
  //               }
  //             }
  //           }

  //           std::unordered_map<std::string, ObjectWork::VariantType> user_defined;
  //           if (task.find("user_defined") != task.end()) {
  //             auto user_defined = task["user_defined"];
  //             if (user_defined.is_object()) {
  //               for (auto& [key, value] : user_defined.items()) {
  //                 user_defined[key] = value;
  //               }
  //             }
  //           }
  //           // fmt::print(" Add object {}\n", (ElementIDType)object);
  //           objects.try_emplace(
  //             object,
  //             ObjectWork{
  //               object, time, std::move(subphase_loads), std::move(user_defined)
  //             }
  //           );
  //         }
  //       }
  //     }

  //     auto communications = phase["communications"];
  //     if (communications.is_array()) {
  //       for (auto const& comm : communications) {
  //         auto type = comm["type"];
  //         if (type == "SendRecv") {
  //           auto bytes = comm["bytes"];
  //           auto messages = comm["messages"];

  //           auto from = comm["from"];
  //           auto to = comm["to"];

  //           ElementIDType from_id = from["id"];
  //           ElementIDType to_id = to["id"];

  //           assert(bytes.is_number());
  //           assert(from.is_number());
  //           assert(to.is_number());

  //           // fmt::print(" From: {}, to: {}\n", from_id, to_id);
json_str
  //           // Object on this rank sent data
  //           if (objects.find(from_id) != objects.end()) {
  //             objects.at(from_id).addSentCommunications(to_id, bytes);
  //           } else if (objects.find(to_id) != objects.end()) {
  //             objects.at(to_id).addReceivedCommunications(from_id, bytes);
  //           }
  //         }
  //       }
  //     }
  //     phase_info.try_emplace(id, PhaseWork{id, std::move(objects)});
  //   }
  // }

  // Rank r{rank_, std::move(phase_info)};

  // std::unordered_map<NodeType, Rank> rank_info;
  // rank_info.try_emplace(rank_, std::move(r));
  // auto info = std::make_unique<Info>(std::move(object_info), std::move(rank_info));

  // // Render the data
  // Render render{std::move(info)};
  // render.generate(2000, 2000);
}

namespace nb = nanobind;
using namespace nb::literals;

NB_MODULE(vttv, m) {
  m.def("tv_from_json", &tv_from_json);
}

} /* end namespace vt::tv::bindings::python */
