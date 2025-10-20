/*
//@HEADER
// *****************************************************************************
//
//                                   config.h
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
#if !defined INCLUDED_VT_TV_UTILITY_CONFIG_H
#define INCLUDED_VT_TV_UTILITY_CONFIG_H

#include <stdexcept>
#include <optional>
#include <cstdint>
#include <vector>

namespace vt::tv::utility {


struct ValidationError : std::runtime_error { using std::runtime_error::runtime_error; };
struct SemanticError   : std::runtime_error { using std::runtime_error::runtime_error; };

/**
 * \struct Config
 *
 * \brief Defines schema of vttv input yaml files
 */
struct Config {
  enum class KeyType { Map, String, Bool, UInt, Float };
  struct Rule {
    const char* name; // key name ("" for root)
    KeyType           type;
    bool        required;
    std::vector<Rule> children; // only when type==Map
  };

  static const Rule& root() {
    static const Rule ROOT{
      "", KeyType::Map, true, {
        {"input", KeyType::Map, true, {
          {"directory",     KeyType::String, true, {}},
          {"n_ranks",       KeyType::UInt,   true, {}},
          {"file_stem",     KeyType::String, false, {}},
        }},
        {"viz", KeyType::Map, false, {
          {"x_ranks",       KeyType::UInt,   false, {}},
          {"y_ranks",       KeyType::UInt,   false, {}},
          {"z_ranks",       KeyType::UInt,   false, {}},
          {"object_jitter", KeyType::Float,  false, {}},
          {"rank_qoi",      KeyType::String, false, {}},
          {"object_qoi",    KeyType::String, false, {}},
          {"save_meshes",   KeyType::Bool,   false, {}},
          {"save_pngs",     KeyType::Bool,   false, {}},
          {"force_continuous_object_qoi", KeyType::Bool, false, {}},
        }},
        {"output", KeyType::Map, false, {
          {"directory",     KeyType::String, false, {}},
          {"file_stem",     KeyType::String, false, {}},
          {"window_size",   KeyType::UInt,   false, {}},
          {"font_size",     KeyType::UInt,   false, {}},
        }},
      }
    };
    return ROOT;
  }
};

} /* end namespace vt::tv::utility */

#endif /*INCLUDED_VT_TV_UTILITY_CONFIG_H*/