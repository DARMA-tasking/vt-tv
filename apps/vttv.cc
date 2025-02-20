/*
//@HEADER
// *****************************************************************************
//
//                                   vttv.cc
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

#include <vt-tv/api/info.h>
#include <vt-tv/utility/json_reader.h>
#include <vt-tv/render/render.h>
#include <vt-tv/utility/parse_render.h>

#include <fmt-vt/format.h>
#include <CLI/CLI11.hpp>

int main(int argc, char** argv) {
  using namespace vt;
  using namespace tv;

  CLI::App app{"TV: Task Visualizer"};

  std::string default_config_file = "config/conf.yaml";
  app.add_option("-c,--conf", default_config_file, "Input configuration file")->required();

  CLI11_PARSE(app, argc, argv);

  std::string yaml_file = app.get_option("-c")->as<std::string>();
  std::filesystem::path config_file_path(yaml_file);

  // If it's a relative path, prepend the SRC_DIR
  if (config_file_path.is_relative()) {
    config_file_path = std::filesystem::path(SRC_DIR) / config_file_path;
  }
  yaml_file = config_file_path.string();

  fmt::print("Input configuration file={}\n", yaml_file);

  utility::ParseRender pr{yaml_file};
  pr.parseAndRender();


  return 0;
}
