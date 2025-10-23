/*
//@HEADER
// *****************************************************************************
//
//                               config_reader.h
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
