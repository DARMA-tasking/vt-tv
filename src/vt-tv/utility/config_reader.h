#if !defined INCLUDED_VT_TV_UTILITY_CONFIG_READER_H
#define INCLUDED_VT_TV_UTILITY_CONFIG_READER_H

#include "config_validator.h"

#include <fmt-vt/format.h>

namespace vt::tv::utility {

struct ConfigReader {
  // Raw values (required + optional)
  struct Input {
    std::string                directory;
    uint64_t                   n_ranks{};
    std::optional<std::string> file_stem;
  };
  struct Viz {
    std::optional<uint64_t>      x_ranks, y_ranks;
    std::optional<double>        object_jitter;
    std::optional<std::string>   rank_qoi, object_qoi;
    std::optional<bool>          save_meshes, save_pngs, force_continuous_object_qoi;
  };
  struct Output {
    std::optional<std::string> directory, file_stem;
    std::optional<uint64_t>    window_size, font_size;
  };

  // Parsed raw sections
  Input  input;
  Viz    viz;
  Output output;

  // Derived grid
  struct Grid { uint64_t x{1}, y{1}; } grid;

  /**
   * \brief Read, validate and parse configuration yaml file
   *
   * \param[in] filename the filename of the yaml configuration
   */
  static ConfigReader from_file(const std::string& filename);

private:
  /**
   * \brief Read and save variables from yaml configuration
   *
   * \param[in] root the root node of the yaml configuration
   */
  static ConfigReader parse(const YAML::Node& root);

  /**
   * \brief Compute visual grid for ranks
   */
  void compute_grid();
};

} /* end namespace vt::tv::utility */

#endif /*INCLUDED_VT_TV_UTILITY_CONFIG_READER_H*/
