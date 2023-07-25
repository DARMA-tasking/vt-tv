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

Render::Render(Info in_info)
: info_(in_info) // std:move ?
, n_ranks_(in_info.getNumRanks())
, n_phases_(in_info.getNumPhases())
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
  auto const& allObjects = info_.getAllObjects();
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
, info_(in_info)
, n_ranks_(in_info.getNumRanks())
, n_phases_(in_info.getNumPhases())
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

  // Initialize jitter
  std::srand(std::time(nullptr));
  auto const& allObjects = info_.getAllObjects();
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

std::pair<double, double> Render::computeObjectQoiRange_() {
  // Initialize object QOI range attributes
  double oq_max = -1 * std::numeric_limits<double>::infinity();
  double oq_min = std::numeric_limits<double>::infinity();
  double oq;

  // Iterate over all ranks
  auto const& objects = this->info_.getAllObjects();
  for (auto const& [obj_id, obj_work] : objects) {
    // Update maximum object qoi
    if (this->object_qoi_ == "load") {
      oq = obj_work.getLoad();
    } else {
      throw std::runtime_error("Invalid QOI: " + this->object_qoi_);
    }
    if (oq > oq_max) oq_max = oq;
    if (oq < oq_min) oq_min = oq;
  }

  // Update extrema attribute
  this->object_qoi_max_ = oq_max;

  // return range
  return std::make_pair(oq_min, oq_max);
}

std::pair<double, double> Render::computeRankQoiRange_() {
  // Initialize rank QOI range attributes
  double rq_max = -1 * std::numeric_limits<double>::infinity();
  double rq_min = std::numeric_limits<double>::infinity();
  double rq;

  // Iterate over all ranks
  for (uint64_t rank_id = 0; rank_id < this->n_ranks_; rank_id++) {
    // Update maximum rank qoi
    if (this->rank_qoi_ == "load") {
      // Get rank loads per phase
      std::unordered_map<PhaseType, double> rank_loads_map = this->info_.getAllLoadsAtRank(rank_id);

      // Get max load for this rank across all phases
      auto pr = std::max_element
      (
          std::begin(rank_loads_map), std::end(rank_loads_map),
          [] (const std::pair<PhaseType, double>& p1, const std::pair<PhaseType, double>& p2) {
              return p1.second < p2.second;
          }
      );
      rq = pr->second;
    } else {
      throw std::runtime_error("Invalid QOI: " + this->object_qoi_);
    }
    if (rq > rq_max) rq_max = rq;
    if (rq < rq_min) rq_min = rq;
  }

  // Update extrema attribute
  this->object_qoi_max_ = rq_max;

  // return range
  return std::make_pair(rq_min, rq_max);
}

std::map<NodeType, std::unordered_map<ElementIDType, ObjectWork>> Render::createObjectMapping_(PhaseType phase) {
  std::map<NodeType, std::unordered_map<ElementIDType, ObjectWork>> object_mapping;
  // Add each rank and its corresponding objects at the given phase to the object mapping
  for (uint64_t rank_id = 0; rank_id < this->n_ranks_; rank_id++) {
    object_mapping.insert(std::make_pair(rank_id, this->info_.getRankObjects(rank_id, phase)));
  }
  return object_mapping;
}

vtkNew<vtkPolyData> Render::createRankMesh_(PhaseType iteration) {
  fmt::print("\n\n");
  fmt::print("----- Creating rank mesh for phase {} -----\n", iteration);
  vtkNew<vtkPoints> rank_points_;
  rank_points_->SetNumberOfPoints(this->n_ranks_);

  vtkNew<vtkDoubleArray> rank_arr;
  std::string rank_array_name = "Rank " + this->rank_qoi_;
  rank_arr->SetName(rank_array_name.c_str());
  rank_arr->SetNumberOfTuples(this->n_ranks_);

  for (uint64_t rank_id = 0; rank_id < this->n_ranks_; rank_id++) {
    std::array<uint64_t, 3> cartesian = this->globalIDToCartesian_(rank_id, this->grid_size_);
    std::array<double, 3> offsets = {
      cartesian[0] * this->grid_resolution_,
      cartesian[1] * this->grid_resolution_,
      cartesian[2] * this->grid_resolution_
    };
    // Insert point based on cartesian coordinates
    rank_points_->SetPoint(rank_id, offsets[0], offsets[1], offsets[2]);

    auto objects = this->info_.getRankObjects(rank_id, iteration);

    double rank_load = 0;
    for (auto [id, object] : objects) {
      rank_load += object.getLoad();
    }

    rank_arr->SetTuple1(rank_id, rank_load);
  }

  vtkNew<vtkPolyData> pd_mesh;
  pd_mesh->SetPoints(rank_points_);
  pd_mesh->GetPointData()->SetScalars(rank_arr);
  fmt::print("----- Finished creating rank mesh for phase {} -----\n", iteration);
  return pd_mesh;
}

