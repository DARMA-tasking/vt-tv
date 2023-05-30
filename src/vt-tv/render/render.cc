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
, n_ranks_(in_info.getNumRanks())
{
  // Generically set rank grid dimensions according to the total number of ranks

  grid_size_[2] = 1; // we assume 2D representation

  int sqrt_n_ranks = std::sqrt(n_ranks_);
  if(sqrt_n_ranks * sqrt_n_ranks == n_ranks_) {
    // n_ranks is a perfect square
    grid_size_[0] = sqrt_n_ranks;
    grid_size_[1] = sqrt_n_ranks;
  } else {
    // n_ranks is not a perfect square
    grid_size_[0] = sqrt_n_ranks; // floor
    grid_size_[1] = sqrt_n_ranks + 1; // ceil
  }

  for (uint64_t d = 0 ; d < 3 ; d++) {
    if(grid_size_[d] > 1) rank_dims_.insert(d);
  }
  max_o_per_dim_ = 0;

  // Initialize jitter
  std::srand(std::time(nullptr));
  auto const& allObjects = info_.getAllObjects(n_ranks_);
  for (auto const& [objectID, objectWork] : allObjects) {
    std::array<double, 3> jitterDims;
    for (uint64_t d = 0; d < 3; d++) {
      if (auto f = this->rank_dims_.find(d); f != this->rank_dims_.end()) {
        jitterDims[d] = ((double)std::rand()/RAND_MAX - 0.5) * object_jitter_;
      } else jitterDims[d] = 0;
    }
    jitter_dims_.insert(std::make_pair(objectID, jitterDims));
  }
};

Render::Render(
  std::array<std::string, 3> in_qoi_request,
  bool in_continuous_object_qoi,
  std::unordered_map<PhaseType, PhaseWork> in_phase_info,
  Info in_info,
  std::array<uint64_t, 3> in_grid_size,
  double in_object_jitter,
  std::string in_output_dir,
  std::string in_output_file_stem,
  double in_resolution
)
: rank_qoi_(in_qoi_request[0])
, object_qoi_(in_qoi_request[2])
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
  std::vector<NodeType> rankSet;
  for (auto const& [_, objInfo] : this->info_.getObjectInfo()) {
    rankSet.push_back(objInfo.getHome());
  }
  return rankSet;
}

std::unordered_map<NodeType, std::unordered_map<ElementIDType, ObjectWork>> Render::create_object_mapping_(PhaseType phase) {
  std::unordered_map<NodeType, std::unordered_map<ElementIDType, ObjectWork>> object_mapping;

  fmt::print("  -creating object mapping-\n");
  fmt::print("   phase: {}\n", phase);
  fmt::print("   n_ranks: {}\n", this->n_ranks_);

  // Add each rank and its corresponding objects at the given phase to the object mapping
  for (uint64_t rank_id = 0; rank_id < this->n_ranks_; rank_id++) {
    object_mapping.insert(std::make_pair(rank_id, this->info_.getRankObjects(rank_id, phase)));
  }

  fmt::print("  -finished creating object mapping-\n");
  return object_mapping;
}

vtkNew<vtkPolyData> Render::create_rank_mesh_(PhaseType iteration) {
  fmt::print("\n\n");
  fmt::print("-----creating rank mesh for phase {} -----\n", iteration);
  vtkNew<vtkPoints> rank_points_;
  rank_points_->SetNumberOfPoints(this->n_ranks_);

  vtkNew<vtkDoubleArray> rank_arr;
  rank_arr->SetName("rank qoi");
  rank_arr->SetNumberOfTuples(this->n_ranks_);

  fmt::print("  Number of ranks in phase: {}\n", this->n_ranks_);
  for (uint64_t rank_id = 0; rank_id < this->n_ranks_; rank_id++) {
    std::array<uint64_t, 3> cartesian = this->global_id_to_cartesian(rank_id, this->grid_size_);
    std::array<double, 3> offsets = {
      cartesian[0] * this->grid_resolution_,
      cartesian[1] * this->grid_resolution_,
      cartesian[2] * this->grid_resolution_
    };
    // Insert point based on cartesian coordinates
    rank_points_->SetPoint(rank_id, offsets[0], offsets[1], offsets[2]);

    // rank_arr->SetTuple1(rank_id, /* qoi */);
  }

  vtkNew<vtkPolyData> pd_mesh;
  pd_mesh->SetPoints(rank_points_);
  fmt::print("-----created rank mesh for phase {} -----\n", iteration);
  return pd_mesh;
}

