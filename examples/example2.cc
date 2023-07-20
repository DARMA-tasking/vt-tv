/*
//@HEADER
// *****************************************************************************
//
//                                example2.cc
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

#include <vtkActor.h>
#include <vtkRenderWindow.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkRenderer.h>
#include <vtkNamedColors.h>
#include <vtkNew.h>
#include <vtkProperty.h>
#include <vtkCamera.h>
#include <vtkPolyData.h>
#include <vtkPolyDataMapper.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkGlyphSource2D.h>
#include <vtkGlyph2D.h>
#include <vtkTransform.h>
#include <vtkTransformPolyDataFilter.h>
#include <vtkColorTransferFunction.h>
#include <vtkTextProperty.h>
#include <vtkArrayCalculator.h>
#include <vtkThresholdPoints.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>

#include <vt-tv/api/info.h>
#include <vt-tv/utility/json_reader.h>

#include <fmt-vt/format.h>

#include "../tests/unit/cmake_config.h"

#include <vt-tv/render/render.h>

#include <filesystem>

int main() {
  using namespace vt;
  using namespace tv;
  // Read JSON file and input data

  std::filesystem::path p = std::filesystem::path(SRC_DIR) / "tests/unit/lb_test_data/";
  std::string path = std::filesystem::absolute(p).string();

  uint64_t n_ranks = 4;

  std::unique_ptr<Info> info = std::make_unique<Info>();

  for (NodeType rank = 0; rank < n_ranks; rank++) {
    utility::JSONReader reader{rank, path + "/data." + std::to_string(rank) + ".json"};
    reader.readFile();
    auto tmpInfo = reader.parseFile();
    info->addInfo(tmpInfo->getObjectInfo(), tmpInfo->getRank(rank));
  }

  info->getPhaseObjects(0,4);
  fmt::print("===================\n");
  info->getAllObjects(4);

  fmt::print("===================\n");
  info->normalizeEdges(0,4);

  auto const& obj_info = info->getObjectInfo();

  fmt::print("Object info size={}\n", obj_info.size());
  fmt::print("Num ranks={}\n", info->getNumRanks());

  // for (auto const& [elm_id, oi] : obj_info) {
  //   fmt::print(
  //     "elm_id={:x}, home={}, migratable={}, index_array size={}\n",
  //     elm_id, oi.getHome(), oi.isMigratable(), oi.getIndexArray().size()
  //   );
  // }

  auto& rank_info = info->getRank(0);

  auto& phases = rank_info.getPhaseWork();

  for (auto const& [phase, phase_work] : phases) {
    // fmt::print("phase={}\n", phase);
    for (auto const& [elm_id, work] : phase_work.getObjectWork()) {
      // fmt::print("\t elm_id={:x}: load={}\n", elm_id, work.getLoad());
    }
  }

  // Instantiate render
  auto r = Render(phases, *info);
  r.generate();
  return 0;
}
