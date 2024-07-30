/*
//@HEADER
// *****************************************************************************
//
//                                   render.h
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
#include <vtkTextActor.h>
#include <vtkTextProperty.h>
#include <vtkArrayCalculator.h>
#include <vtkThresholdPoints.h>
#include <vtkWindowToImageFilter.h>
#include <vtkPNGWriter.h>
#include <vtkRegularPolygonSource.h>
#include <vtkSphereSource.h>
#include <vtkBitArray.h>
#include <vtkLine.h>
#include <vtkCellData.h>
#include <vtkLookupTable.h>
#include <vtkDiscretizableColorTransferFunction.h>

#include <vtkPolyDataWriter.h>
#include <vtkExodusIIWriter.h>
#include <vtkXMLPolyDataWriter.h>

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
#include <set>
#include <array>
#include <variant>
#include <cmath>
#include <regex>

namespace vt { namespace tv {

/**
 * \struct Render
 *
 * \brief Handler for visualization
 */
struct Render {
private:
  enum ColorType {
    Default,
    BlueToRed,
    HotSpot,
    WhiteToBlack
  };

  // quantities of interest
  std::string rank_qoi_ = "load";
  std::string object_qoi_ = "load";
  bool continuous_object_qoi_;

  // Render input data
  Info info_;
  uint64_t n_ranks_ = 0;
  uint64_t n_phases_ = 0;
  std::array<uint64_t, 3> grid_size_ = {1, 1, 1};
  double object_jitter_ = 0.5;
  std::set<uint64_t> rank_dims_;
  std::string output_dir_;
  std::string output_file_stem_;
  double grid_resolution_ = 1.0;
  bool save_meshes_ = false;
  bool save_pngs_ = false;
  PhaseType selected_phase_ = std::numeric_limits<PhaseType>::max();

  // numeric parameters
  std::variant<std::pair<double, double>, std::set<std::variant<double,int>>> object_qoi_range_;
  std::pair<double, double> rank_qoi_range_;

  // Maximum object attribute values
  uint64_t max_o_per_dim_ = 0;
  double object_qoi_max_ = 0.0;
  double object_volume_max_ = 0.0;
  double object_load_max_ = 0.0;

  // Jitter per object
  std::unordered_map<ElementIDType, std::array<double, 3>> jitter_dims_;

  /**
   * \brief Function to initialize object dimensions using some randomness
   */
  void initJitterDims();

  /**
   * \brief Compute maximum value of object volumes.
   *
   * \return max object volume
   */
  double computeMaxObjectVolume_();

  /**
   * \brief Compute range of object qoi.
   *
   * \return object qoi range
   */
  std::variant<std::pair<double, double>, std::set<std::variant<double,int>>> computeObjectQoiRange_();

  /**
   * \brief Compute range of rank qoi.
   *
   * \return rank qoi range
   */
  std::pair<double, double> computeRankQoiRange_();

  /**
   * \brief Compute average of rank qoi.
   *
   * @param qoi quantity of interest on which to compute average
   *
   * \return rank qoi range
   */
  double computeRankQoiAverage_(PhaseType phase, std::string qoi);

  /**
   * \brief Create mapping of objects in ranks
   *
   * \param[in] phase phase index
   *
   * \return mapping
   */
  std::map<NodeType, std::unordered_map<ElementIDType, ObjectWork>> createObjectMapping_(PhaseType phase);

  /**
   * \brief Map ranks to polygonal mesh.
   *
   * \param[in] iteration phase index
   *
   * \return rank mesh
   */
  vtkNew<vtkPolyData> createRankMesh_(PhaseType iteration);

  /**
   * \brief Map objects to polygonal mesh.
   *
   * \param[in] phase phase
   *
   * \return object mesh
   */
  vtkNew<vtkPolyData> createObjectMesh_(PhaseType phase);

  static void getRgbFromTab20Colormap_(int index, double& r, double& g, double& b);