vtkNew<vtkPolyData> Render::create_object_mesh_(PhaseWork phase) {
  fmt::print("\n\n");
  fmt::print("-----creating object mesh for phase {} -----\n", phase.getPhase());
  // Retrieve number of mesh points and bail out early if empty set
  uint64_t n_o = this->info_.getPhaseObjects(phase.getPhase(), this->n_ranks_).size();
  fmt::print("Number of objects in phase: {} -----\n", n_o);

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

  // Iterate over ranks and objects to create mesh points
  uint64_t point_index = 0;
  std::map<ElementIDType, uint64_t> objectid_to_index;

  auto object_mapping = this->create_object_mapping_(p_id);

  fmt::print("object_mapping size: {}\n", object_mapping.size());

  // Iterate through object mapping
  for (auto const& [rankID, objects] : object_mapping) {
    std::array<uint64_t, 3> ijk = this->global_id_to_cartesian(rankID, this->grid_size_);

    std::array<double, 3> offsets = {
      ijk[0] * this->grid_resolution_,
      ijk[1] * this->grid_resolution_,
      ijk[2] * this->grid_resolution_
    };

    // Compute local object block parameters
    uint64_t n_o_rank = objects.size();

    uint64_t n_o_per_dim = ceil( pow( n_o_rank, 1.0 / this->rank_dims_.size()) );
    if (n_o_per_dim > this->max_o_per_dim_) {
      this->max_o_per_dim_ = n_o_per_dim;
    }
    double o_resolution = this->grid_resolution_ / (n_o_per_dim + 1.);

    // Create point coordinates
    std::array<uint64_t, 3> rank_size = {1, 1, 1};
    for (uint64_t d = 0; d < 3; d++) {
      if (auto f = this->rank_dims_.find(d); f != this->rank_dims_.end()) {
        rank_size[d] = n_o_per_dim;
      } else rank_size[d] = 1;
    }

    std::array<double, 3> centering = {0, 0, 0};
    for (uint64_t d = 0; d < 3; d++) {
      if (auto f = this->rank_dims_.find(d); f != this->rank_dims_.end()) {
        centering[d] = 0.5 * o_resolution * (n_o_per_dim - 1.0);
      } else {
        centering[d] = 0.0;
      }
    }

    auto const& rank = this->info_.getRank(rankID);

    //@TODO: clean this stuff up

    struct cmpByMigratableThenID {
      cmpByMigratableThenID(const Info& info) :info_(info) {}
      bool operator()(const ObjectWork& lhs, const ObjectWork& rhs) const {
        ElementIDType lhsID = lhs.getID();
        ElementIDType rhsID = rhs.getID();
        bool migratableLhs = info_.getObjectInfo().at(lhsID).isMigratable();
        bool migratableRhs = info_.getObjectInfo().at(rhsID).isMigratable();
        if(migratableLhs != migratableRhs) {
            return migratableLhs < migratableRhs; // non-migratable comes first
        }
        return lhsID < rhsID; // Then sort by ID
      }
      private:
      Info info_;
    };
    std::map<ObjectWork, uint64_t, cmpByMigratableThenID> ordered_objects(cmpByMigratableThenID(this->info_));

    for (auto const& [objectID, objectWork] : objects) {
      bool migratable = this->info_.getObjectInfo().at(objectID).isMigratable();
      ordered_objects.insert(std::make_pair(objectWork, migratable));
    }

    // Add rank objects to point set
    int i = 0;
    for (auto const& [objectWork, sentinel] : ordered_objects) {

      // Insert point using offset and rank coordinates
      std::array<double, 3> currentPointPosition = {0, 0, 0};
      int d = 0;
      for (auto c : this->global_id_to_cartesian(i, rank_size)) {
        currentPointPosition[d] = offsets[d] - centering[d] + (
          jitter_dims_.at(objectWork.getID())[d] + c) * o_resolution;
        d++;
      }

      points->SetPoint(
        point_index,
        currentPointPosition[0],
        currentPointPosition[1],
        currentPointPosition[2]
      );

      // Set object attributes
      q_arr->SetTuple1(point_index, objectWork.getLoad());
      b_arr->SetTuple1(point_index, sentinel);
      i++;
      point_index++;
      objectid_to_index.insert(std::make_pair(objectWork.getID(), point_index));
    }
  }

  vtkNew<vtkDoubleArray> lineValuesArray;
  lineValuesArray->SetName("bytes");
  vtkNew<vtkCellArray> lines;
  uint64_t n_e = 0;
  auto phaseObjects = this->info_.getPhaseObjects(phase.getPhase(), this->n_ranks_);

  for (auto& [id, objWork] : phaseObjects) {
    fmt::print("id {}\n", id);
    std::map<ElementIDType, double>& receivedCommunications = objWork.getSent();
    fmt::print("map size: {}\n", receivedCommunications.size());
    for (auto& [from_id, bytes] : receivedCommunications) {
      fmt::print("id {} <-> from_id {}\n", id, from_id);
      vtkNew<vtkLine> line;
      lineValuesArray->InsertNextTuple1(bytes);
      line->GetPointIds()->SetId(0, objectid_to_index.at(id));
      line->GetPointIds()->SetId(1, objectid_to_index.at(from_id));
      lines->InsertNextCell(line);
    }
  }

  vtkNew<vtkPolyData> pd_mesh;
  pd_mesh->SetPoints(points);
  pd_mesh->SetLines(lines);
  pd_mesh->GetPointData()->SetScalars(q_arr);
  pd_mesh->GetPointData()->AddArray(b_arr);
  pd_mesh->GetCellData()->SetScalars(lineValuesArray);

  fmt::print("-----finished creating object mesh-----\n");

  // Setup the visualization pipeline
  vtkNew<vtkNamedColors> namedColors;
  vtkNew<vtkPolyData> linesPolyData;
  linesPolyData->SetPoints(points);
  linesPolyData->SetLines(lines);
  linesPolyData->GetCellData()->SetScalars(lineValuesArray);
  vtkNew<vtkPolyDataMapper> mapper;
  mapper->SetInputData(linesPolyData);

  vtkNew<vtkActor> actor;
  actor->SetMapper(mapper);
  actor->GetProperty()->SetLineWidth(4);

  vtkNew<vtkRenderer> renderer;
  renderer->AddActor(actor);
  renderer->SetBackground(namedColors->GetColor3d("SlateGray").GetData());

  vtkNew<vtkRenderWindow> window;
  window->SetWindowName("Colored Lines");
  window->AddRenderer(renderer);

  vtkNew<vtkRenderWindowInteractor> interactor;
  interactor->SetRenderWindow(window);

  // Visualize
  window->Render();
  interactor->Start();

  return pd_mesh;
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

/* static */ std::array<uint64_t, 3> Render::global_id_to_cartesian(
    uint64_t flat_id, std::array<uint64_t, 3> grid_sizes
) {
  std::array<uint64_t, 3> cartesian = {0, 0, 0};
  // Sanity check
  uint64_t n01 = grid_sizes[0] * grid_sizes[1];
  if (flat_id < 0 || flat_id >= n01 * grid_sizes[2]) {
    throw std::out_of_range("Index error");
  }

  // Compute successive Euclidean divisions
  uint64_t quot1 = flat_id / n01;
  uint64_t rem1 = flat_id % n01;
  uint64_t quot2 = rem1 / grid_sizes[0];
  uint64_t rem2 = rem1 % grid_sizes[0];
  cartesian[0] = rem2;
  cartesian[1] = quot2;
  cartesian[2] = quot1;
  return cartesian;
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

/*static*/ void Render::createObjectPipeline(
  vtkPolyData* object_mesh,
  vtkPolyData* rank_mesh
) {
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
  // rank_mapper->SetLookupTable(createColorTransferFunction({0,0.01}));
  // rank_mapper->SetScalarRange({0,0.01});

  vtkNew<vtkActor> rank_actor;
  rank_actor->SetMapper(rank_mapper);
  auto qoi_actor = createScalarBarActor(rank_mapper, "Rank XXX", 0.5, 0.9);
  qoi_actor->DrawBelowRangeSwatchOn();
  qoi_actor->SetBelowRangeAnnotation("<");
  qoi_actor->DrawAboveRangeSwatchOn();
  qoi_actor->SetAboveRangeAnnotation(">");
  renderer->AddActor(rank_actor);
  renderer->AddActor2D(qoi_actor);

  renderer->ResetCamera();
  vtkNew<vtkRenderWindow> render_window;
  render_window->AddRenderer(renderer);
  render_window->SetWindowName("LBAF");
  render_window->SetSize(100, 100);
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
  for (auto const& [phase, phase_work] : this->phase_info_) {
    // vtkPolyData* testPolyData = this->create_object_mesh_(phase_work);
    vtkNew<vtkPolyData> object_mesh = this->create_object_mesh_(phase_work);
    vtkNew<vtkPolyData> rank_mesh = this->create_rank_mesh_(phase_work.getPhase());

    this->createObjectPipeline(
      object_mesh.Get(), rank_mesh.Get()
    );

    vtkNew<vtkXMLPolyDataWriter> writer;
    std::string object_mesh_filename = "object_mesh_" + std::to_string(phase) + ".vtp";
    writer->SetFileName(object_mesh_filename.c_str());
    writer->SetInputData(object_mesh);
    writer->Write();

    vtkNew<vtkXMLPolyDataWriter> writer2;
    std::string rank_mesh_filneame = "rank_mesh_" + std::to_string(phase) + ".vtp";
    writer2->SetFileName(rank_mesh_filneame.c_str());
    writer2->SetInputData(rank_mesh);
    writer2->Write();

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
    std::array<uint64_t, 3> test_dims = {5, 5, 1};
    auto test_coordinates = this->global_id_to_cartesian(actor_i, test_dims);
    sphereSource->SetCenter(test_coordinates[0], test_coordinates[1], test_coordinates[2]);

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
