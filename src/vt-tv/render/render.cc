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

  object_qoi_range_ = this->computeObjectQoiRange_();
  this->computeMaxObjectVolume_();
};

Render::Render(
  std::array<std::string, 3> in_qoi_request,
  bool in_continuous_object_qoi,
  Info in_info,
  std::array<uint64_t, 3> in_grid_size,
  double in_object_jitter,
  std::string in_output_dir,
  std::string in_output_file_stem,
  double in_resolution,
  bool in_save_meshes,
  bool in_save_pngs,
  PhaseType in_selected_phase
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
, save_meshes_(in_save_meshes)
, save_pngs_(in_save_pngs)
, selected_phase_(in_selected_phase)
{
  if (selected_phase_ != std::numeric_limits<PhaseType>::max()) {
    n_phases_ = std::max(selected_phase_ + 1, n_phases_);
  }

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

  object_qoi_range_ = this->computeObjectQoiRange_();
  rank_qoi_range_ = this->computeRankQoiRange_();
};

double Render::computeMaxObjectVolume_() {
  // Initialize object volume limits
  double ov_max = -1 * std::numeric_limits<double>::infinity();
  double ov;
  double max_received_ov;
  double max_sent_ov;

  // Iterate over all ranks
  auto const& objects = this->info_.getAllObjects();
  for (auto const& [obj_id, obj_work] : objects) {
    // Update maximum object qoi
    auto ov_received = obj_work.getReceived();
    max_received_ov = std::max_element(
      std::begin(ov_received), std::end(ov_received),
      [] (const std::pair<double, double> & p1, const std::pair<double, double> & p2) {
        return p1.second < p2.second;
      }
    )->first;
    auto ov_sent = obj_work.getSent();
    max_sent_ov = std::max_element(
      std::begin(ov_sent), std::end(ov_sent),
      [] (const std::pair<double, double> & p1, const std::pair<double, double> & p2) {
        return p1.second < p2.second;
      }
    )->first;
    if (ov > ov_max) ov_max = ov;
  }
  return ov;
}

std::variant<std::pair<double, double>, std::set<double>> Render::computeObjectQoiRange_() {
  // Initialize object QOI range attributes
  double oq_max = -1 * std::numeric_limits<double>::infinity();
  double oq_min = std::numeric_limits<double>::infinity();
  double oq;
  std::set<double> oq_all;

  // Iterate over all ranks
  auto const& objects = this->info_.getAllObjects();
  for (auto const& [obj_id, obj_work] : objects) {
    // Update maximum object qoi
    if (this->object_qoi_ == "load") {
      oq = obj_work.getLoad();

      if (!continuous_object_qoi_) {
        oq_all.insert(oq);
        if(oq_all.size() > 20) {
          oq_all.clear();
          continuous_object_qoi_ = true;
        }
      }
    } else {
      throw std::runtime_error("Invalid QOI: " + this->object_qoi_);
    }
    if (oq > oq_max) oq_max = oq;
    if (oq < oq_min) oq_min = oq;
  }

  // Update extrema attribute
  this->object_qoi_max_ = oq_max;

  if (continuous_object_qoi_) {
    // return range
    return std::make_pair(oq_min, oq_max);
  } else {
    // return support
    return oq_all;
  }
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

double Render::computeRankQoiAverage_(PhaseType phase, std::string qoi) {
  // Initialize rank QOI range attributes
  double rq_sum = 0.0;

  if (qoi == "load"){
    auto rank_loads_at_phase = this->info_.getAllRankLoadsAtPhase(phase);
    for (auto [rank, rank_load] : rank_loads_at_phase){
      rq_sum += rank_load;
    }
    return rq_sum / rank_loads_at_phase.size();
  }
  else{
    throw std::runtime_error("Invalid QOI: " + qoi);
  }
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

void Render::get_rgb_from_colormap(int index, double& r, double& g, double& b) {
  const std::vector<std::tuple<double, double, double>> tab20_cmap = {
    {0.12156862745098039, 0.4666666666666667, 0.7058823529411765},
    {0.6823529411764706, 0.7803921568627451, 0.9098039215686274},
    {1.0, 0.4980392156862745, 0.054901960784313725},
    {1.0, 0.7333333333333333, 0.47058823529411764},
    {0.17254901960784313, 0.6274509803921569, 0.17254901960784313},
    {0.596078431372549, 0.8745098039215686, 0.5411764705882353},
    {0.8392156862745098, 0.15294117647058825, 0.1568627450980392},
    {1.0, 0.596078431372549, 0.5882352941176471},
    {0.5803921568627451, 0.403921568627451, 0.7411764705882353},
    {0.7725490196078432, 0.6901960784313725, 0.8352941176470589},
    {0.5490196078431373, 0.33725490196078434, 0.29411764705882354},
    {0.7686274509803922, 0.611764705882353, 0.5803921568627451},
    {0.8901960784313725, 0.4666666666666667, 0.7607843137254902},
    {0.9686274509803922, 0.7137254901960784, 0.8235294117647058},
    {0.4980392156862745, 0.4980392156862745, 0.4980392156862745},
    {0.7803921568627451, 0.7803921568627451, 0.7803921568627451},
    {0.7372549019607844, 0.7411764705882353, 0.13333333333333333},
    {0.8588235294117647, 0.8588235294117647, 0.5529411764705883},
    {0.09019607843137255, 0.7450980392156863, 0.8117647058823529},
    {0.6196078431372549, 0.8549019607843137, 0.8980392156862745}
  };
  if (index < 0 || index >= tab20_cmap.size()) {
    throw std::runtime_error("Index out of bounds for tab20 colormap.");
  }
  std::tie(r, g, b) = tab20_cmap[index];
}

/*static*/ vtkSmartPointer<vtkDiscretizableColorTransferFunction> Render::createColorTransferFunction(
  std::variant<std::pair<double, double>, std::set<double>> attribute_range, double attribute_avg, ColorType ct
) {
  vtkSmartPointer<vtkDiscretizableColorTransferFunction> ctf = vtkSmartPointer<vtkDiscretizableColorTransferFunction>::New();
  ctf->SetNanColorRGBA(1., 1., 1., 0.);
  ctf->UseBelowRangeColorOn();
  ctf->UseAboveRangeColorOn();

  // Make discrete when requested
  if(std::holds_alternative<std::set<double>>(attribute_range)) {
    const std::set<double>& values = std::get<std::set<double>>(attribute_range);
    // Handle the set type
    ctf->DiscretizeOn();
    int n_colors = values.size();
    ctf->IndexedLookupOn();
    ctf->SetNumberOfIndexedColors(n_colors);
    int i = 0;
    for (double v : values) {
      ctf->SetAnnotation(v, std::to_string(v));
      // Use discrete color map
      double r, g, b;
      get_rgb_from_colormap(i, r, g, b);
      const double rgb[3] = {r, g, b};
      ctf->SetIndexedColorRGB(i, rgb);
      i++;
    }
    ctf->Build();
    return ctf;
  } else if (std::holds_alternative<std::pair<double, double>>(attribute_range)) {
    const std::pair<double, double>& range = std::get<std::pair<double, double>>(attribute_range);
    switch (ct) {
    case BlueToRed: {
      ctf->SetColorSpaceToDiverging();
      double const mid_point = (range.first + range.second) * .5;
      ctf->AddRGBPoint(range.first, .231, .298, .753);
      ctf->AddRGBPoint(mid_point, .865, .865, .865);
      ctf->AddRGBPoint(range.second, .906, .016, .109);
      ctf->SetBelowRangeColor(0.0, 1.0, 0.0);
      ctf->SetAboveRangeColor(1.0, 0.0, 1.0);
      break;
    }
    case HotSpot: {
      ctf->SetColorSpaceToDiverging();
      double const mid_point = attribute_avg;
      ctf->AddRGBPoint(range.first, .231, .298, .753);
      ctf->AddRGBPoint(mid_point, .865, .865, .865);
      ctf->AddRGBPoint(range.second, .906, .016, .109);
      ctf->SetBelowRangeColor(0.0, 1.0, 1.0);
      ctf->SetAboveRangeColor(1.0, 1.0, 0.0);
      break;
    }
    case WhiteToBlack: {
      ctf->AddRGBPoint(range.first, 1.0, 1.0, 1.0);
      ctf->AddRGBPoint(range.second, 0.0, 0.0, 0.0);
      ctf->SetBelowRangeColor(0.0, 0.0, 1.0);
      ctf->SetAboveRangeColor(1.0, 0.0, 0.0);
      break;
    }
    case Default: {
      double const mid_point = (range.first + range.second) * .5;
      ctf->AddRGBPoint(range.first, .431, .761, .161);
      ctf->AddRGBPoint(mid_point, .98, .992, .059);
      ctf->AddRGBPoint(range.second, 1.0, .647, 0.0);
      ctf->SetBelowRangeColor(0.8, 0.8, .8);
      ctf->SetAboveRangeColor(1.0, 0.0, 1.0);
      break;
    }
    }
    return ctf;
  } else {
    throw std::runtime_error("Unexpected type in attribute_range variant.");
  }
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

/*static*/ void Render::createPipeline2(
  PhaseType phase,
  vtkPolyData* rank_mesh,
  vtkPolyData* object_mesh,
  double qoi_range[2],
  double load_range[2],
  double max_volume,
  double glyph_factor,
  int win_size,
  std::string output_dir,
  std::string output_file_stem
) {
  vtkNew<vtkRenderer> renderer;
  renderer->SetBackground(1.0, 1.0, 1.0);
  renderer->GetActiveCamera()->ParallelProjectionOn();

//   vtkNew<vtkGlyphSource2D> rank_glyph;
//   rank_glyph->SetGlyphTypeToSquare();
//   rank_glyph->SetScale(.95);
//   rank_glyph->FilledOn();
//   rank_glyph->CrossOff();
//   vtkNew<vtkGlyph2D> rank_glypher;
//   rank_glypher->SetSourceConnection(rank_glyph->GetOutputPort());
//   rank_glypher->SetInputData(rank_mesh);
//   rank_glypher->SetScaleModeToDataScalingOff();

//   //Lower glyphs slightly for visibility
//   vtkNew<vtkTransform> z_lower;
//   z_lower->Translate(0.0, 0.0, -0.01);
//   vtkNew<vtkTransformPolyDataFilter> trans;
//   trans->SetTransform(z_lower);
//   trans->SetInputConnection(rank_glypher->GetOutputPort());

//   vtkNew<vtkPolyDataMapper> rank_mapper;
//   rank_mapper->SetInputConnection(trans->GetOutputPort());
//   rank_mapper->SetLookupTable(createColorTransferFunction(qoi_range));
//   rank_mapper->SetScalarRange(qoi_range);

//   vtkNew<vtkActor> rank_actor;
//   rank_actor->SetMapper(rank_mapper);
//   auto qoi_actor = createScalarBarActor_(rank_mapper, "Rank XXX", 0.5, 0.9);
//   qoi_actor->DrawBelowRangeSwatchOn();
//   qoi_actor->SetBelowRangeAnnotation("<");
//   qoi_actor->DrawAboveRangeSwatchOn();
//   qoi_actor->SetAboveRangeAnnotation(">");
//   renderer->AddActor(rank_actor);
//   renderer->AddActor2D(qoi_actor);

//   vtkNew<vtkLookupTable> bw_lut;
//   double lut_range[2] = {0.0, max_volume};
//   bw_lut->SetTableRange(lut_range);
//   bw_lut->SetSaturationRange(0, 0);
//   bw_lut->SetHueRange(0, 0);
//   bw_lut->SetValueRange(1, 0);
//   bw_lut->SetNanColor(1.0, 1.0, 1.0, 0.0);
//   bw_lut->Build();

//   vtkNew<vtkArrayCalculator> sqrtT;
//   sqrtT->SetInputData(object_mesh);
//   sqrtT->AddScalarArrayName("Load");
//   char const* sqrtT_str = "sqrt(Load)";
//   sqrtT->SetFunction(sqrtT_str);
//   sqrtT->SetResultArrayName(sqrtT_str);
//   sqrtT->Update();
//   auto sqrtT_out = sqrtT->GetDataSetOutput();
//   sqrtT_out->GetPointData()->SetActiveScalars("Migratable");

//   std::vector<std::tuple<double, std::string>> items{{0.0, "Square"}, {1.0, "Circle"}};
//   vtkPolyDataMapper* glyph_mapper_out = nullptr;
//   for (auto&& [k,v] : items) {
//     vtkNew<vtkThresholdPoints> thresh;
//     thresh->SetInputData(sqrtT_out);
//     thresh->ThresholdBetween(k, k);
//     thresh->Update();
//     auto thresh_out = thresh->GetOutput();
//     if (not thresh_out->GetNumberOfPoints())
//       continue;
//     thresh_out->GetPointData()->SetActiveScalars(sqrtT_str);

//     // Glyph by square root of object loads
//     vtkNew<vtkGlyphSource2D> glyph;
//     if (v == "Square") {
//       glyph->SetGlyphTypeToSquare();
//     } else {
//       glyph->SetGlyphTypeToCircle();
//     }
//     glyph->SetResolution(32);
//     glyph->SetScale(1.0);
//     glyph->FilledOn();
//     glyph->CrossOff();
//     vtkNew<vtkGlyph3D> glypher;
//     glypher->SetSourceConnection(glyph->GetOutputPort());
//     glypher->SetInputData(thresh_out);
//     glypher->SetScaleModeToScaleByScalar();
//     glypher->SetScaleFactor(glyph_factor);
//     glypher->Update();
//     glypher->GetOutput()->GetPointData()->SetActiveScalars("Load");

//     // Raise glyphs slightly for visibility
//     vtkNew<vtkTransform> z_raise;
//     z_raise->Translate(0.0, 0.0, 0.01);
//     vtkNew<vtkTransformPolyDataFilter> trans;
//     trans->SetTransform(z_raise);
//     trans->SetInputData(glypher->GetOutput());

//     // Create mapper and actor for glyphs
//     vtkNew<vtkPolyDataMapper> glyph_mapper;
//     glyph_mapper_out = glyph_mapper;
//     glyph_mapper->SetInputConnection(trans->GetOutputPort());
//     glyph_mapper->SetLookupTable(createColorTransferFunction(load_range, 0, BlueToRed));
//     glyph_mapper->SetScalarRange(load_range);
//     vtkNew<vtkActor> glyph_actor;
//     glyph_actor->SetMapper(glyph_mapper);
//     renderer->AddActor(glyph_actor);
//   }

//   if (glyph_mapper_out) {
//     auto load_actor = createScalarBarActor_(glyph_mapper_out, "Object Load", 0.55, 0.55);
//     renderer->AddActor2D(load_actor);
//   }

//   renderer->ResetCamera();
//   vtkNew<vtkRenderWindow> render_window;
//   render_window->AddRenderer(renderer);
//   render_window->SetWindowName("LBAF");
//   render_window->SetSize(win_size, win_size);
//   render_window->Render();

//   vtkNew<vtkWindowToImageFilter> w2i;
//   w2i->SetInput(render_window);
//   w2i->SetScale(3);

//   vtkNew<vtkPNGWriter> writer;
//   writer->SetInputConnection(w2i->GetOutputPort());
//   std::string png_filename = output_dir + output_file_stem + std::to_string(phase) + ".png";
//   writer->SetFileName(png_filename.c_str());
//   writer->SetCompressionLevel(2);
//   writer->Write();
}

/* static */ vtkSmartPointer<vtkRenderer> Render::setupRenderer() {
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  renderer->SetBackground(1.0, 1.0, 1.0);  // Set background to white
  renderer->GetActiveCamera()->ParallelProjectionOn();
  return renderer;
}

/* static */ vtkSmartPointer<vtkActor> Render::createRanksActor(
  PhaseType phase,
  vtkPolyData* rank_mesh,
  std::variant<std::pair<double, double>, std::set<double>> rank_qoi_range
) {
  // Create square glyphs at ranks
  vtkSmartPointer<vtkGlyphSource2D> rank_glyph = vtkSmartPointer<vtkGlyphSource2D>::New();
  rank_glyph->SetGlyphTypeToSquare();
  rank_glyph->SetScale(0.95);
  rank_glyph->FilledOn();
  rank_glyph->CrossOff();
  vtkSmartPointer<vtkGlyph2D> rank_glypher = vtkSmartPointer<vtkGlyph2D>::New();
  rank_glypher->SetSourceConnection(rank_glyph->GetOutputPort());
  rank_glypher->SetInputData(rank_mesh);
  rank_glypher->SetScaleModeToDataScalingOff();

  // Lower glyphs slightly for visibility
  vtkSmartPointer<vtkTransform> z_lower = vtkSmartPointer<vtkTransform>::New();
  z_lower->Translate(0.0, 0.0, -0.01);
  vtkSmartPointer<vtkTransformPolyDataFilter> trans = vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  trans->SetTransform(z_lower);
  trans->SetInputConnection(rank_glypher->GetOutputPort());

  // Create mapper for rank glyphs
  vtkSmartPointer<vtkPolyDataMapper> rank_mapper = vtkSmartPointer<vtkPolyDataMapper>::New();
  rank_mapper->SetInputConnection(trans->GetOutputPort());
  rank_mapper->SetLookupTable(createColorTransferFunction(rank_qoi_range, BlueToRed));
  // Check the type held by the variant qoi range and set the scalar range appropriately
  if (std::holds_alternative<std::pair<double, double>>(rank_qoi_range)) {
    auto range_pair = std::get<std::pair<double, double>>(rank_qoi_range);
    rank_mapper->SetScalarRange(range_pair.first, range_pair.second);
  } else if (std::holds_alternative<std::set<double>>(rank_qoi_range)) {
    const auto& range_set = std::get<std::set<double>>(rank_qoi_range);
    if (!range_set.empty()) {
      rank_mapper->SetScalarRange(*range_set.begin(), *range_set.rbegin());
    } else {
      rank_mapper->SetScalarRange(0., 0.);
    }
  } else {
    // Handle unexpected type or set a default scalar range
    throw std::runtime_error("Unexpected type in rank qoi range variant.");
  }

  vtkSmartPointer<vtkActor> rank_actor = vtkSmartPointer<vtkActor>::New();
  rank_actor->SetMapper(rank_mapper);

  return rank_actor;
}



void Render::createPipeline(
  PhaseType phase,
  vtkPolyData* rank_mesh,
  vtkPolyData* object_mesh,
  uint64_t edge_width,
  double max_volume,
  double glyph_factor,
  int win_size,
  std::string output_dir,
  std::string output_file_stem
) {
  vtkSmartPointer<vtkRenderer> renderer = this->setupRenderer();

  std::variant<std::pair<double, double>, std::set<double>> rank_qoi_variant(rank_qoi_range_);
  vtkSmartPointer<vtkActor> rank_actor = this->createRanksActor(
    phase,
    rank_mesh,
    rank_qoi_variant
  );
  renderer->AddActor(rank_actor);

  // // Scalar bar for rank
  // vtkSmartPointer<vtkActor2D> qoi_actor = this->createScalarBar(rank_mapper, "rank", 0.5, 0.9);
  // renderer->AddActor2D(qoi_actor);

  // // Object glyphs and associated components (only created if object_mesh is provided)
  // if (object_mesh) {
  //   vtkSmartPointer<vtkActor> edge_actor = this->createObjectEdgeActor(object_mesh, edge_width);
  //   renderer->AddActor(edge_actor);

  //   vtkSmartPointer<vtkGlyph3D> object_glypher = this->createObjectGlyphs(object_mesh, glyph_factor);
  //   vtkSmartPointer<vtkPolyDataMapper> glyph_mapper = this->setupObjectGlyphMapper(object_glypher);
  //   vtkSmartPointer<vtkActor> glyph_actor = this->createObjectGlyphActor(glyph_mapper);
  //   renderer->AddActor(glyph_actor);

  //   // Scalar bar for objects
  //   vtkSmartPointer<vtkActor2D> load_actor = this->createObjectScalarBar(glyph_mapper, "object");
  //   renderer->AddActor2D(load_actor);
  // }

  // // Text actor
  // vtkSmartPointer<vtkTextActor> text_actor = this->createTextActor(iteration, p_id);
  // renderer->AddActor(text_actor);

  // return setupRenderWindow(renderer, win_size);

  renderer->ResetCamera();
  vtkNew<vtkRenderWindow> render_window;
  render_window->AddRenderer(renderer);
  render_window->SetWindowName("vt-tv");
  render_window->SetSize(win_size, win_size);
  render_window->Render();

  vtkNew<vtkWindowToImageFilter> w2i;
  w2i->SetInput(render_window);
  w2i->SetScale(3);

  vtkNew<vtkPNGWriter> writer;
  writer->SetInputConnection(w2i->GetOutputPort());
  std::string png_filename = output_dir + output_file_stem + std::to_string(phase) + ".png";
  writer->SetFileName(png_filename.c_str());
  writer->SetCompressionLevel(2);
  writer->Write();
}

void Render::generate() {
  std::pair<double, double> rank_qoi_range = this->computeRankQoiRange_();
  double rank_qoi_min = std::get<0>(rank_qoi_range);
  double rank_qoi_max = std::get<1>(rank_qoi_range);

  if (std::holds_alternative<std::pair<double, double>>(object_qoi_range_)) {
    auto range_pair = std::get<std::pair<double, double>>(object_qoi_range_);
    double object_qoi_min = std::get<0>(range_pair);
    double object_qoi_max = std::get<1>(range_pair);
    fmt::print("Rank {} range: {}, {}\n", rank_qoi_, rank_qoi_min, rank_qoi_max);
    fmt::print("Object {} range: {}, {}\n", object_qoi_, object_qoi_min, object_qoi_max);
  }

  fmt::print("selected phase={}\n", selected_phase_);

  for(PhaseType phase = 0; phase < this->n_phases_; phase++) {
    if (
      selected_phase_ == std::numeric_limits<PhaseType>::max() or
      selected_phase_ == phase
    ) {

      this->info_.normalizeEdges(phase);

      vtkNew<vtkPolyData> object_mesh = this->createObjectMesh_(phase);
      vtkNew<vtkPolyData> rank_mesh = this->createRankMesh_(phase);

      if (save_meshes_){
        fmt::print("Writing object mesh for phase {}\n", phase);
        vtkNew<vtkXMLPolyDataWriter> writer;
        std::string object_mesh_filename = output_dir_ + output_file_stem_ + "_object_mesh_" + std::to_string(phase) + ".vtp";
        writer->SetFileName(object_mesh_filename.c_str());
        writer->SetInputData(object_mesh);
        writer->Write();

        fmt::print("Writing rank mesh for phase {}\n", phase);
        vtkNew<vtkXMLPolyDataWriter> writer2;
        std::string rank_mesh_filneame = output_dir_ + output_file_stem_ + "_rank_mesh_" + std::to_string(phase) + ".vtp";
        writer2->SetFileName(rank_mesh_filneame.c_str());
        writer2->SetInputData(rank_mesh);
        writer2->Write();
      }

      if (save_pngs_){
        fmt::print("Rendering visualization PNG for phase {}\n", phase);

        std::pair<double, double> obj_qoi_range;
        try {
          obj_qoi_range = std::get<std::pair<double, double>>(this->object_qoi_range_);
        }
        catch(const std::exception& e) {
          std::cerr << e.what() << '\n';
          obj_qoi_range = {0, 1};
        }
        auto load_range = this->computeRankQoiRange_();

        double obj_qoi_range_in[2] = {obj_qoi_range.first, obj_qoi_range.second};
        double load_range_in[2] = {load_range.first, load_range.second};
        createPipeline(
          phase,
          rank_mesh,
          object_mesh,
          10,
          100,
          1,
          1080,
          output_dir_,
          output_file_stem_
        );
      }
    }
  }
}

}} /* end namesapce vt::tv */
