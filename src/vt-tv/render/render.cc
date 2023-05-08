/*
//@HEADER
// *****************************************************************************
//
//                                 render.cc
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

#include "vt-tv/render/render.h"

// #include <nanobind/nanobind.h>

// namespace nb = nanobind;

// using namespace nb::literals;

namespace vt { namespace tv {

Render::Render(std::unordered_map<PhaseType, PhaseWork> in_phase_info, Info in_info)
: phase_info_(std::move(in_phase_info))
, info_(in_info)
{ };

Render::Render(
  Triplet<std::string> in_qoi_request,
  bool in_continuous_object_qoi,
  std::unordered_map<PhaseType, PhaseWork> in_phase_info,
  Info in_info,
  Triplet<uint64_t> in_grid_size,
  double in_object_jitter,
  std::string in_output_dir,
  std::string in_output_file_stem,
  double in_resolution
)
: rank_qoi_(in_qoi_request.one)
, object_qoi_(in_qoi_request.three)
, continuous_object_qoi_(in_continuous_object_qoi)
, phase_info_(in_phase_info)
, info_(in_info)
, grid_size_(in_grid_size)
, object_jitter_(in_object_jitter)
, output_dir_(in_output_dir)
, output_file_stem_(in_output_file_stem)
, grid_resolution_(in_resolution)
{
  // initialize number of ranks
  n_ranks_ = info_.getNumRanks();

  // initialize rank dimensions according to given grid
  for (uint64_t d = 0 ; d < 3 ; d++) {
    if(grid_size_[d] > 1) rank_dims_.insert(d);
  }
  max_o_per_dim_ = 0;

  // @TODO compute constant per object jitter

  // Initialize load range
  this->object_load_range_ = this->compute_object_load_range();
};

std::tuple<TimeType, TimeType> Render::compute_object_load_range() {
  // Initialize space-time object QOI range attributes
  TimeType oq_max = -1 * std::numeric_limits<double>::infinity();
  TimeType oq_min = std::numeric_limits<double>::infinity();
  TimeType ol;

  // Iterate over all phases
  for (auto const& [_, phase_work] : this->phase_info_) {
    // Iterate over all objects in phase
    for (auto const& [elm_id, work] : phase_work.getObjectWork()) {
      // Update maximum object load
      ol = work.getLoad();
      if (ol > oq_max) oq_max = ol;
      if (ol < oq_min) oq_min = ol;
    }
  }

  // Update extrema attribute
  this->object_load_max_ = oq_max;

  // return range
  return std::make_tuple(oq_min, oq_max);
}

std::vector<NodeType> Render::getRanks(PhaseType phase_in) const {
  fmt::print("Getting Ranks in phase: {}\n", phase_in);
  std::vector<NodeType> rankSet;
  for (auto const& [_, objInfo] : this->info_.getObjectInfo()) {
    fmt::print("  rank: {}\n", objInfo.getHome());
    rankSet.push_back(objInfo.getHome());
  }
  return rankSet;
}

vtkPolyData* Render::create_rank_mesh_(PhaseType iteration) {
  vtkNew<vtkPoints> rank_points_;
  rank_points_->SetNumberOfPoints(this->n_ranks_);
  for (uint64_t rank_id = 0; rank_id < this->n_ranks_; rank_id++) {
    Triplet cartesian = this->global_id_to_cartesian(rank_id, this->grid_size_);
    Triplet offsets = Triplet(
      cartesian.one * this->grid_resolution_,
      cartesian.two * this->grid_resolution_,
      cartesian.three * this->grid_resolution_
      );
    // Insert point based on cartesian coordinates
    rank_points_->SetPoint(rank_id, offsets.one, offsets.two, offsets.three);
  }

  vtkNew<vtkPolyData> pd_mesh;
  pd_mesh->SetPoints(rank_points_);
  return pd_mesh;
}

vtkPolyData* Render::create_object_mesh_(PhaseWork phase) {
  // Retrieve number of mesh points and bail out early if empty set
  uint64_t n_o = phase.getObjectWork().size();

  // Create point array for object quantity of interest
  vtkNew<vtkDoubleArray> q_arr;
  q_arr->SetName(this->object_qoi_.c_str());
  q_arr->SetNumberOfTuples(n_o);

  // Load array must be added when it is not the object QOI
  vtkNew<vtkDoubleArray> l_arr;
  if (object_qoi_ != "load") {
    l_arr->SetName("load");
    l_arr->SetNumberOfTuples(n_o);
  }

  // Create bit array for object migratability
  vtkNew<vtkBitArray> b_arr;
  b_arr->SetName("migratable");
  b_arr->SetNumberOfTuples(n_o);

  // Create and size point set
  vtkNew<vtkPoints> points;
  points->SetNumberOfPoints(n_o);

  // Retrieve elements constant across all ranks
  PhaseType p_id = phase.getPhase();
  std::vector<NodeType> ranks = this->getRanks(p_id);
  std::string object_qoi = this->object_qoi_;

  // @todo
  // Iterate over ranks and objects to create mesh points
  uint64_t point_index = 0;
  std::map<ObjectInfo, uint64_t> point_to_index;

  // Iterate through ranks
  for (uint64_t rank_id = 0; rank_id < this->n_ranks_; rank_id++) {
    // Find objects in this rank
    for (auto const& [objectID, objectInfo] : this->info_.getObjectInfo()) {
      if(objectInfo.getHome() == rank_id) {

      }
    }
  }

  vtkNew<vtkPolyData> pd_mesh;

  return pd_mesh;


  // for (auto const& [objectID, objectInfo] : this->info_.getObjectInfo()) {
  //   // Determine rank offsets
  //   // @TODO change values to struct values, change syntax
  //   fmt::print("rank_id: {}\n",rank_id);
  //   const auto [i, j, k] = this->global_id_to_cartesian(rank_id, std::make_tuple(2,2,1));
  //   fmt::print("cartesian: [{}, {}, {}]\n",i,j,k);
  //   std::tuple<uint64_t, uint64_t, uint64_t> offsets = std::make_tuple(i*this->grid_resolution_, j*this->grid_resolution_, k*this->grid_resolution_);
  //   const auto [o1, o2, o3] = offsets;
  //   fmt::print("offsets: [{}, {}, {}]\n",o1,o2,o3);

  //   // Compute local object block parameters
  //   uint64_t n_o_rank = objects.size();
  //   fmt::print("n_o_rank: {}\n",n_o_rank);
  //   // @TODO add rank_dims_ struct attribute
  //   // n_o_per_dim = math.ceil(n_o_rank ** (1. / len(self.__rank_dims)))
  //   uint64_t n_o_per_dim = ceil( pow( n_o_rank, 1.0 / 2) );
  //   if (n_o_per_dim > this->max_o_per_dim_) {
  //     this->max_o_per_dim_ = n_o_per_dim;
  //   }
  //   double o_resolution = this->grid_resolution_ / (n_o_per_dim + 1.);

  //   // Create point coordinates
  //   std::set<uint64_t> rank_size;
  //   for (uint64_t d = 0; d < 3; d++) {
  //     if (auto f = this->rank_dims_.find(d); f != this->rank_dims_.end())
  //     rank_size.insert(n_o_per_dim); else rank_size.insert(1);
  //   }
  //   std::set<double> centering;
  //   for (uint64_t d = 0; d < 3; d++) {
  //     if (auto f = this->rank_dims_.find(d); f != this->rank_dims_.end()) {
  //       centering.insert(0.5 * o_resolution * (n_o_per_dim - 1.0));
  //     } else {
  //       centering.insert(0.0);
  //     }
  //   }

  //   // Order objects of current rank
  //   // NodeType r = ranks[rank_id];
  //   // std::map<ElementIDType,TimeType> objects_list;
  //   // objects_list = std::sort(objects.begin(), objects.end(), [](uint64_t x)
  //   //                                                           {
  //   //                                                             return x.getID();
  //   //                                                           });
  //   rank_id++;
  // }
}

/*static*/ vtkNew<vtkColorTransferFunction> Render::createColorTransferFunction(
  double range[2], double avg_load, ColorType ct
) {
  vtkNew<vtkColorTransferFunction> ctf;
  ctf->SetNanColorRGBA(1., 1., 1., 0.);
  ctf->UseBelowRangeColorOn();
  ctf->UseAboveRangeColorOn();

  switch (ct) {
  case BlueToRed: {
    ctf->SetColorSpaceToDiverging();
    double const mid_point = (range[0] + range[1]) * .5;
    ctf->AddRGBPoint(range[0], .231, .298, .753);
    ctf->AddRGBPoint(mid_point, .865, .865, .865);
    ctf->AddRGBPoint(range[1], .906, .016, .109);
    ctf->SetBelowRangeColor(0.0, 1.0, 0.0);
    ctf->SetAboveRangeColor(1.0, 0.0, 1.0);
    break;
  }
  case HotSpot: {
    ctf->SetColorSpaceToDiverging();
    double const mid_point = avg_load;
    ctf->AddRGBPoint(range[0], .231, .298, .753);
    ctf->AddRGBPoint(mid_point, .865, .865, .865);
    ctf->AddRGBPoint(range[1], .906, .016, .109);
    ctf->SetBelowRangeColor(0.0, 1.0, 1.0);
    ctf->SetAboveRangeColor(1.0, 1.0, 0.0);
    break;
  }
  case WhiteToBlack: {
    ctf->AddRGBPoint(range[0], 1.0, 1.0, 1.0);
    ctf->AddRGBPoint(range[1], 0.0, 0.0, 0.0);
    ctf->SetBelowRangeColor(0.0, 0.0, 1.0);
    ctf->SetAboveRangeColor(1.0, 0.0, 0.0);
    break;
  }
  case Default: {
    double const mid_point = (range[0] + range[1]) * .5;
    ctf->AddRGBPoint(range[0], .431, .761, .161);
    ctf->AddRGBPoint(mid_point, .98, .992, .059);
    ctf->AddRGBPoint(range[1], 1.0, .647, 0.0);
    ctf->SetBelowRangeColor(0.8, 0.8, .8);
    ctf->SetAboveRangeColor(1.0, 0.0, 1.0);
    break;
  }
  }

  return ctf;
}

