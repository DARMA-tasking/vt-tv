/*
//@HEADER
// *****************************************************************************
//
//                             config_validator.cc
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
#include "config_validator.h"

namespace vt::tv::utility {

void ConfigValidator::validate(const YAML::Node& root) {
  if (!root || !root.IsMap()) throw ValidationError("Top-level YAML must be a map.");
  std::vector<std::string> path;
  validateNode(root, Config::root(), path);
}

std::string ConfigValidator::joinPath(const std::vector<std::string>& p) {
  if (p.empty()) return "<root>";
  std::string s;
  for (size_t i=0;i<p.size();++i){ if(i) s+='.'; s+=p[i]; }
  return s;
}

template <typename Scalar>
void ConfigValidator::ensureScalar(const YAML::Node& n, const std::vector<std::string>& p, const char* exp) {
  if (!n || !n.IsScalar()) typeErr(p, exp);
  try { (void)n.as<Scalar>(); } catch (...) { typeErr(p, exp); }
}

void ConfigValidator::validateNode(const YAML::Node& n, const Config::Rule& r, std::vector<std::string> p) {
  if (r.name && *r.name) p.push_back(r.name);
  switch (r.type) {
    case Config::KeyType::Map: {
      if (!n || !n.IsMap()) typeErr(p, "map");
      for (auto& c : r.children) {
        YAML::Node ch = n[c.name];
        if (c.required && !ch) missing(p, c.name);
        if (ch) validateNode(ch, c, p);
      }
    } break;
    case Config::KeyType::String: ensureScalar<std::string>(n, p, "string"); break;
    case Config::KeyType::Bool:   ensureScalar<bool>(n, p, "bool"); break;
    case Config::KeyType::UInt:   ensureScalar<uint64_t>(n, p, "non-negative integer"); break;
    case Config::KeyType::Float:  ensureScalar<double>(n, p, "float"); break;
  }
}

} /* end namespace vt::tv::utility */