  static vtkSmartPointer<vtkDiscretizableColorTransferFunction> createColorTransferFunction_(
    std::variant<std::pair<double, double>, std::set<std::variant<double,int>>> attribute_range, ColorType ct = ColorType::Default
  );

  static vtkSmartPointer<vtkScalarBarActor> createScalarBarActor_(
    vtkSmartPointer<vtkMapper> mapper,
    const std::string& title,
    double x, double y,
    uint64_t font_size = 50,
    std::set<std::variant<double,int>> values = {}
  );

  static vtkSmartPointer<vtkRenderer> setupRenderer_();

  static vtkSmartPointer<vtkMapper> createRanksMapper_(
    vtkPolyData* rank_mesh,
    std::variant<std::pair<double, double>, std::set<std::variant<double,int>>> rank_qoi_range
  );

  /**
   * \brief Map global index to its Cartesian grid coordinates.
   *
   * \param[in] flat_id the index of the entity in the phase
   * \param[in] grid_sizes number of entities per dimension x,y,z
   *
   * \return i,j,k Cartesian coordinates
   */
  static std::array<uint64_t, 3> globalIDToCartesian_(
    uint64_t flat_id, std::array<uint64_t, 3> grid_sizes
  );

public:
  /**
   * \brief Construct render
   *
   * \param[in] in_info info about the ranks and phases
   */
  Render(Info in_info);

  /**
   * \brief Construct render
   *
   * \param[in] in_qoi_request description of rank and object quantities of interest
   * \param[in] in_continuous_object_qoi always treat object QOI as continuous or not
   * \param[in] in_info general info
   * \param[in] in_grid_size triplet containing grid sizes in each dimension
   * \param[in] in_object_jitter coefficient of random jitter with magnitude < 1
   * \param[in] in_output_dir output directory
   * \param[in] in_output_file_stem file name stem
   * \param[in] in_resolution grid_resolution value
   * \param[in] in_selected_phase the phase selected (if max then render all)
   */
  Render(
    std::array<std::string, 3> in_qoi_request,
    bool in_continuous_object_qoi,
    Info& in_info,
    std::array<uint64_t, 3> in_grid_size,
    double in_object_jitter,
    std::string in_output_dir,
    std::string in_output_file_stem,
    double in_resolution,
    bool in_save_meshes,
    bool in_save_pngs,
    PhaseType in_selected_phase = std::numeric_limits<PhaseType>::max()
  );

  /**
   * @brief Export a visualization PNG from meshes.
   *
   * @param phase Phase to render.
   * @param rank_mesh Mesh data for the ranks.
   * @param object_mesh Mesh data for the objects.
   * @param edge_width Width of the edges in the visualization.
   * @param max_volume Maximum volume of communications
   * @param glyph_factor Factor to control the size of glyphs.
   * @param win_size Size of the render window.
   * @param output_dir Directory in which to output artifacts
   * @param output_file_stem Stem for the artifact naming
   * @return A smart pointer to the resulting render window.
   */
  void renderPNG(
    PhaseType phase,
    vtkPolyData* rank_mesh,
    vtkPolyData* object_mesh,
    uint64_t edge_width,
    double glyph_factor,
    uint64_t win_size,
    uint64_t font_size,
    std::string output_dir,
    std::string output_file_stem
  );

  void generate(uint64_t font_size = 50, uint64_t win_size = 2000);

#ifdef VT_TV_HAS_TESTS
  /**
   * \brief Function to get internal object jitter dimensions.
   */
  const std::unordered_map<ElementIDType, std::array<double, 3>> getJitterDims(){
    return jitter_dims_;
  }

  /**
   * \brief Function to set manually the internal object jitter dimensions.
   */
  void setJitterDims(std::unordered_map<ElementIDType, std::array<double, 3>> jitter_dims){
    jitter_dims_ = jitter_dims;
  }
#endif
};

}} /* end namespace vt::tv */

#endif /*INCLUDED_VT_TV_RENDER_RENDER_H*/
