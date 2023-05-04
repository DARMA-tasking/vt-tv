/*
//@HEADER
// *****************************************************************************
//
//                                 render.h
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

#if !defined INCLUDED_VT_TV_RENDER_RENDER_H
#define INCLUDED_VT_TV_RENDER_RENDER_H

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
#include <vtkScalarBarActor.h>
#include <vtkTextProperty.h>
#include <vtkArrayCalculator.h>
#include <vtkThresholdPoints.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include <vtkRegularPolygonSource.h>
#include <vtkSphereSource.h>
#include <vtkBitArray.h>

#include "vt-tv/api/rank.h"
#include "vt-tv/api/info.h"

#include <fmt-vt/format.h>
#include <ostream>
#include <cmath>
#include <algorithm>
#include <iterator>
#include <cstdlib>
#include <tuple>
#include <limits>
#include <map>
#include <unordered_set>

namespace vt { namespace tv {


/**
 * \struct Render
 *
 * \brief Handler for visualisation
 */
struct Render {
private:
  enum ColorType {
    Default,
    BlueToRed,
    HotSpot,
    WhiteToBlack
  };

  std::unordered_map<PhaseType, PhaseWork> phase_info_;
  Info info_;
  TimeType object_load_max_;
  std::string object_qoi_ = "load";

  /**
   * \brief Decide object quantity storage type and compute it.
   *
   * \return void
   */
  void compute_object_load_range();

  /**
   * \brief get ranks belonging to phase
   *
   * \return set of ranks
   */
  std::unordered_set<NodeType> getRanks(PhaseType phase_in) const;

  /**
   * \brief Map ranks to polygonal mesh.
   *
   * \param[in] iteration phase index
   *
   * \return rank mesh
   */
  vtkPolyData* create_rank_mesh_(PhaseType iteration) const;

  /**
   * \brief Map objects to polygonal mesh.
   *
   * \param[in] phase phase
   *
   * \return object mesh
   */
  vtkPolyData* create_object_mesh_(PhaseWork phase) const;

  static vtkNew<vtkColorTransferFunction> createColorTransferFunction(
    double range[2], double avg_load = 0, ColorType ct = ColorType::Default
  );

  static vtkNew<vtkScalarBarActor> createScalarBarActor(
    vtkPolyDataMapper* mapper, std::string title, double x, double y
  );

  /**
   * \brief Map global index to its Cartesian grid coordinates.
   *
   * \param[in] flat_id the index of the entity in the phase
   * \param[in] grid_sizes number of entities per dimension x,y,z
   *
   * \return i,j,k Cartesian coordinates
   */
  static std::tuple<uint64_t, uint64_t, uint64_t> global_id_to_cartesian(
    uint64_t flat_id, std::tuple<uint64_t, uint64_t, uint64_t> grid_sizes
  );

public:
  /**
   * \brief Construct render
   *
   * \param[in] in_phase_info the phases
   * \param[in] in_info info about the ranks and phases
   */
  Render(std::unordered_map<PhaseType, PhaseWork> in_phase_info, Info in_info);

  static void createPipeline(
    vtkPoints* rank_points,
    vtkCellArray* rank_lines,
    vtkDoubleArray* qois,
    double qoi_range[2],
    vtkPolyData* object_mesh,
    double glyph_factor,
    double load_range[2],
    int phase,
    int iteration,
    double imbalance,
    int win_size
  );

  void generate(/*bool save_meshes, bool gen_vizqoi*/);
};

}} /* end namesapce vt::tv */

#endif /*INCLUDED_VT_TV_RENDER_RENDER_H*/
