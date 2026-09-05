/*
//@HEADER
// *****************************************************************************
//
//                              config_validator.h
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
#if !defined INCLUDED_VT_TV_UTILITY_CONFIG_VALIDATOR_H
#define INCLUDED_VT_TV_UTILITY_CONFIG_VALIDATOR_H

#include "config.h"

#include <yaml-cpp/yaml.h>

namespace vt::tv::utility {

/**
 * \struct ConfigValidator
 *
 * \brief Validate input yaml configuration file
 */
struct ConfigValidator {
  /**
   * \brief Validate configuration yaml
   *
   * \param[in] root the root node of the yaml
   */
  static void validate(const YAML::Node& root);

private:
  /**
   * \brief Join vector of path strings as a nicer string
   */
  static std::string joinPath(const std::vector<std::string>& p);

  static void missing(const std::vector<std::string>& p, const char* k) {
    throw ValidationError("Missing required key: '" + (joinPath(p) + "." + k) + "'.");
  }

  static void typeErr(const std::vector<std::string>& p, const char* exp) {
    throw ValidationError("Invalid type at '" + joinPath(p) + "': expected " + exp + ".");
  }

  template <typename Scalar>
  static void ensureScalar(const YAML::Node& n, const std::vector<std::string>& p, const char* exp);

  /**
   * \brief Validate node of configuration yaml recursively
   */
  static void validateNode(const YAML::Node& n, const Config::Rule& r, std::vector<std::string> p);
};


} /* end namespace vt::tv::utility */

#endif /*INCLUDED_VT_TV_UTILITY_CONFIG_VALIDATOR_H*/