/*static*/ vtkNew<vtkScalarBarActor> Render::createScalarBarActor(
  vtkPolyDataMapper* mapper, std::string title, double x, double y
) {
  vtkNew<vtkScalarBarActor> scalar_bar_actor;
  scalar_bar_actor->SetLookupTable(mapper->GetLookupTable());

  scalar_bar_actor->SetOrientationToHorizontal();
  scalar_bar_actor->UnconstrainedFontSizeOn();
  scalar_bar_actor->SetNumberOfLabels(2);
  scalar_bar_actor->SetHeight(0.08);
  scalar_bar_actor->SetWidth(0.4);
  scalar_bar_actor->SetLabelFormat("%.2G");
  scalar_bar_actor->SetBarRatio(0.3);
  scalar_bar_actor->DrawTickLabelsOn();

  std::vector<vtkTextProperty*> props;
  props.push_back(scalar_bar_actor->GetTitleTextProperty());
  props.push_back(scalar_bar_actor->GetLabelTextProperty());
  props.push_back(scalar_bar_actor->GetAnnotationTextProperty());

  for (auto&& p : props) {
    p->SetColor(0.0, 0.0, 0.0);
    p->ItalicOff();
    p->BoldOff();
    p->SetFontFamilyToArial();
    p->SetFontSize(72);
  }

  scalar_bar_actor->SetTitle(title.c_str());
  auto position = scalar_bar_actor->GetPositionCoordinate();
  position->SetCoordinateSystemToNormalizedViewport();
  position->SetValue(x, y, 0.0);

  return scalar_bar_actor;
}

