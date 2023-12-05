#include "tv.h"

namespace vt::tv::bindings::python {

void tv_from_json(const std::vector<std::string>& input_json_per_rank_list, const std::string& input_yaml_params_str, uint64_t num_ranks) {

  // parse the input yaml parameters
  try {
    // Load the configuration from serialized YAML
    YAML::Node viz_config = YAML::Load(input_yaml_params_str);


    std::array<std::string, 3> qoi_request = {
      viz_config["rank_qoi"].as<std::string>(),
      "",
      viz_config["object_qoi"].as<std::string>()
    };

    bool save_meshes = viz_config["save_meshes"].as<bool>();
    bool save_pngs = true; // lbaf always saves pngs
    bool continuous_object_qoi = viz_config["force_continuous_object_qoi"].as<bool>();

    std::array<uint64_t, 3> grid_size = {
      viz_config["x_ranks"].as<uint64_t>(),
      viz_config["y_ranks"].as<uint64_t>(),
      viz_config["z_ranks"].as<uint64_t>()
    };

    double object_jitter = viz_config["object_jitter"].as<double>();

    std::string output_dir = viz_config["output_visualization_dir"].as<std::string>();
    std::filesystem::path output_path(output_dir);

    // Throw an error if the output directory does not exist or is not absolute
    if (!std::filesystem::exists(output_path)) {
      throw std::runtime_error("Visualization output directory does not exist.");
    }
    if (!output_path.is_absolute()) {
      throw std::runtime_error("Visualization output directory must be absolute.");
    }

    // append / to avoid problems with file stems
    if (!output_dir.empty() && output_dir.back() != '/') {
      output_dir += '/';
    }

    std::string output_file_stem = viz_config["output_visualization_file_stem"].as<std::string>();

    uint64_t win_size = 2000;
    if (viz_config["window_size"]) {
      win_size = viz_config["window_size"].as<uint64_t>();
    }

    // Use automatic font size if not defined by user
    // 0.025 is the factor of the window size determined to be ideal for the font size
    uint64_t font_size = 0.025 * win_size;
    if (viz_config["font_size"]) {
      font_size = viz_config["font_size"].as<uint64_t>();
    }

    // print all saved configuration parameters
    fmt::print("vt-tv:\n Parameters:\n");
    fmt::print("   x_ranks: {}\n", grid_size[0]);
    fmt::print("   y_ranks: {}\n", grid_size[1]);
    fmt::print("   z_ranks: {}\n", grid_size[2]);
    fmt::print("   object_jitter: {}\n", object_jitter);
    fmt::print("   rank_qoi: {}\n", qoi_request[0]);
    fmt::print("   object_qoi: {}\n", qoi_request[2]);
    fmt::print("   save_meshes: {}\n", save_meshes);
    fmt::print("   save_pngs: {}\n", save_pngs);
    fmt::print("   force_continuous_object_qoi: {}\n", continuous_object_qoi);
    fmt::print("   output_visualization_dir: {}\n", output_dir);
    fmt::print("   output_visualization_file_stem: {}\n", output_file_stem);
    fmt::print("   window_size: {}\n", win_size);
    fmt::print("   font_size: {}\n", font_size);

    using json = nlohmann::json;
    // Read the json for the rank
    std::unique_ptr<Info> info = std::make_unique<Info>();

    assert(input_json_per_rank_list.size() == num_ranks && "Must have the same number of json files as ranks");

    for (NodeType rank_id = 0; rank_id < num_ranks; rank_id++) {
      std::string rank_json_str = input_json_per_rank_list[rank_id];
    try {
      std::cerr << "vt-tv: Parsing JSON for rank " << rank_id << "\n";
      auto j = json::parse(rank_json_str);
      assert(j != nullptr && "Must have valid json");

      std::cerr << "vt-tv: Writing JSON to disk for rank " << rank_id << "\n";
      std::string filename = fmt::format("/home/pierrelp/Develop/NGA/vt-tv/output_{}.json", rank_id);
      std::ofstream o(filename);
      o << std::setw(2) << j << std::endl;
      o.close();

      std::unordered_map<ElementIDType, ObjectInfo> object_info;
      std::unordered_map<PhaseType, PhaseWork> phase_info;

      std::cerr << "vt-tv: Reading rank " << rank_id << "\n";

      auto phases = j["phases"];

      if (phases.is_array()) {
        for (auto const& phase : phases) {
          auto id = phase["id"];
          std::cerr << "vt-tv: Reading phase " << id << "\n";
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

                std::cerr << "vt-tv: Processing object " << object << " in phase " << id << "\n";
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

                  std::unordered_map<std::string, ObjectWork::VariantType> user_defined;
                  if (task.find("user_defined") != task.end()) {
                    auto user_defined = task["user_defined"];
                    if (user_defined.is_object()) {
                      for (auto& [key, value] : user_defined.items()) {
                        user_defined[key] = value;
                      }
                    }
                  }
                  // fmt::print(" Add object {}\n", (ElementIDType)object);
                  objects.try_emplace(
                    object,
                    ObjectWork{
                      object, time, std::move(subphase_loads), std::move(user_defined)
                    }
                  );
                }
              }
            }

            if (phase.find("communications") != phase.end()) {
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
                    assert(from.is_number());
                    assert(to.is_number());

                    // fmt::print(" From: {}, to: {}\n", from_id, to_id);
                    // Object on this rank sent data
                    if (objects.find(from_id) != objects.end()) {
                      objects.at(from_id).addSentCommunications(to_id, bytes);
                    } else if (objects.find(to_id) != objects.end()) {
                      objects.at(to_id).addReceivedCommunications(from_id, bytes);
                    }
                  }
                }
              }
            }
            phase_info.try_emplace(id, PhaseWork{id, std::move(objects)});
          }
        }
        fmt::print(" vt-tv: Adding rank {}\n", rank_id);
        Rank r{rank_id, std::move(phase_info)};

        info->addInfo(std::move(object_info), std::move(r));

      }
      catch(const std::exception& e)
      {
        std::cerr << "vt-tv: Error reading data for rank " << rank_id << ": " << e.what() << '\n';
      }
    }
    // Instantiate render
    Render render(
      qoi_request, continuous_object_qoi, *info, grid_size, object_jitter,
      output_dir, output_file_stem, 1.0, save_meshes, save_pngs, std::numeric_limits<PhaseType>::max()
    );
    render.generate(font_size, win_size);
  } catch (std::exception const& e) {
    std::cout << "vt-tv: Error reading the configuration file: " << e.what() << std::endl;
  }

  fmt::print("vt-tv: Done.\n");
}

namespace nb = nanobind;
using namespace nb::literals;

NB_MODULE(vttv, m) {
  m.def("tv_from_json", &tv_from_json);
}

} /* end namespace vt::tv::bindings::python */