bool compareObjects(const std::pair<ObjectWork, uint64_t>& p1, const std::pair<ObjectWork, uint64_t>& p2) {
  ElementIDType lhsID = p1.first.getID();
  ElementIDType rhsID = p2.first.getID();
  uint64_t migratableLhs = p1.second;
  uint64_t migratableRhs = p2.second;
  if (migratableLhs != migratableRhs) {
    return migratableLhs < migratableRhs; // non-migratable comes first
  }
  return lhsID < rhsID; // then sort by ID
}

vtkNew<vtkPolyData> Render::createObjectMesh_(PhaseType phase) {
  fmt::print("\n\n");
  fmt::print("----- Creating object mesh for phase {} -----\n", phase);
  // Retrieve number of mesh points and bail out early if empty set
  uint64_t n_o = this->info_.getPhaseObjects(phase).size();
  fmt::print("  Number of objects in phase: {}\n", n_o);

  // Create point array for object quantity of interest
  vtkNew<vtkDoubleArray> q_arr;
  std::string object_array_name = "Object " + this->object_qoi_;
  q_arr->SetName(object_array_name.c_str());
  q_arr->SetNumberOfTuples(n_o);

  // Load array must be added when it is not the object QOI
  vtkNew<vtkDoubleArray> l_arr;
  if (object_qoi_ != "load") {
    l_arr->SetName("Object load");
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
  std::vector<NodeType> ranks = this->info_.getRankIDs();
  std::string object_qoi = this->object_qoi_;

  // Iterate over ranks and objects to create mesh points
  uint64_t point_index = 0;
  std::map<ElementIDType, uint64_t> objectid_to_index;
  // sent_volumes is a vector to store the communications ("from" object id, "sent to" object id, and volume)
  std::vector<std::tuple<ElementIDType, ElementIDType, double>> sent_volumes;

  auto object_mapping = this->createObjectMapping_(phase);

  // Iterate through object mapping
  for (auto const& [rankID, objects] : object_mapping) {
    std::array<uint64_t, 3> ijk = this->globalIDToCartesian_(rankID, this->grid_size_);

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

    std::vector<std::pair<ObjectWork, uint64_t>> ordered_objects;

    for (auto const& [objectID, objectWork] : objects) {
      bool migratable = this->info_.getObjectInfo().at(objectID).isMigratable();
      ordered_objects.push_back(std::make_pair(objectWork, migratable));
    }

    // Sort objects
    std::sort(ordered_objects.begin(), ordered_objects.end(), compareObjects);

    // Add rank objects to point set
    int i = 0;
    for (auto const& [objectWork, sentinel] : ordered_objects) {
      // fmt::print("Object ID: {}, sentinel: {}\n", objectWork.getID(), sentinel);

      // Insert point using offset and rank coordinates
      std::array<double, 3> currentPointPosition = {0, 0, 0};
      int d = 0;
      for (auto c : this->globalIDToCartesian_(i, rank_size)) {
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

      auto objSent = objectWork.getSent();
      for (auto [k, v] : objSent) {
        sent_volumes.push_back(std::make_tuple(point_index, k, v));
      }

      i++;
      objectid_to_index.insert(std::make_pair(objectWork.getID(), point_index));
      point_index++;
    }
  }

  vtkNew<vtkDoubleArray> lineValuesArray;
  lineValuesArray->SetName("bytes");
  vtkNew<vtkCellArray> lines;
  uint64_t n_e = 0;
  std::map<std::tuple<ElementIDType, ElementIDType>, std::tuple<uint64_t, double>> edge_values;

  fmt::print("  Creating inter-object communication edges\n");
  for(auto& [pt_index, k, v] : sent_volumes) {
    // sort the point index and the object id in the "ij" tuple
    std::tuple<ElementIDType, ElementIDType> ij;
    if (pt_index <= objectid_to_index.at(k)) {
      ij = {pt_index, objectid_to_index.at(k)};
    }
    else {
      ij = {objectid_to_index.at(k), pt_index};
    }

    std::tuple<uint64_t, double> edge_value;
    if (auto e_ij = edge_values.find(ij); e_ij != edge_values.end()) {
      // If the line already exist we just update the volume
      auto current_edge = std::get<0>(edge_values.at(ij));
      auto current_v = std::get<1>(edge_values.at(ij));
      edge_values.at(ij) = {current_edge, current_v + v};
      lineValuesArray->SetTuple1(std::get<0>(edge_values.at(ij)), std::get<1>(edge_values.at(ij)));
      // fmt::print("\tupdating edge {} ({}--{}): {}\n", current_edge, std::get<0>(ij), std::get<1>(ij), current_v+v);
    }
    else {
      // If it doesn't, we create it
      // fmt::print("\tcreating edge {} ({}--{}): {}\n", n_e, std::get<0>(ij), std::get<1>(ij), v);
      edge_value = {n_e, v};
      edge_values.insert({ij, edge_value});
      n_e += 1;
      lineValuesArray->InsertNextTuple1(v);
      vtkNew<vtkLine> line;
      line->GetPointIds()->SetId(0, std::get<0>(ij));
      line->GetPointIds()->SetId(1, std::get<1>(ij));
      lines->InsertNextCell(line);
    }
  }

  vtkNew<vtkPolyData> pd_mesh;
  pd_mesh->SetPoints(points);
  pd_mesh->SetLines(lines);
  pd_mesh->GetPointData()->SetScalars(q_arr);
  pd_mesh->GetPointData()->AddArray(b_arr);
  pd_mesh->GetCellData()->SetScalars(lineValuesArray);

  fmt::print("----- Finished creating object mesh -----\n");

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

/*static*/ vtkNew<vtkScalarBarActor> Render::createScalarBarActor_(
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

/* static */ std::array<uint64_t, 3> Render::globalIDToCartesian_(
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
  auto qoi_actor = createScalarBarActor_(rank_mapper, "Rank XXX", 0.5, 0.9);
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
    auto load_actor = createScalarBarActor_(glyph_mapper_out, "Object Load", 0.55, 0.55);
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

/*static*/ void Render::createPipeline2(
  vtkPolyData* object_mesh,
  vtkPolyData* rank_mesh
) {
  vtkNew<vtkRenderer> renderer;
  renderer->SetBackground(1.0, 1.0, 1.0);
  renderer->GetActiveCamera()->ParallelProjectionOn();

  // Rank glyphs
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

  vtkNew<vtkActor> rank_actor;
  rank_actor->SetMapper(rank_mapper);

  // Object glyphs
  vtkNew<vtkGlyphSource2D> object_glyph;
  object_glyph->SetGlyphTypeToCircle();
  object_glyph->SetScale(0.2);
  object_glyph->FilledOn();
  object_glyph->CrossOff();
  vtkNew<vtkGlyph2D> object_glypher;
  object_glypher->SetSourceConnection(object_glyph->GetOutputPort());
  object_glypher->SetInputData(object_mesh);
  object_glypher->SetScaleModeToDataScalingOff();

  vtkNew<vtkPolyDataMapper> object_mapper;
  object_mapper->SetInputConnection(object_glypher->GetOutputPort());

  vtkNew<vtkActor> object_actor;
  object_actor->SetMapper(object_mapper);

  // Edges
  vtkNew<vtkPolyDataMapper> edge_mapper;
  edge_mapper->SetInputData(object_mesh);
  edge_mapper->SetScalarModeToUseCellData();
  edge_mapper->SetScalarRange(0.0, 40);

  vtkNew<vtkActor> edge_actor;
  edge_actor->SetMapper(edge_mapper);
  edge_actor->GetProperty()->SetLineWidth(10);

  // Create renderer
  renderer->AddActor(rank_actor);
  renderer->AddActor(object_actor);
  renderer->AddActor(edge_actor);
  vtkNew<vtkNamedColors> colors;
  renderer->SetBackground(colors->GetColor3d("steelblue").GetData());

  renderer->ResetCamera();
  vtkNew<vtkRenderWindow> render_window;
  render_window->AddRenderer(renderer);
  render_window->SetWindowName("LBAF");
  render_window->SetSize(500, 500);
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
  std::pair<double, double> rank_qoi_range = this->computeRankQoiRange_();
  double rank_qoi_min = std::get<0>(rank_qoi_range);
  double rank_qoi_max = std::get<1>(rank_qoi_range);

  std::pair<double, double> object_qoi_range = this->computeObjectQoiRange_();
  double object_qoi_min = std::get<0>(object_qoi_range);
  double object_qoi_max = std::get<1>(object_qoi_range);

  fmt::print("Rank {} range: {}, {}\n", rank_qoi_, rank_qoi_min, rank_qoi_max);
  fmt::print("Object {} range: {}, {}\n", object_qoi_, object_qoi_min, object_qoi_max);

  for(PhaseType phase = 0; phase < this->n_phases_; phase++) {
    this->info_.normalizeEdges(phase);

    vtkNew<vtkPolyData> object_mesh = this->createObjectMesh_(phase);
    vtkNew<vtkPolyData> rank_mesh = this->createRankMesh_(phase);

    fmt::print("Writing object mesh for phase {}\n", phase);
    vtkNew<vtkXMLPolyDataWriter> writer;
    std::string object_mesh_filename = "object_mesh_" + std::to_string(phase) + ".vtp";
    writer->SetFileName(object_mesh_filename.c_str());
    writer->SetInputData(object_mesh);
    writer->Write();

    fmt::print("Writing rank mesh for phase {}\n", phase);
    vtkNew<vtkXMLPolyDataWriter> writer2;
    std::string rank_mesh_filneame = "rank_mesh_" + std::to_string(phase) + ".vtp";
    writer2->SetFileName(rank_mesh_filneame.c_str());
    writer2->SetInputData(rank_mesh);
    writer2->Write();
  }
}

}} /* end namesapce vt::tv */
