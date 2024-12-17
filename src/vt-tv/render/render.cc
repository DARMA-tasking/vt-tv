/*
//@HEADER
// *****************************************************************************
//
//                                  render.cc
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

#include "vt-tv/render/render.h"

// #include <nanobind/nanobind.h>

// namespace nb = nanobind;

// using namespace nb::literals;

namespace vt::tv {

Render::Render(Info in_info)
  : info_(in_info) // std:move ?
    ,
    n_ranks_(in_info.getNumRanks()),
    n_phases_(in_info.getNumPhases()) {
  // If selected_phase is not provided, use all phases
  selected_phase_ = std::numeric_limits<PhaseType>::max();

  // Generically set rank grid dimensions according to the total number of ranks
  grid_size_[2] = 1; // we assume 2D representation

  uint64_t sqrt_n_ranks =
    static_cast<uint64_t>(std::sqrt(static_cast<double>(n_ranks_)));
  if (sqrt_n_ranks * sqrt_n_ranks == n_ranks_) {
    // n_ranks is a perfect square
    grid_size_[0] = sqrt_n_ranks;
    grid_size_[1] = sqrt_n_ranks;
  } else {
    // n_ranks is not a perfect square
    grid_size_[0] = sqrt_n_ranks;     // floor
    grid_size_[1] = sqrt_n_ranks + 1; // ceil
  }

  for (uint64_t d = 0; d < 3; d++) {
    if (grid_size_[d] > 1)
      rank_dims_.insert(d);
  }
  max_o_per_dim_ = 0;

  // Set the info selected_phase
  this->info_.setSelectedPhase(selected_phase_);

  // Normalize communication edges
  if (selected_phase_ != std::numeric_limits<PhaseType>::max()) {
    this->info_.normalizeEdges(selected_phase_);
  } else {
    for (PhaseType phase = 0; phase < this->n_phases_; phase++) {
      this->info_.normalizeEdges(phase);
    }
  }

  // Initialize jitter
  std::srand(std::time(nullptr));
  auto const& allObjects = info_.getAllObjectIDs();
  for (auto const& objectID : allObjects) {
    std::array<double, 3> jitterDims;
    for (uint64_t d = 0; d < 3; d++) {
      if (auto f = this->rank_dims_.find(d); f != this->rank_dims_.end()) {
        jitterDims[d] = ((double)std::rand() / RAND_MAX - 0.5) * object_jitter_;
      } else
        jitterDims[d] = 0;
    }
    jitter_dims_.insert(std::make_pair(objectID, jitterDims));
  }

  object_qoi_range_ = this->computeObjectQOIRange_();
  rank_qoi_range_ = this->computeRankQOIRange_();
  object_volume_max_ = this->computeMaxObjectVolume_();
  object_load_max_ = this->info_.getMaxLoad();
};

Render::Render(
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
  PhaseType in_selected_phase)
  : rank_qoi_(in_qoi_request[0]),
    object_qoi_(in_qoi_request[2]),
    continuous_object_qoi_(in_continuous_object_qoi),
    info_(in_info),
    n_ranks_(in_info.getNumRanks()),
    n_phases_(in_info.getNumPhases()),
    grid_size_(in_grid_size),
    object_jitter_(in_object_jitter),
    output_dir_(in_output_dir),
    output_file_stem_(in_output_file_stem),
    grid_resolution_(in_resolution),
    save_meshes_(in_save_meshes),
    save_pngs_(in_save_pngs),
    selected_phase_(in_selected_phase) {
  // initialize number of ranks
  n_ranks_ = info_.getNumRanks();

  // initialize rank dimensions according to given grid
  for (uint64_t d = 0; d < 3; d++) {
    if (grid_size_[d] > 1)
      rank_dims_.insert(d);
  }
  max_o_per_dim_ = 0;

  // Set the info selected_phase
  this->info_.setSelectedPhase(selected_phase_);

  // Normalize communication edges
  if (selected_phase_ != std::numeric_limits<PhaseType>::max()) {
    this->info_.normalizeEdges(selected_phase_);
  } else {
    for (PhaseType phase = 0; phase < this->n_phases_; phase++) {
      this->info_.normalizeEdges(phase);
    }
  }

  // Initialize jitter
  std::srand(std::time(nullptr));
  auto const& allObjects = info_.getAllObjectIDs();
  for (auto const& objectID : allObjects) {
    std::array<double, 3> jitterDims;
    for (uint64_t d = 0; d < 3; d++) {
      if (auto f = this->rank_dims_.find(d); f != this->rank_dims_.end()) {
        jitterDims[d] = ((double)std::rand() / RAND_MAX - 0.5) * object_jitter_;
      } else
        jitterDims[d] = 0;
    }
    jitter_dims_.insert(std::make_pair(objectID, jitterDims));
  }

  object_qoi_range_ = this->computeObjectQOIRange_();
  rank_qoi_range_ = this->computeRankQOIRange_();
  object_volume_max_ = this->computeMaxObjectVolume_();
  object_load_max_ = this->info_.getMaxLoad();
};

double Render::computeMaxObjectVolume_() {
  double ov_max = this->info_.getMaxVolume();
  return ov_max;
}

std::variant<std::pair<double, double>, std::set<std::variant<double, int>>>
Render::computeObjectQOIRange_() {
  // Initialize object QOI range attributes
  double oq_max = -1 * std::numeric_limits<double>::infinity();
  double oq_min = std::numeric_limits<double>::infinity();
  std::set<std::variant<double, int>> oq_all;

  // Update the QOI range
  auto updateQOIRange = [&](auto const& objects, PhaseType phase) {
    for (auto const& [obj_id, obj_work] : objects) {
      // Update maximum object qoi
      auto oq = info_.getObjectQOIAtPhase<double>(obj_id, phase, this->object_qoi_);
      if (!continuous_object_qoi_) {
        // Allow for integer categorical QOI (i.e. rank_id)
        if (oq == static_cast<int>(oq)) {
          oq_all.insert(static_cast<int>(oq));
        } else {
          oq_all.insert(oq);
        }
        if (oq_all.size() > 20) {
          oq_all.clear();
          continuous_object_qoi_ = true;
        }
      }
      if (oq > oq_max)
        oq_max = oq;
      if (oq < oq_min)
        oq_min = oq;
    }
  };

  // Iterate over all ranks
  if (selected_phase_ != std::numeric_limits<PhaseType>::max()) {
    auto const& objects = this->info_.getPhaseObjects(selected_phase_);
    updateQOIRange(objects, selected_phase_);
  } else {
    for (PhaseType phase = 0; phase < this->n_phases_; phase++) {
      auto const& objects = this->info_.getPhaseObjects(phase);
      updateQOIRange(objects, phase);
    }
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

std::pair<double, double> Render::computeRankQOIRange_() {
  // Initialize rank QOI range attributes
  double rq_max = -1 * std::numeric_limits<double>::infinity();
  double rq_min = std::numeric_limits<double>::infinity();
  double rqmax_for_phase;
  double rqmin_for_phase;

  // Iterate over all ranks
  for (uint64_t rank_id = 0; rank_id < this->n_ranks_; rank_id++) {
    auto rank_qoi_map = info_.getAllQOIAtRank(rank_id, this->rank_qoi_);

    // Get max qoi for this rank across all phases
    auto prmax = std::max_element(
      std::begin(rank_qoi_map),
      std::end(rank_qoi_map),
      [](auto const& p1, auto const& p2) { return p1.second < p2.second; }
    );
    rqmax_for_phase = prmax->second;

    // Get min qoi for this rank across all phases
    auto prmin = std::max_element(
      std::begin(rank_qoi_map),
      std::end(rank_qoi_map),
      [](auto const& p1, auto const& p2) { return p1.second > p2.second; }
    );
    rqmin_for_phase = prmin->second;

    if (rqmax_for_phase > rq_max)
      rq_max = rqmax_for_phase;
    if (rqmin_for_phase < rq_min)
      rq_min = rqmin_for_phase;
  }

  // return range
  return std::make_pair(rq_min, rq_max);
}

double Render::computeRankQOIAverage_(PhaseType phase, std::string qoi) {
  // Initialize rank QOI range attributes
  double rq_sum = 0.0;
  auto const& rank_loads_at_phase =
    this->info_.getAllRankQOIAtPhase<double>(phase, qoi);
  for (auto const& [rank, rank_load] : rank_loads_at_phase) {
    rq_sum += rank_load;
  }
  return rq_sum / rank_loads_at_phase.size();
}

std::map<NodeType, std::unordered_map<ElementIDType, ObjectWork>>
Render::createObjectMapping_(PhaseType phase) {
  std::map<NodeType, std::unordered_map<ElementIDType, ObjectWork>>
    object_mapping;
  // Add each rank and its corresponding objects at the given phase to the object mapping
  for (uint64_t rank_id = 0; rank_id < this->n_ranks_; rank_id++) {
    object_mapping.insert(
      std::make_pair(rank_id, this->info_.getRankObjects(rank_id, phase)));
  }
  return object_mapping;
}

template <typename T, typename U>
vtkNew<U> Render::createRankArrayUserDefined(
  PhaseType phase, std::string const& key
) {
  vtkNew<U> array;
  std::string array_name = key;
  array->SetName(array_name.c_str());
  array->SetNumberOfTuples(n_ranks_);

  for (uint64_t rank_id = 0; rank_id < n_ranks_; rank_id++) {
    auto const& cur_rank_info = info_.getRanks().at(rank_id);
    auto const& value = info_.getRankUserDefined(cur_rank_info, phase, key);
    //fmt::print("phase={}, key={}, rank_id={}\n", phase, key, rank_id);
    array->SetTuple1(rank_id, std::get<T>(value));
  }
  return array;
}

template <typename T, typename U>
vtkNew<U> Render::createRankArrayComputed(
  PhaseType phase, std::string const& key
) {
  vtkNew<U> array;
  std::string array_name = key;
  array->SetName(array_name.c_str());
  array->SetNumberOfTuples(n_ranks_);

  for (uint64_t rank_id = 0; rank_id < n_ranks_; rank_id++) {
    array->SetTuple1(rank_id, info_.getRankQOIAtPhase<T>(rank_id, phase, key));
  }
  return array;
}

vtkNew<vtkPolyData> Render::createRankMesh_(PhaseType phase) {
  fmt::print("\n\n");
  fmt::print("----- Creating rank mesh for phase {} -----\n", phase);
  vtkNew<vtkPoints> rank_points_;
  rank_points_->SetNumberOfPoints(n_ranks_);

  for (uint64_t rank_id = 0; rank_id < n_ranks_; rank_id++) {
    std::array<uint64_t, 3> cartesian =
      globalIDToCartesian_(rank_id, grid_size_);
    std::array<double, 3> offsets = {
      cartesian[0] * grid_resolution_,
      cartesian[1] * grid_resolution_,
      cartesian[2] * grid_resolution_
    };

    // Insert point based on cartesian coordinates
    rank_points_->SetPoint(rank_id, offsets[0], offsets[1], offsets[2]);
  }

  vtkNew<vtkPolyData> pd_mesh;
  pd_mesh->SetPoints(rank_points_);

  // First, check user-defined to see if we already have the QOI calculated, if
  // so grab that first value to determine the type of the variant to allocate
  // the VTK array
  auto const has_user_defined_qoi = info_.hasRankUserDefined(rank_qoi_);
  if (has_user_defined_qoi) {
    auto const& test_value = info_.getFirstRankUserDefined(rank_qoi_);
    if (std::holds_alternative<double>(test_value)) {
      pd_mesh->GetPointData()->SetScalars(
        createRankArrayUserDefined<double, vtkDoubleArray>(phase, rank_qoi_)
      );
    } else if (std::holds_alternative<int>(test_value)) {
      pd_mesh->GetPointData()->SetScalars(
        createRankArrayUserDefined<int, vtkIntArray>(phase, rank_qoi_)
      );
    }
  } else {
    // We need to calculate the QOI since it's not in user-defined
    // Lookup the type in a map to determine which vtk type array to utilize
    auto const& types = info_.computable_qoi_types;
    if (auto iter = types.find(rank_qoi_); iter != types.end()) {
      VtkTypeEnum type = iter->second;
      if (type == VtkTypeEnum::TYPE_DOUBLE) {
        pd_mesh->GetPointData()->SetScalars(
          createRankArrayComputed<double, vtkDoubleArray>(phase, rank_qoi_)
        );
      } else if (type == VtkTypeEnum::TYPE_INT) {
        pd_mesh->GetPointData()->SetScalars(
          createRankArrayComputed<int, vtkIntArray>(phase, rank_qoi_)
        );
      }
    }
  }

  auto const& rank_info = info_.getRanks().at(0);
  auto const& keys = info_.getRankUserDefinedKeys(rank_info, phase);
  for (auto const& key : keys) {
    auto const& test_value = info_.getRankUserDefined(rank_info, phase, key);
    if (std::holds_alternative<double>(test_value)) {
      pd_mesh->GetPointData()->AddArray(
        createRankArrayUserDefined<double, vtkDoubleArray>(phase, key)
      );
    } else if (std::holds_alternative<int>(test_value)) {
      pd_mesh->GetPointData()->AddArray(
        createRankArrayUserDefined<int, vtkIntArray>(phase, key)
      );
    }
  }

  fmt::print("----- Finished creating rank mesh for phase {} -----\n", phase);
  return pd_mesh;
}

bool compareObjects(
  const std::pair<ObjectWork, uint64_t>& p1,
  const std::pair<ObjectWork, uint64_t>& p2) {
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
  q_arr->SetName(this->object_qoi_.c_str());
  q_arr->SetNumberOfTuples(n_o);

  // Load array must be added when it is not the object QOI
  vtkNew<vtkDoubleArray> l_arr;
  if (this->object_qoi_ != "load") {
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
  std::string object_qoi = this->object_qoi_;

  // Iterate over ranks and objects to create mesh points
  uint64_t point_index = 0;
  std::map<ElementIDType, uint64_t> objectid_to_index;
  // sent_volumes is a vector to store the communications ("from" object id, "sent to" object id, and volume)
  std::vector<std::tuple<ElementIDType, ElementIDType, double>> sent_volumes;

  auto object_mapping = this->createObjectMapping_(phase);

  std::map<std::string, VtkTypeEnum> qoi_map;

  // Iterate through object mapping
  for (auto const& [rankID, objects] : object_mapping) {
    std::array<uint64_t, 3> ijk =
      this->globalIDToCartesian_(rankID, this->grid_size_);

    std::array<double, 3> offsets = {
      ijk[0] * this->grid_resolution_,
      ijk[1] * this->grid_resolution_,
      ijk[2] * this->grid_resolution_};

    // Compute local object block parameters
    uint64_t n_o_rank = objects.size();

    uint64_t n_o_per_dim = ceil(pow(n_o_rank, 1.0 / this->rank_dims_.size()));
    if (n_o_per_dim > this->max_o_per_dim_) {
      this->max_o_per_dim_ = n_o_per_dim;
    }
    double o_resolution = this->grid_resolution_ / (n_o_per_dim + 1.);

    // Create point coordinates
    std::array<uint64_t, 3> rank_size = {1, 1, 1};
    for (uint64_t d = 0; d < 3; d++) {
      if (auto f = this->rank_dims_.find(d); f != this->rank_dims_.end()) {
        rank_size[d] = n_o_per_dim;
      } else
        rank_size[d] = 1;
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
      for (auto const& [key, _] : objectWork.getUserDefined()) {
        auto const& value = objectWork.getUserDefined().at(key);
        VtkTypeEnum t = VtkTypeEnum::TYPE_DOUBLE;
        if (std::holds_alternative<int>(value)) {
          t = VtkTypeEnum::TYPE_INT;
        }
        qoi_map[key] = t;
      }
    }

    // Sort objects
    std::sort(ordered_objects.begin(), ordered_objects.end(), compareObjects);

    // Add rank objects to point set
    int i = 0;
    for (auto const& [objectWork, migratable] : ordered_objects) {
      // fmt::print("Object ID: {}, migratable: {}\n", objectWork.getID(), migratable);

      // Insert point using offset and rank coordinates
      std::array<double, 3> currentPointPosition = {0, 0, 0};
      int d = 0;
      for (auto c : this->globalIDToCartesian_(i, rank_size)) {
        currentPointPosition[d] = offsets[d] - centering[d] +
          (jitter_dims_.at(objectWork.getID())[d] + c) * o_resolution;
        d++;
      }

      points->SetPoint(
        point_index,
        currentPointPosition[0],
        currentPointPosition[1],
        currentPointPosition[2]);

      // Set object attributes
      ElementIDType obj_id = objectWork.getID();
      auto oq = info_.getObjectQOIAtPhase<double>(obj_id, phase, object_qoi_);
      q_arr->SetTuple1(point_index, oq);
      b_arr->SetTuple1(point_index, migratable);
      if (this->object_qoi_ != "load") {
        l_arr->SetTuple1(point_index, objectWork.getLoad());
      }

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
  std::
    map<std::tuple<ElementIDType, ElementIDType>, std::tuple<uint64_t, double>>
      edge_values;

  fmt::print("  Creating inter-object communication edges\n");
  for (auto& [pt_index, k, v] : sent_volumes) {
    // sort the point index and the object id in the "ij" tuple
    std::tuple<ElementIDType, ElementIDType> ij;
    if (pt_index <= objectid_to_index.at(k)) {
      ij = {pt_index, objectid_to_index.at(k)};
    } else {
      ij = {objectid_to_index.at(k), pt_index};
    }

    std::tuple<uint64_t, double> edge_value;
    if (auto e_ij = edge_values.find(ij); e_ij != edge_values.end()) {
      // If the line already exist we just update the volume
      auto current_edge = std::get<0>(edge_values.at(ij));
      auto current_v = std::get<1>(edge_values.at(ij));
      edge_values.at(ij) = {current_edge, current_v + v};
      lineValuesArray->SetTuple1(
        std::get<0>(edge_values.at(ij)), std::get<1>(edge_values.at(ij)));
      // fmt::print("\tupdating edge {} ({}--{}): {}\n", current_edge, std::get<0>(ij), std::get<1>(ij), current_v+v);
    } else {
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
  if (this->object_qoi_ != "load") {
    pd_mesh->GetPointData()->AddArray(l_arr);
  }
  pd_mesh->GetCellData()->SetScalars(lineValuesArray);

  for (auto const& [key, vtk_type] : qoi_map) {
    if (vtk_type == VtkTypeEnum::TYPE_DOUBLE) {
      addObjectArray<double, vtkDoubleArray>(pd_mesh, phase, key);
    } else if (vtk_type == VtkTypeEnum::TYPE_INT) {
      addObjectArray<int, vtkIntArray>(pd_mesh, phase, key);
    }
  }

  fmt::print("----- Finished creating object mesh -----\n");

  return pd_mesh;
}

template <typename T, typename U>
void Render::addObjectArray(
  vtkNew<vtkPolyData>& pd_mesh, PhaseType phase, std::string const& key
) {
  auto const num_objects = info_.getPhaseObjects(phase).size();

  vtkNew<U> array;
  array->SetName(key.c_str());
  array->SetNumberOfTuples(num_objects);

  int point_index = 0;
  auto object_mapping = createObjectMapping_(phase);
  for (auto const& [rankID, objects] : object_mapping) {
    std::vector<std::pair<ObjectWork, uint64_t>> ordered_objects;
    for (auto const& [objectID, objectWork] : objects) {
      bool migratable = info_.getObjectInfo().at(objectID).isMigratable();
      ordered_objects.push_back(std::make_pair(objectWork, migratable));
    }
    std::sort(ordered_objects.begin(), ordered_objects.end(), compareObjects);

    // Add rank objects to point set
    for (auto const& [objectWork, migratable] : ordered_objects) {
      if (
        auto it = objectWork.getUserDefined().find(key);
        it != objectWork.getUserDefined().end()
      ) {
        //fmt::print("phase={}, key={}, id={}\n", phase, key, objectWork.getID());
        auto const& value = it->second;
        array->SetTuple1(point_index, std::get<T>(value));
      } else {
        array->SetTuple1(point_index, T{});
      }
      point_index++;
    }
  }

  pd_mesh->GetPointData()->AddArray(array);
}

void Render::getRgbFromTab20Colormap_(
  int index, double& r, double& g, double& b) {
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
    {0.6196078431372549, 0.8549019607843137, 0.8980392156862745}};
  if (index < 0 || static_cast<std::size_t>(index) >= tab20_cmap.size()) {
    throw std::runtime_error("Index out of bounds for tab20 colormap.");
  }
  std::tie(r, g, b) = tab20_cmap[index];
}

/*static*/ vtkSmartPointer<vtkDiscretizableColorTransferFunction>
Render::createColorTransferFunction_(
  std::variant<std::pair<double, double>, std::set<std::variant<double, int>>>
    attribute_range,
  ColorType ct) {
  vtkSmartPointer<vtkDiscretizableColorTransferFunction> ctf =
    vtkSmartPointer<vtkDiscretizableColorTransferFunction>::New();
  ctf->SetNanColorRGBA(1., 1., 1., 0.);
  ctf->UseBelowRangeColorOn();
  ctf->UseAboveRangeColorOn();

  // Make discrete when requested
  if (std::holds_alternative<std::set<std::variant<double, int>>>(
        attribute_range)) {
    const std::set<std::variant<double, int>>& values =
      std::get<std::set<std::variant<double, int>>>(attribute_range);

    // Handle the set type
    ctf->DiscretizeOn();
    int n_colors = values.size();
    ctf->IndexedLookupOn();
    ctf->SetNumberOfIndexedColors(n_colors);
    int i = 0;
    for (auto v : values) {
      std::visit(
        [&ctf](auto&& val) { ctf->SetAnnotation(val, std::to_string(val)); },
        v);
      // Use discrete color map
      double r, g, b;
      getRgbFromTab20Colormap_(i, r, g, b);
      const double rgb[3] = {r, g, b};
      ctf->SetIndexedColorRGB(i, rgb);
      i++;
    }
    ctf->Build();
    return ctf;
  } else if (std::holds_alternative<std::pair<double, double>>(
               attribute_range)) {
    const std::pair<double, double>& range =
      std::get<std::pair<double, double>>(attribute_range);
    switch (ct) {
    case ColorType::BlueToRed: {
      ctf->SetColorSpaceToDiverging();
      double const mid_point = (range.first + range.second) * .5;
      ctf->AddRGBPoint(range.first, .231, .298, .753);
      ctf->AddRGBPoint(mid_point, .865, .865, .865);
      ctf->AddRGBPoint(range.second, .906, .016, .109);
      ctf->SetBelowRangeColor(0.0, 1.0, 0.0);
      ctf->SetAboveRangeColor(1.0, 0.0, 1.0);
      break;
    }
    case ColorType::HotSpot: {
      ctf->SetColorSpaceToDiverging();
      double const mid_point1 = (range.second - range.first) * 0.25;
      double const mid_point2 = (range.second - range.first) * 0.75;

      ctf->AddRGBPoint(range.first, 0.0, 0.0, 1.0);  // Blue
      ctf->AddRGBPoint(mid_point1, 0.0, 1.0, 0.0);   // Green
      ctf->AddRGBPoint(mid_point2, 1.0, 0.8, 0.0);   // Orange
      ctf->AddRGBPoint(range.second, 1.0, 0.0, 0.0); // Red

      ctf->SetBelowRangeColor(0.0, 1.0, 1.0); // Cyan
      ctf->SetAboveRangeColor(1.0, 1.0, 0.0); // Yellow
      break;
    }
    case ColorType::WhiteToBlack: {
      ctf->AddRGBPoint(range.first, 1.0, 1.0, 1.0);
      ctf->AddRGBPoint(range.second, 0.0, 0.0, 0.0);
      ctf->SetBelowRangeColor(0.0, 0.0, 1.0);
      ctf->SetAboveRangeColor(1.0, 0.0, 0.0);
      break;
    }
    case ColorType::Default: {
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

/*static*/ vtkSmartPointer<vtkScalarBarActor> Render::createScalarBarActor_(
  vtkSmartPointer<vtkMapper> mapper,
  const std::string& title,
  double x,
  double y,
  uint64_t font_size,
  std::set<std::variant<double, int>> values) {
  vtkSmartPointer<vtkScalarBarActor> scalar_bar_actor =
    vtkSmartPointer<vtkScalarBarActor>::New();
  scalar_bar_actor->SetLookupTable(mapper->GetLookupTable());

  // Set default parameters
  scalar_bar_actor->SetOrientationToHorizontal();
  scalar_bar_actor->UnconstrainedFontSizeOn();
  scalar_bar_actor->SetHeight(0.08);
  scalar_bar_actor->SetWidth(0.42);
  scalar_bar_actor->SetBarRatio(0.3);
  scalar_bar_actor->DrawTickLabelsOn();
  scalar_bar_actor->SetLabelFormat("%.2G");

  if (!values.empty()) {
    scalar_bar_actor->SetNumberOfLabels(values.size());
    scalar_bar_actor->SetAnnotationLeaderPadding(8);

    std::string formatted_title = title;
    std::replace(formatted_title.begin(), formatted_title.end(), '_', ' ');
    std::string title_with_newline = formatted_title + '\n';
    scalar_bar_actor->SetTitle(title_with_newline.c_str());
  } else {
    scalar_bar_actor->SetNumberOfLabels(2);
    std::string formatted_title = title;
    std::replace(formatted_title.begin(), formatted_title.end(), '_', ' ');
    scalar_bar_actor->SetTitle(formatted_title.c_str());
  }

  // Modify properties for all text in scalar bar actor
  std::vector<vtkTextProperty*> properties;
  properties.push_back(scalar_bar_actor->GetTitleTextProperty());
  properties.push_back(scalar_bar_actor->GetLabelTextProperty());
  properties.push_back(scalar_bar_actor->GetAnnotationTextProperty());

  for (vtkTextProperty* prop : properties) {
    prop->SetColor(0.0, 0.0, 0.0);
    prop->ItalicOff();
    prop->BoldOff();
    prop->SetFontFamilyToArial();
    prop->SetFontSize(font_size);
  }

  // Set custom parameters
  vtkCoordinate* position = scalar_bar_actor->GetPositionCoordinate();
  position->SetCoordinateSystemToNormalizedViewport();
  position->SetValue(x, y, 0.0);

  return scalar_bar_actor;
}

/* static */ std::array<uint64_t, 3> Render::globalIDToCartesian_(
  uint64_t flat_id, std::array<uint64_t, 3> grid_sizes) {
  std::array<uint64_t, 3> cartesian = {0, 0, 0};
  // Sanity check
  uint64_t n01 = grid_sizes[0] * grid_sizes[1];
  if (flat_id >= n01 * grid_sizes[2]) {
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

/* static */ vtkSmartPointer<vtkRenderer> Render::setupRenderer_() {
  vtkSmartPointer<vtkRenderer> renderer = vtkSmartPointer<vtkRenderer>::New();
  renderer->SetBackground(1.0, 1.0, 1.0); // Set background to white
  renderer->GetActiveCamera()->ParallelProjectionOn();
  return renderer;
}

/* static */ vtkSmartPointer<vtkMapper> Render::createRanksMapper_(
  vtkPolyData* rank_mesh,
  std::variant<std::pair<double, double>, std::set<std::variant<double, int>>>
    rank_qoi_range) {
  // Create square glyphs at ranks
  vtkSmartPointer<vtkGlyphSource2D> rank_glyph =
    vtkSmartPointer<vtkGlyphSource2D>::New();
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
  vtkSmartPointer<vtkTransformPolyDataFilter> trans =
    vtkSmartPointer<vtkTransformPolyDataFilter>::New();
  trans->SetTransform(z_lower);
  trans->SetInputConnection(rank_glypher->GetOutputPort());

  // Create mapper for rank glyphs
  vtkSmartPointer<vtkPolyDataMapper> rank_mapper =
    vtkSmartPointer<vtkPolyDataMapper>::New();
  rank_mapper->SetInputConnection(trans->GetOutputPort());
  rank_mapper->SetLookupTable(
    createColorTransferFunction_(rank_qoi_range, ColorType::BlueToRed));
  // Check the type held by the variant qoi range and set the scalar range appropriately
  if (std::holds_alternative<std::pair<double, double>>(rank_qoi_range)) {
    auto range_pair = std::get<std::pair<double, double>>(rank_qoi_range);
    rank_mapper->SetScalarRange(range_pair.first, range_pair.second);
  } else if (std::holds_alternative<std::set<std::variant<double, int>>>(
               rank_qoi_range)) {
    const auto& range_set =
      std::get<std::set<std::variant<double, int>>>(rank_qoi_range);
    if (!range_set.empty()) {
      auto range_begin = *range_set.begin();
      auto range_end = *range_set.rbegin();
      if (std::holds_alternative<int>(range_begin)) {
        rank_mapper->SetScalarRange(
          std::get<int>(range_begin), std::get<int>(range_end));
      } else {
        rank_mapper->SetScalarRange(
          std::get<double>(range_begin), std::get<double>(range_end));
      }
    } else {
      rank_mapper->SetScalarRange(0., 0.);
    }
  } else {
    // Handle unexpected type or set a default scalar range
    throw std::runtime_error("Unexpected type in rank qoi range variant.");
  }

  return rank_mapper;
}

void Render::renderPNG(
  PhaseType phase,
  vtkPolyData* rank_mesh,
  vtkPolyData* object_mesh,
  uint64_t edge_width,
  double glyph_factor,
  uint64_t win_size,
  uint64_t font_size,
  std::string output_dir,
  std::string output_file_stem) {
  // Setup rendering space
  vtkSmartPointer<vtkRenderer> renderer = setupRenderer_();

  // Create rank mapper for later use and create corresponding rank actor
  std::variant<std::pair<double, double>, std::set<std::variant<double, int>>>
    rank_qoi_variant(rank_qoi_range_);
  vtkSmartPointer<vtkMapper> rank_mapper =
    createRanksMapper_(rank_mesh, rank_qoi_variant);
  vtkSmartPointer<vtkActor> rank_actor = vtkSmartPointer<vtkActor>::New();
  rank_actor->SetMapper(rank_mapper);

  // Create qoi scale legend for ranks
  std::string rank_qoi_scale_title = "Rank " + this->rank_qoi_;
  vtkSmartPointer<vtkScalarBarActor> rank_qoi_scale_actor =
    createScalarBarActor_(
      rank_mapper, rank_qoi_scale_title, 0.5, 0.9, font_size);
  rank_qoi_scale_actor->DrawBelowRangeSwatchOn();
  rank_qoi_scale_actor->SetBelowRangeAnnotation("<");
  rank_qoi_scale_actor->DrawAboveRangeSwatchOn();
  rank_qoi_scale_actor->SetAboveRangeAnnotation(">");

  // Add rank visualization to renderer
  renderer->AddActor(rank_actor);
  renderer->AddActor2D(rank_qoi_scale_actor);

  if (this->object_qoi_ != "") {
    // Create white to black lookup table
    vtkSmartPointer<vtkLookupTable> bw_lut =
      vtkSmartPointer<vtkLookupTable>::New();
    bw_lut->SetTableRange(0.0, this->object_volume_max_);
    bw_lut->SetSaturationRange(0, 0);
    bw_lut->SetHueRange(0, 0);
    bw_lut->SetValueRange(1, 0);
    bw_lut->SetNanColor(1.0, 1.0, 1.0, 0.0);
    bw_lut->Build();

    // Create mapper for inter-object edges
    vtkSmartPointer<vtkPolyDataMapper> edge_mapper =
      vtkSmartPointer<vtkPolyDataMapper>::New();
    edge_mapper->SetInputData(object_mesh);
    edge_mapper->SetScalarModeToUseCellData();
    edge_mapper->SetScalarRange(0.0, this->object_volume_max_);
    edge_mapper->SetLookupTable(bw_lut);

    // Create communication volume and its scalar bar actors
    vtkSmartPointer<vtkActor> edge_actor = vtkSmartPointer<vtkActor>::New();
    edge_actor->SetMapper(edge_mapper);
    edge_actor->GetProperty()->SetLineWidth(edge_width);
    vtkSmartPointer<vtkScalarBarActor> volume_actor = createScalarBarActor_(
      edge_mapper, "Inter-Object Volume", 0.04, 0.04, font_size);
    // Add communications visualization to renderer
    renderer->AddActor(edge_actor);
    renderer->AddActor2D(volume_actor);

    // Compute square root of object loads
    vtkSmartPointer<vtkArrayCalculator> sqrtL =
      vtkSmartPointer<vtkArrayCalculator>::New();
    sqrtL->SetInputData(object_mesh);
    sqrtL->AddScalarArrayName("load");
    std::string sqrtL_str = "sqrt(load)";
    sqrtL->SetFunction(sqrtL_str.c_str());
    sqrtL->SetResultArrayName(sqrtL_str.c_str());
    sqrtL->Update();
    vtkDataSet* sqrtL_out = sqrtL->GetDataSetOutput();
    sqrtL_out->GetPointData()->SetActiveScalars("migratable");

    // Glyph sentinel and migratable objects separately: 0 is for non-migratable objects, 1 for migratable
    std::map<double, vtkSmartPointer<vtkPolyDataMapper>> glyph_mappers = {
      {0.0, vtkSmartPointer<vtkPolyDataMapper>::New()},
      {1.0, vtkSmartPointer<vtkPolyDataMapper>::New()}};
    std::map<double, std::string> glyph_types = {
      {0.0, "Square"}, {1.0, "Circle"}};
    for (const auto& [k, v] : glyph_types) {
      vtkSmartPointer<vtkThresholdPoints> thresh =
        vtkSmartPointer<vtkThresholdPoints>::New();
      thresh->SetInputData(sqrtL_out);
      thresh->ThresholdBetween(k, k);
      thresh->Update();
      vtkPolyData* thresh_out = thresh->GetOutput();

      if (thresh_out->GetNumberOfPoints() == 0) {
        continue;
      }
      thresh_out->GetPointData()->SetActiveScalars(sqrtL_str.c_str());

      // Glyph by square root of object quantity of interest
      vtkSmartPointer<vtkGlyphSource2D> glyph =
        vtkSmartPointer<vtkGlyphSource2D>::New();
      if (v == "Square") {
        glyph->SetGlyphTypeToSquare();
      } else if (v == "Circle") {
        glyph->SetGlyphTypeToCircle();
      }
      glyph->SetResolution(64);
      glyph->SetScale(1.0);
      glyph->FilledOn();
      glyph->CrossOff();

      vtkSmartPointer<vtkGlyph3D> glypher = vtkSmartPointer<vtkGlyph3D>::New();
      glypher->SetSourceConnection(glyph->GetOutputPort());
      glypher->SetInputData(thresh_out);
      glypher->SetScaleModeToScaleByScalar();
      glypher->SetScaleFactor(glyph_factor);
      glypher->Update();
      glypher->GetOutput()->GetPointData()->SetActiveScalars(
        this->object_qoi_.c_str());

      vtkSmartPointer<vtkTransform> zRaise =
        vtkSmartPointer<vtkTransform>::New();
      zRaise->Translate(0.0, 0.0, 0.01);

      vtkSmartPointer<vtkTransformPolyDataFilter> trans =
        vtkSmartPointer<vtkTransformPolyDataFilter>::New();
      trans->SetTransform(zRaise);
      trans->SetInputData(glypher->GetOutput());

      glyph_mappers.at(k)->SetInputConnection(trans->GetOutputPort());
      glyph_mappers.at(k)->SetLookupTable(
        createColorTransferFunction_(this->object_qoi_range_));

      if (std::holds_alternative<std::pair<double, double>>(
            this->object_qoi_range_)) {
        auto range =
          std::get<std::pair<double, double>>(this->object_qoi_range_);
        // Manually set scalar range so either mapper (migratable vs non-migratable) can be used for the scalar bar range
        glyph_mappers.at(k)->SetScalarRange(range.first, range.second);
      }

      vtkSmartPointer<vtkActor> object_glyph_actor =
        vtkSmartPointer<vtkActor>::New();
      object_glyph_actor->SetMapper(glyph_mappers.at(k));

      // Add objects visualization to renderer
      renderer->AddActor(object_glyph_actor);
    }

    if (glyph_mappers.at(1.0)) {
      std::string object_qoi_name = "Object " + this->object_qoi_;
      std::set<std::variant<double, int>> values = {};
      // Check continuity of object qoi
      if (std::holds_alternative<std::pair<double, double>>(
            this->object_qoi_range_)) {
        values = {};
      } else if (std::holds_alternative<std::set<std::variant<double, int>>>(
                   this->object_qoi_range_)) {
        values = std::get<std::set<std::variant<double, int>>>(
          this->object_qoi_range_);
      } else {
        throw std::runtime_error(
          "Unexpected type in object_qoi_range variant.");
      }
      vtkSmartPointer<vtkActor2D> object_qoi_scalar_bar_actor =
        createScalarBarActor_(
          glyph_mappers.at(1.0),
          object_qoi_name.c_str(),
          0.52,
          0.04,
          font_size,
          values);
      renderer->AddActor2D(object_qoi_scalar_bar_actor);
    }
  }

  // Add field data text information to render
  // Create text
  std::stringstream ss;
  if (selected_phase_ != std::numeric_limits<PhaseType>::max()) {
    ss << "Phase: " << phase << "\n";
  } else {
    ss << "Phase: " << phase << "/" << (this->n_phases_ - 1) << "\n";
  }
  ss << "Load Imbalance: " << std::fixed << std::setprecision(2)
     << this->info_.getImbalance(phase);
  // Setup text actor
  vtkSmartPointer<vtkTextActor> text_actor =
    vtkSmartPointer<vtkTextActor>::New();
  text_actor->SetInput(ss.str().c_str());
  vtkTextProperty* textProp = text_actor->GetTextProperty();
  textProp->SetColor(0.0, 0.0, 0.0);
  textProp->ItalicOff();
  textProp->BoldOff();
  textProp->SetFontFamilyToArial();
  textProp->SetFontSize(font_size);
  textProp->SetLineSpacing(1.5);
  // Place text
  vtkCoordinate* position = text_actor->GetPositionCoordinate();
  position->SetCoordinateSystemToNormalizedViewport();
  position->SetValue(0.04, 0.91, 0.0);
  // Add text to render
  renderer->AddActor(text_actor);

  // Setup rendering window
  renderer->ResetCamera();
  vtkNew<vtkRenderWindow> render_window;
  render_window->AddRenderer(renderer);
  render_window->SetWindowName("vt-tv");
  render_window->SetSize(win_size, win_size);
  render_window->Render();

  // Setup image from window
  vtkNew<vtkWindowToImageFilter> w2i;
  w2i->SetInput(render_window);
  w2i->SetScale(1);

  // Export the PNG image
  vtkNew<vtkPNGWriter> writer;
  writer->SetInputConnection(w2i->GetOutputPort());
  std::string png_filename =
    output_dir + output_file_stem + std::to_string(phase) + ".png";
  writer->SetFileName(png_filename.c_str());
  writer->SetCompressionLevel(2);
  writer->Write();
}

void Render::generate(uint64_t font_size, uint64_t win_size) {
  double rank_qoi_min = rank_qoi_range_.first;
  double rank_qoi_max = rank_qoi_range_.second;

  if (std::holds_alternative<std::pair<double, double>>(object_qoi_range_)) {
    auto range_pair = std::get<std::pair<double, double>>(object_qoi_range_);
    double object_qoi_min = range_pair.first;
    double object_qoi_max = range_pair.second;
    fmt::print(
      "Rank {} range: {}, {}\n", rank_qoi_, rank_qoi_min, rank_qoi_max);
    fmt::print(
      "Object {} range: {}, {}\n", object_qoi_, object_qoi_min, object_qoi_max);
  }

  fmt::print("selected phase={}\n", selected_phase_);

  auto createMeshAndRender = [&](PhaseType phase) {
    vtkNew<vtkPolyData> object_mesh = this->createObjectMesh_(phase);
    vtkNew<vtkPolyData> rank_mesh = this->createRankMesh_(phase);

    if (save_meshes_) {
      fmt::print("== Writing object mesh for phase {}\n", phase);
      vtkNew<vtkXMLPolyDataWriter> writer;
      std::string object_mesh_filename = output_dir_ + output_file_stem_ +
        "_object_mesh_" + std::to_string(phase) + ".vtp";
      writer->SetFileName(object_mesh_filename.c_str());
      writer->SetInputData(object_mesh);
      writer->Write();

      fmt::print("== Writing rank mesh for phase {}\n", phase);
      vtkNew<vtkXMLPolyDataWriter> writer2;
      std::string rank_mesh_filneame = output_dir_ + output_file_stem_ +
        "_rank_mesh_" + std::to_string(phase) + ".vtp";
      writer2->SetFileName(rank_mesh_filneame.c_str());
      writer2->SetInputData(rank_mesh);
      writer2->Write();
    }

    if (save_pngs_) {
      fmt::print("== Rendering visualization PNG for phase {}\n", phase);

      std::pair<double, double> obj_qoi_range;
      try {
        obj_qoi_range =
          std::get<std::pair<double, double>>(this->object_qoi_range_);
      } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        obj_qoi_range = {0, 1};
      }

      uint64_t window_size = win_size;
      uint64_t edge_width = 0.03 * window_size /
        *std::max_element(this->grid_size_.begin(), this->grid_size_.end());
      double glyph_factor = 0.8 * this->grid_resolution_ /
        ((this->max_o_per_dim_ + 1) * std::sqrt(object_load_max_));
      fmt::print("  Image size: {}x{}px\n", win_size, win_size);
      fmt::print("  Font size: {}pt\n", font_size);
      this->renderPNG(
        phase,
        rank_mesh,
        object_mesh,
        edge_width,
        glyph_factor,
        window_size,
        font_size,
        output_dir_,
        output_file_stem_);
    }
  };

  if (selected_phase_ != std::numeric_limits<PhaseType>::max()) {
    createMeshAndRender(selected_phase_);
  } else {
    for (PhaseType phase = 0; phase < this->n_phases_; phase++) {
      createMeshAndRender(phase);
    }
  }
}

} // namespace vt::tv