/* static */ Triplet<uint64_t> Render::global_id_to_cartesian(
    uint64_t flat_id, Triplet<uint64_t> grid_sizes
) {
  // Sanity check
  uint64_t n01 = grid_sizes.one * grid_sizes.two;
  if (flat_id < 0 || flat_id >= n01 * grid_sizes.three) {
    throw std::out_of_range("Index error");
  }

  // Compute successive Euclidean divisions
  uint64_t quot1 = flat_id / n01;
  uint64_t rem1 = flat_id % n01;
  uint64_t quot2 = rem1 / grid_sizes.one;
  uint64_t rem2 = rem1 % grid_sizes.one;
  return Triplet(rem2, quot2, quot1);
}

/*static*/ void Render::createPipeline(
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
) {
  vtkNew<vtkPolyData> rank_mesh;
  rank_mesh->SetPoints(rank_points);
  rank_mesh->SetLines(rank_lines);
  rank_mesh->GetPointData()->SetScalars(qois);

  vtkNew<vtkRenderer> renderer;
  renderer->SetBackground(1.0, 1.0, 1.0);
  renderer->GetActiveCamera()->ParallelProjectionOn();

  vtkNew<vtkGlyphSource2D> rank_glyph;
  rank_glyph->SetGlyphTypeToSquare();
  rank_glyph->SetScale(.95);
  rank_glyph->FilledOn();
  rank_glyph->CrossOff();
  vtkNew<vtkGlyph2D> rank_glypher;
  rank_glypher->SetSourceConnection(rank_glyph->GetOutputPort());
  rank_glypher->SetInputData(rank_mesh);
  rank_glypher->SetScaleModeToDataScalingOff();

  //Lower glyphs slightly for visibility
  vtkNew<vtkTransform> z_lower;
  z_lower->Translate(0.0, 0.0, -0.01);
  vtkNew<vtkTransformPolyDataFilter> trans;
  trans->SetTransform(z_lower);
  trans->SetInputConnection(rank_glypher->GetOutputPort());

  vtkNew<vtkPolyDataMapper> rank_mapper;
  rank_mapper->SetInputConnection(trans->GetOutputPort());
  rank_mapper->SetLookupTable(createColorTransferFunction(qoi_range));
  rank_mapper->SetScalarRange(qoi_range);

  vtkNew<vtkActor> rank_actor;
  rank_actor->SetMapper(rank_mapper);
  auto qoi_actor = createScalarBarActor(rank_mapper, "Rank XXX", 0.5, 0.9);
  qoi_actor->DrawBelowRangeSwatchOn();
  qoi_actor->SetBelowRangeAnnotation("<");
  qoi_actor->DrawAboveRangeSwatchOn();
  qoi_actor->SetAboveRangeAnnotation(">");
  renderer->AddActor(rank_actor);
  renderer->AddActor2D(qoi_actor);

  // vtkNew<vtkLookupTable> bw_lut;
  // bw_lut->SetTableRange((0.0, self.__max_object_volume));
  // bw_lut->SetSaturationRange(0, 0);
  // bw_lut->SetHueRange(0, 0);
  // bw_lut->SetValueRange(1, 0);
  // bw_lut->SetNanColor(1.0, 1.0, 1.0, 0.0);
  // bw_lut->Build();

  vtkNew<vtkArrayCalculator> sqrtT;
  sqrtT->SetInputData(object_mesh);
  sqrtT->AddScalarArrayName("Load");
  char const* sqrtT_str = "sqrt(Load)";
  sqrtT->SetFunction(sqrtT_str);
  sqrtT->SetResultArrayName(sqrtT_str);
  sqrtT->Update();
  auto sqrtT_out = sqrtT->GetDataSetOutput();
  sqrtT_out->GetPointData()->SetActiveScalars("Migratable");

  std::vector<std::tuple<double, std::string>> items{{0.0, "Square"}, {1.0, "Circle"}};
  vtkPolyDataMapper* glyph_mapper_out = nullptr;
  for (auto&& [k,v] : items) {
    vtkNew<vtkThresholdPoints> thresh;
    thresh->SetInputData(sqrtT_out);
    thresh->ThresholdBetween(k, k);
    thresh->Update();
    auto thresh_out = thresh->GetOutput();
    if (not thresh_out->GetNumberOfPoints())
      continue;
    thresh_out->GetPointData()->SetActiveScalars(sqrtT_str);

    // Glyph by square root of object loads
    vtkNew<vtkGlyphSource2D> glyph;
    if (v == "Square") {
      glyph->SetGlyphTypeToSquare();
    } else {
      glyph->SetGlyphTypeToCircle();
    }
    glyph->SetResolution(32);
    glyph->SetScale(1.0);
    glyph->FilledOn();
    glyph->CrossOff();
    vtkNew<vtkGlyph3D> glypher;
    glypher->SetSourceConnection(glyph->GetOutputPort());
    glypher->SetInputData(thresh_out);
    glypher->SetScaleModeToScaleByScalar();
    glypher->SetScaleFactor(glyph_factor);
    glypher->Update();
    glypher->GetOutput()->GetPointData()->SetActiveScalars("Load");

    // Raise glyphs slightly for visibility
    vtkNew<vtkTransform> z_raise;
    z_raise->Translate(0.0, 0.0, 0.01);
    vtkNew<vtkTransformPolyDataFilter> trans;
    trans->SetTransform(z_raise);
    trans->SetInputData(glypher->GetOutput());

    // Create mapper and actor for glyphs
    vtkNew<vtkPolyDataMapper> glyph_mapper;
    glyph_mapper_out = glyph_mapper;
    glyph_mapper->SetInputConnection(trans->GetOutputPort());
    glyph_mapper->SetLookupTable(createColorTransferFunction(load_range, 0, BlueToRed));
    glyph_mapper->SetScalarRange(load_range);
    vtkNew<vtkActor> glyph_actor;
    glyph_actor->SetMapper(glyph_mapper);
    renderer->AddActor(glyph_actor);
  }

  if (glyph_mapper_out) {
    auto load_actor = createScalarBarActor(glyph_mapper_out, "Object Load", 0.55, 0.55);
    renderer->AddActor2D(load_actor);
  }

  renderer->ResetCamera();
  vtkNew<vtkRenderWindow> render_window;
  render_window->AddRenderer(renderer);
  render_window->SetWindowName("LBAF");
  render_window->SetSize(win_size, win_size);
  render_window->Render();

  vtkNew<vtkWindowToImageFilter> w2i;
  w2i->SetInput(render_window);
  w2i->SetScale(3);

  vtkNew<vtkPNGWriter> writer;
  writer->SetInputConnection(w2i->GetOutputPort());
  writer->SetFileName("test.png");
  writer->SetCompressionLevel(2);
  writer->Write();
}

