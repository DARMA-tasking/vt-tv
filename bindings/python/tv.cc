#include "tv.h"

namespace vt::tv::bindings::python {

void tvFromJson(const std::vector<std::string>& input_json_per_rank_list, const std::string& input_yaml_params_str, uint64_t num_ranks) {
  std::string startup_logo = std::string("        __           __\n")
                           + std::string(" _   __/ /_         / /__   __\n")
                           + std::string("| | / / __/ _____  / __/ | / /\n")
                           + std::string("| |/ / /   /____/ / /_ | |/ /\n")
                           + std::string("|___/\\__/         \\__/ |___/\n");
  fmt::print("==============================\n");
  fmt::print(startup_logo);
  fmt::print("==============================\n");

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
    fmt::print("Input Configuration Parameters:\n");
    fmt::print("  x_ranks: {}\n", grid_size[0]);
    fmt::print("  y_ranks: {}\n", grid_size[1]);
    fmt::print("  z_ranks: {}\n", grid_size[2]);
    fmt::print("  object_jitter: {}\n", object_jitter);
    fmt::print("  rank_qoi: {}\n", qoi_request[0]);
    fmt::print("  object_qoi: {}\n", qoi_request[2]);
    fmt::print("  save_meshes: {}\n", save_meshes);
    fmt::print("  save_pngs: {}\n", save_pngs);
    fmt::print("  force_continuous_object_qoi: {}\n", continuous_object_qoi);
    fmt::print("  output_visualization_dir: {}\n", output_dir);
    fmt::print("  output_visualization_file_stem: {}\n", output_file_stem);
    fmt::print("  window_size: {}\n", win_size);
    fmt::print("  font_size: {}\n", font_size);

    using json = nlohmann::json;

    assert(input_json_per_rank_list.size() == num_ranks && "Must have the same number of json files as ranks");

    // Initialize the info object, that will hold data for all ranks for all phases
    std::unique_ptr<Info> info = std::make_unique<Info>();

    #ifdef VT_TV_N_THREADS
      const int threads = VT_TV_N_THREADS;
    #else
      const int threads = 2;
    #endif
    #ifdef VT_TV_OPENMP_ENABLED
    #if VT_TV_OPENMP_ENABLED
      omp_set_num_threads(threads);
      // print number of threads
      fmt::print("vt-tv: Using {} threads\n", threads);
      # pragma omp parallel for
    #endif
    #endif
    for (int64_t rank_id = 0; rank_id < num_ranks; rank_id++) {
      fmt::print("Reading file for rank {}\n", rank_id);
      std::string rank_json_str = input_json_per_rank_list[rank_id];
      utility::JSONReader reader{static_cast<NodeType>(rank_id)};
      reader.readString(rank_json_str);
      auto tmpInfo = reader.parse();
      #ifdef VT_TV_OPENMP_ENABLED
      #if VT_TV_OPENMP_ENABLED
        #pragma omp critical
      #endif
      #endif
      {
        info->addInfo(tmpInfo->getObjectInfo(), tmpInfo->getRank(rank_id));
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
    exit(1);
  }

  fmt::print("vt-tv: Done.\n");
}

namespace nb = nanobind;
using namespace nb::literals;

NB_MODULE(vttv, m) {
  m.def("tvFromJson", &tvFromJson);
}

} /* end namespace vt::tv::bindings::python */