void Render::generate() {
  // Create vector of number of objects per phase
  std::vector<uint64_t> n_objects_list;
  vtkPolyData* testPolyData;
  for (auto const& [phase, phase_work] : this->phase_info_) {
    testPolyData = this->create_object_mesh_(phase_work);
    n_objects_list.push_back(phase_work.getObjectWork().size());
  }

  // Create vector of vectors of object loads per phase
  std::vector<std::vector<TimeType>> phases_object_loads;
  for (auto const& [phase, phase_work] : this->phase_info_) {
    std::vector<TimeType> object_loads;
    for (auto const& [elm_id, work] : phase_work.getObjectWork()) {
      object_loads.push_back(work.getLoad());
    }
    phases_object_loads.push_back(object_loads);
  }

  // Find min max values of loads for phase 0 for visualization scaling
  auto max_load = *max_element(std::begin(phases_object_loads[0]), std::end(phases_object_loads[0]));
  auto min_load = *min_element(std::begin(phases_object_loads[0]), std::end(phases_object_loads[0]));

  // Assign number of objects as number of actors in the visualization
  uint64_t n_actors = n_objects_list[0];

  // Add objects in visualization as spheres and scale them appropriately
  vtkNew<vtkNamedColors> colors;
  vtkNew<vtkRenderer> renderer;
  std::vector<std::string> color_vector = {"Red", "Green", "Cornsilk"};
  for (uint64_t actor_i = 0; actor_i < n_actors; actor_i++)
  {
    // Create a sphere
    vtkNew<vtkSphereSource> sphereSource;
    sphereSource->SetRadius(pow(phases_object_loads[0][actor_i] / (max_load-min_load), 2) * 0.5);
    const auto [i, j, k] = this->global_id_to_cartesian(actor_i, {5, 5, 1});
    sphereSource->SetCenter(i, j, k);

    // Make the surface smooth.
    sphereSource->SetPhiResolution(100);
    sphereSource->SetThetaResolution(100);

    // Add sphere to mapper
    vtkNew<vtkPolyDataMapper> mapper;
    mapper->SetInputConnection(sphereSource->GetOutputPort());

    // Add actor to render
    vtkNew<vtkActor> actor;
    actor->SetMapper(mapper);
    actor->GetProperty()->SetColor(colors->GetColor3d(color_vector[2]).GetData());
    renderer->AddActor(actor);
  }

  // Set background color
  renderer->SetBackground(colors->GetColor3d("steelblue").GetData());

  vtkNew<vtkRenderWindow> renderWindow;
  renderWindow->AddRenderer(renderer);

  vtkNew<vtkRenderWindowInteractor> renderWindowInteractor;
  renderWindowInteractor->SetRenderWindow(renderWindow);

  renderWindow->SetWindowName("Render");
  renderWindow->Render();
  renderWindowInteractor->Start();
}

}} /* end namesapce vt::tv */
