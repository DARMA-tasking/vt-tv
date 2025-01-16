/*
//@HEADER
// *****************************************************************************
//
//                                    info.h
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

#if !defined INCLUDED_VT_TV_API_INFO_H
#define INCLUDED_VT_TV_API_INFO_H

#include "vt-tv/api/types.h"
#include "vt-tv/api/rank.h"
#include "vt-tv/api/object_info.h"

#include <fmt-vt/format.h>

#include <unordered_map>
#include <cassert>
#include <set>

namespace vt::tv {

/**
 * \struct Info
 *
 * \brief All the information for a set of ranks and phases.
 *
 * @todo: Elaborate this...
 *
 */
struct Info {
  Info(
    std::unordered_map<ElementIDType, ObjectInfo> in_object_info,
    std::unordered_map<NodeType, Rank> in_ranks)
    : object_info_(std::move(in_object_info)),
      ranks_(std::move(in_ranks)) { }

  Info() = default;

  /**
   * \brief Add more information about a new rank
   *
   * \param[in] object_info object information to merge with existing data
   * \param[in] r the rank work
   */
  void
  addInfo(std::unordered_map<ElementIDType, ObjectInfo> object_info, Rank r) {
    for (auto x : object_info) {
      object_info_.try_emplace(x.first, std::move(x.second));
    }

    assert(ranks_.find(r.getRankID()) == ranks_.end() && "Rank must not exist");
    ranks_.try_emplace(r.getRankID(), std::move(r));
  }

  void setSelectedPhase(PhaseType selected_phase) {
    selected_phase_ = selected_phase;
  }

  /**
   * \brief Get all object info
   *
   * \return map of object info
   */
  auto const& getObjectInfo() const { return object_info_; }

  /**
   * \brief Get all ranks
   *
   * \return map of Ranks
   */
  auto const& getRanks() const { return ranks_; }

  /**
   * \brief Get all rank ids
   *
   * \return Vector of rank ids
   */
  std::vector<NodeType> getRankIDs() const {
    std::vector<NodeType> ids;
    for (auto [rankid, _] : ranks_) {
      ids.push_back(rankid);
    }
    return ids;
  }

  /**
   * \brief Get rank
   *
   * \return Rank
   */
  auto const& getRank(NodeType rank_id) const { return ranks_.at(rank_id); }

  /**
   * \brief Get number of phases, which should be the same across ranks
   *
   * \return The number of phases
   */
  uint64_t getNumPhases() const {
    uint64_t n_phases = 0;
    if (this->ranks_.size() > 0) {
      n_phases = this->ranks_.at(0).getNumPhases();
      for (NodeType rank_id = 1;
           rank_id < static_cast<NodeType>(this->ranks_.size());
           rank_id++) {
        if (ranks_.at(rank_id).getNumPhases() != n_phases) {
          throw std::runtime_error(
            "Number of phases must be consistent across ranks");
        }
      }
    }
    return n_phases;
  }

  /////////////////////////////////////////////////////////////////////////////////////////
  ////////////                                                                 ////////////
  ////////////                     GENERALIZED QOI GETTERS                     ////////////
  ////////////                                                                 ////////////
  /////////////////////////////////////////////////////////////////////////////////////////

  /*  -----------------------------------  Getters  -----------------------------------  */

  /**
   * \brief Converts a QOI from QOIVariantTypes to \c T
   */
  template <typename T>
  T convertQOIVariantTypeToT_(QOIVariantTypes const& variant) const {
    if (std::holds_alternative<int>(variant)) {
      return static_cast<T>(std::get<int>(variant));
    } else if (std::holds_alternative<double>(variant)) {
      return static_cast<T>(std::get<double>(variant));
    } else if (std::holds_alternative<std::string>(variant)) {
      throw std::runtime_error(
        "QOI type must be numerical (received std::string).");
    } else {
      throw std::runtime_error(
        "Invalid QOI type received (must be numerical).");
    }
  }

  enum struct VtkTypeEnum : int {
    TYPE_DOUBLE,
    TYPE_INT
  };

  std::unordered_map<std::string, VtkTypeEnum> computable_qoi_types = {
    {"load", VtkTypeEnum::TYPE_DOUBLE},
    {"received_volume", VtkTypeEnum::TYPE_DOUBLE},
    {"sent_volume", VtkTypeEnum::TYPE_DOUBLE},
    {"max_volume", VtkTypeEnum::TYPE_DOUBLE},
    {"number_of_objects", VtkTypeEnum::TYPE_INT},
    {"number_of_migratable_objects", VtkTypeEnum::TYPE_INT},
    {"migratable_load", VtkTypeEnum::TYPE_DOUBLE},
    {"sentinel_load", VtkTypeEnum::TYPE_DOUBLE},
    {"id", VtkTypeEnum::TYPE_INT},
    {"rank_id", VtkTypeEnum::TYPE_INT}
  };

  /**
   * \brief Returns a getter to a specified rank QOI
   */
  template <typename T>
  std::function<T(Rank, PhaseType)>
  getRankQOIGetter(std::string const& rank_qoi) const {
    std::function<T(Rank, PhaseType)> qoi_getter;
    if (rank_qoi == "load") {
      qoi_getter = [&](Rank rank, PhaseType phase) {
        return convertQOIVariantTypeToT_<T>(getRankLoad(rank, phase));
      };
    } else if (rank_qoi == "received_volume") {
      qoi_getter = [&](Rank rank, PhaseType phase) {
        return convertQOIVariantTypeToT_<T>(
          getRankReceivedVolume(rank, phase));
      };
    } else if (rank_qoi == "sent_volume") {
      qoi_getter = [&](Rank rank, PhaseType phase) {
        return convertQOIVariantTypeToT_<T>(getRankSentVolume(rank, phase));
      };
    } else if (rank_qoi == "number_of_objects") {
      qoi_getter = [&](Rank rank, PhaseType phase) {
        return convertQOIVariantTypeToT_<T>(getRankNumObjects(rank, phase));
      };
    } else if (rank_qoi == "number_of_migratable_objects") {
      qoi_getter = [&](Rank rank, PhaseType phase) {
        return convertQOIVariantTypeToT_<T>(
          getRankNumMigratableObjects(rank, phase));
      };
    } else if (rank_qoi == "migratable_load") {
      qoi_getter = [&](Rank rank, PhaseType phase) {
        return convertQOIVariantTypeToT_<T>(getRankMigratableLoad(rank, phase));
      };
    } else if (rank_qoi == "sentinel_load") {
      qoi_getter = [&](Rank rank, PhaseType phase) {
        return convertQOIVariantTypeToT_<T>(getRankSentinelLoad(rank, phase));
      };
    } else if (rank_qoi == "id") {
      qoi_getter = [&](Rank rank, PhaseType) {
        return convertQOIVariantTypeToT_<T>(getRankID(rank));
      };
    } else {
      // Look in attributes (will throw an error if QOI doesn't exist)
      qoi_getter = [&](Rank rank, PhaseType) {
        return convertQOIVariantTypeToT_<T>(getRankAttribute(rank, rank_qoi));
      };
    }
    return qoi_getter;
  }

  /**
   * \brief Returns a getter to a specified object QOI
   */
  template <typename T>
  std::function<T(ObjectWork)>
  getObjectQOIGetter(std::string const& object_qoi) const {
    std::function<T(ObjectWork)> qoi_getter;
    if (object_qoi == "load") {
      qoi_getter = [&](ObjectWork obj) {
        return convertQOIVariantTypeToT_<T>(getObjectLoad(obj));
      };
    } else if (object_qoi == "received_volume") {
      qoi_getter = [&](ObjectWork obj) {
        return convertQOIVariantTypeToT_<T>(getObjectReceivedVolume(obj));
      };
    } else if (object_qoi == "sent_volume") {
      qoi_getter = [&](ObjectWork obj) {
        return convertQOIVariantTypeToT_<T>(getObjectSentVolume(obj));
      };
    } else if (object_qoi == "max_volume") {
      qoi_getter = [&](ObjectWork obj) {
        return convertQOIVariantTypeToT_<T>(getObjectMaxVolume(obj));
      };
    } else if (object_qoi == "id") {
      qoi_getter = [&](ObjectWork obj) {
        return convertQOIVariantTypeToT_<T>(getObjectID(obj));
      };
    } else if (object_qoi == "rank_id") {
      qoi_getter = [&](ObjectWork obj) {
        return convertQOIVariantTypeToT_<T>(getObjectRankID(obj));
      };
    } else {
      // Look in attributes and user_defined (will throw an error if QOI doesn't exist)
      qoi_getter = [&](ObjectWork obj) {
        return convertQOIVariantTypeToT_<T>(
          getObjectAttributeOrUserDefined(obj, object_qoi));
      };
    }
    return qoi_getter;
  }

  /*  ---------------------------------  Rank Getters  ---------------------------------  */

  /**
   * \brief Get QOI of a given rank at a given phase
   *
   * \return a map of QOI per rank
   */
  template <typename T = double>
  T getRankQOIAtPhase(
    ElementIDType rank_id, PhaseType phase, std::string const& rank_qoi
  ) const {
    auto qoi_getter = getRankQOIGetter<T>(rank_qoi);
    auto const& rank = this->ranks_.at(rank_id);
    return qoi_getter(rank, phase);
  }

  /**
   * \brief Get QOI of a given rank across all phases
   *
   * \return a map of QOI per rank
   */
  template <typename T = double>
  std::unordered_map<PhaseType, T>
  getAllQOIAtRank(ElementIDType rank_id, std::string const& rank_qoi) const {
    auto const& rank = ranks_.at(rank_id);
    auto const& phase_work = rank.getPhaseWork();

    std::unordered_map<PhaseType, T> rank_qois;

    if (hasRankUserDefined(rank_qoi)) {
      auto const& test_value = getFirstRankUserDefined(rank_qoi);
      for (auto const& [phase, _] : phase_work) {
        if (std::holds_alternative<double>(test_value)) {
          rank_qois.emplace(
            phase, static_cast<T>(
              std::get<double>(getRankUserDefined(rank, phase, rank_qoi))
            )
          );
        } else if (std::holds_alternative<int>(test_value)) {
          rank_qois.emplace(
            phase, static_cast<T>(
              std::get<int>(getRankUserDefined(rank, phase, rank_qoi))
            )
          );
        }
      }
    } else {
      auto qoi_getter = getRankQOIGetter<T>(rank_qoi);
      for (auto const& [phase, _] : phase_work) {
        rank_qois.emplace(phase, qoi_getter(rank, phase));
      }
    }

    return rank_qois;
  }

  /**
   * \brief Get QOI of all ranks at given phase
   *
   * \return a map of QOI per rank
   */
  template <typename T = double>
  std::unordered_map<ElementIDType, T>
  getAllRankQOIAtPhase(PhaseType phase, std::string const& rank_qoi) const {
    std::unordered_map<ElementIDType, T> rank_qois;

    if (hasRankUserDefined(rank_qoi)) {
      auto const& test_value = getFirstRankUserDefined(rank_qoi);
      for (uint64_t rank_id = 0; rank_id < ranks_.size(); rank_id++) {
        auto const& rank = ranks_.at(rank_id);
        if (std::holds_alternative<double>(test_value)) {
          rank_qois.emplace(
            phase, static_cast<T>(
              std::get<double>(getRankUserDefined(rank, phase, rank_qoi))
            )
          );
        } else if (std::holds_alternative<int>(test_value)) {
          rank_qois.emplace(
            phase, static_cast<T>(
              std::get<int>(getRankUserDefined(rank, phase, rank_qoi))
            )
          );
        }
      }
    } else {
      auto qoi_getter = getRankQOIGetter<T>(rank_qoi);
      for (auto const& [rank_id, rank] : this->ranks_) {
        rank_qois.emplace(rank_id, qoi_getter(rank, phase));
      }
    }

    return rank_qois;
  }

  /*  --------------------------------  Object Getters  --------------------------------  */

  /**
   * \brief Get a specified object's QOI at a given phase
   *
   * \return the object QOI
   */
  template <typename T>
  T getObjectQOIAtPhase(
    ElementIDType obj_id, PhaseType phase, std::string const& obj_qoi
  ) const {
    auto const& objects = this->getPhaseObjects(phase);
    auto const& obj = objects.at(obj_id);
    auto const& ud = obj.getUserDefined();

    if (auto it = ud.find(obj_qoi); it != ud.end()) {
      if (std::holds_alternative<double>(it->second)) {
        return static_cast<T>(std::get<double>(it->second));
      } else if (std::holds_alternative<int>(it->second)) {
        return static_cast<T>(std::get<int>(it->second));
      }
    }

    auto qoi_getter = getObjectQOIGetter<T>(obj_qoi);
    return qoi_getter(obj);
  }

  /**
   * \brief Get all objects for a given rank and phase
   *
   * \param[in] rank_id the rank
   * \param[in] phase the phase
   *
   * \return the objects
   */
  std::unordered_map<ElementIDType, ObjectWork>
  getRankObjects(ElementIDType rank_id, PhaseType phase) const {
    std::unordered_map<ElementIDType, ObjectWork> objects;

    // Get Rank info for specified rank
    auto const& rank_info = ranks_.at(rank_id);

    // Get history of phases for this rank
    auto const& phase_history_at_rank = rank_info.getPhaseWork();

    // Get phase work at specified phase
    auto const& phase_work_at_rank = phase_history_at_rank.find(phase);

    // Get all objects at specified phase
    auto const& object_work_at_phase_at_rank =
      phase_work_at_rank->second.getObjectWork();

    for (auto const& [elm_id, obj_work] : object_work_at_phase_at_rank) {
      objects.insert(std::make_pair(elm_id, obj_work));
    }

    return objects;
  }

  /**
   * \brief Get all objects in all ranks for a given phase
   *
   * \param[in] phase the phase
   *
   * \return the objects
   */
  std::unordered_map<ElementIDType, ObjectWork>
  getPhaseObjects(PhaseType phase) const {
    // fmt::print("Phase: {}\n", phase);

    // Map of objects at given phase
    std::unordered_map<ElementIDType, ObjectWork> objects_at_phase;

    // Go through all ranks and get all objects at given phase
    for (uint64_t rank = 0; rank < this->ranks_.size(); rank++) {
      // fmt::print("  Rank: {}\n",rank);
      // Get Rank info for specified rank
      auto const& rank_info = ranks_.at(rank);

      // Get history of phases for this rank
      auto const& phase_history = rank_info.getPhaseWork();

      // Get phase work at specified phase
      auto const& phase_work = phase_history.find(phase);
      if (phase_work == phase_history.end()) {
        auto ex = "info::getPhaseObjects: Phase " + std::to_string(phase) +
          " doesn't exist for rank " + std::to_string(rank);
        throw std::runtime_error(ex);
      }

      // Get all objects at specified phase
      auto const& object_work_at_phase = phase_work->second.getObjectWork();

      for (auto const& [elm_id, obj_work] : object_work_at_phase) {
        // fmt::print("    Object Id: {}\n", elm_id);
        objects_at_phase.insert(std::make_pair(elm_id, obj_work));
      }
    }
    return objects_at_phase;
  }

  /**
   * \brief Get maximum inter-object communication volume across all ranks and phases
   *
   * \return the maximum volume
   */
  double getMaxVolume() const {
    double ov_max = 0.;

    /* Iterate over all phases: each object is re-initialized when
    advancing to the next phase (in the JSON-reader), thus different memory spaces
    are used for an object of the same id but of a different phase.
    This means the object communications are not phase persistent, so one can't obtain
    the maximum volume by iterated through object ids.
    */
    auto n_phases = this->getNumPhases();

    if (selected_phase_ != std::numeric_limits<PhaseType>::max()) {
      auto const& objects = this->getPhaseObjects(selected_phase_);
      for (auto const& [obj_id, obj_work] : objects) {
        auto obj_max_v = obj_work.getMaxVolume();
        if (obj_max_v > ov_max)
          ov_max = obj_max_v;
      }
    } else {
      for (PhaseType phase = 0; phase < n_phases; phase++) {
        {
          auto const& objects = this->getPhaseObjects(phase);
          for (auto const& [obj_id, obj_work] : objects) {
            auto obj_max_v = obj_work.getMaxVolume();
            if (obj_max_v > ov_max)
              ov_max = obj_max_v;
          }
        }
      }
    }
    return ov_max;
  }

  /**
   * \brief Get maximum load of objects across all ranks and phases
   *
   * \return the maximum load
   */
  double getMaxLoad() const {
    double ol_max = 0.;

    auto n_phases = this->getNumPhases();

    if (selected_phase_ != std::numeric_limits<PhaseType>::max()) {
      auto const& objects = this->getPhaseObjects(selected_phase_);
      for (auto const& [obj_id, obj_work] : objects) {
        auto obj_load = obj_work.getLoad();
        if (obj_load > ol_max)
          ol_max = obj_load;
      }
    } else {
      for (PhaseType phase = 0; phase < n_phases; phase++) {
        auto const& objects = this->getPhaseObjects(phase);
        for (auto const& [obj_id, obj_work] : objects) {
          auto obj_load = obj_work.getLoad();
          if (obj_load > ol_max)
            ol_max = obj_load;
        }
      }
    }
    return ol_max;
  }

  /**
   * \brief Create mapping of all objects in all ranks for a given phase (made for allowing changes to these objects)
   *
   * \param[in] phase the phase
   *
   * \return the objects
   */
  std::unordered_map<ElementIDType, ObjectWork>
  createPhaseObjectsMapping(PhaseType phase) {
    // fmt::print("Phase: {}\n", phase);

    // Map of objects at given phase
    std::unordered_map<ElementIDType, ObjectWork> objects_at_phase;

    // Go through all ranks and get all objects at given phase
    for (uint64_t rank = 0; rank < this->ranks_.size(); rank++) {
      // fmt::print("  Rank: {}\n",rank);
      // Get Rank info for specified rank
      auto& rank_info = ranks_.at(rank);

      // Get history of phases for this rank
      auto& phase_history = rank_info.getPhaseWork();

      // Get phase work at specified phase
      auto phase_work = phase_history.find(phase);

      // Get all objects at specified phase
      auto& object_work_at_phase = phase_work->second.getObjectWork();

      for (auto const& [elm_id, obj_work] : object_work_at_phase) {
        // fmt::print("    Object Id: {}\n", elm_id);
        objects_at_phase.insert(std::make_pair(elm_id, obj_work));
      }
    }
    return objects_at_phase;
  }

  /**
   * \brief Get all object IDs across all ranks and phases
   *
   * \return the objects
   */
  std::set<ElementIDType> getAllObjectIDs() const {
    // Map of objects at given phase
    std::set<ElementIDType> objects;

    // Go through all ranks and get all objects at given phase
    for (uint64_t rank = 0; rank < this->ranks_.size(); rank++) {
      // fmt::print("Rank: {}\n",rank);
      // Get Rank info for specified rank
      auto const& rank_info = this->ranks_.at(rank);

      // Get history of phases for this rank
      auto const& phase_history = rank_info.getPhaseWork();

      // Go through history of all phases
      for (auto const& [phase, phase_work] : phase_history) {
        // fmt::print("|->  Phase: {}\n", phase);
        // Get all objects at this phase
        auto const& object_work_at_phase = phase_work.getObjectWork();

        for (auto const& [elm_id, obj_work] : object_work_at_phase) {
          // fmt::print("|    |-> Object Id: {}\n", elm_id);
          objects.insert(elm_id);
        }
      }
    }
    fmt::print("Size of all objects: {}\n", objects.size());
    return objects;
  }

  /**
   * \brief Get number of ranks stored here
   *
   * \return number of ranks
   */
  std::size_t getNumRanks() const { return ranks_.size(); }

  /**
   * \brief Normalize communications for a phase: ensure receives and sends coincide
   *
   * \return void
   */
  void normalizeEdges(PhaseType phase) {
    fmt::print("\n---- Normalizing Edges for phase {} ----\n", phase);
    // Vector of tuples of communications to add: {side_to_be_modified, id1, id2, bytes} for an id1 -> id2 communication (1 sends to 2, 2 receives from 1)
    // if type is "sender", communication has to be added to sent communications for object id1
    // if type is "recipient", communication has to be added to received communications for object id2
    std::vector<std::tuple<std::string, ElementIDType, ElementIDType, double>>
      communications_to_add;

    auto phase_objects = createPhaseObjectsMapping(phase);
    // Checking all communications for object A in all objects of all ranks at given phase: A <- ... and A -> ...
    for (auto& [A_id, object_work] : phase_objects) {
      // fmt::print("- Object ID: {}\n", A_id);
      auto sent = object_work.getSent();
      // fmt::print(" Has {} sent communications", sent.size());
      auto received = object_work.getReceived();
      //      fmt::print(" and {} received communications.\n", received.size());
      // Going through A -> ... communications
      //      fmt::print(" Checking sent communications:\n");
      for (auto& [B_id, bytes] : sent) {
        //        fmt::print("  Communication sent to object {} of {} bytes:\n", B_id, bytes);
        // check if B exists for the A -> B communication
        if (phase_objects.find(B_id) != phase_objects.end()) {
          //          fmt::print("  Found recipient object {} when searching for communication sent by object {} of {} bytes.\n", B_id, A_id, bytes);
          auto to_object_work = phase_objects.at(B_id);
          auto target_received = to_object_work.getReceived();
          //          fmt::print(  "Object {} has {} received communications.\n", B_id, target_received.size());
          // Check if B has symmetric B <- A received communication
          if (target_received.find(A_id) != target_received.end()) {
            //            fmt::print(  "   Object {} already has received communication from object {}.\n", B_id, A_id);
          } else {
            //            fmt::print(  "   Object {} doesn't have received communication from object {}. Pushing to list of communications to add.\n", B_id, A_id);
            communications_to_add.push_back(
              std::make_tuple("recipient", A_id, B_id, bytes));
          }
        } else {
          fmt::print(
            "  /!\\ Didn't find recipient object {} when searching for "
            "communication sent by object {} of {} bytes.\n",
            B_id,
            A_id,
            bytes);
        }
      }
      // Going through A <- ... communications
      //      fmt::print(" Checking received communications:\n");
      for (auto& [B_id, bytes] :
           received) { // Going through A <- ... communications
        //        fmt::print("  Communication received from object {} of {} bytes:\n", B_id, bytes);
        // check if B exists for the A <- B communication
        if (phase_objects.find(B_id) != phase_objects.end()) {
          //          fmt::print("  Found sender object {} when searching for communication received by object {} of {} bytes.\n", B_id, A_id, bytes);
          auto from_object_work = phase_objects.at(B_id);
          auto target_sent = from_object_work.getSent();
          //          fmt::print(  "Object {} has {} sent communications.\n", B_id, target_sent.size());
          // Check if B has symmetric B -> A received communication
          if (target_sent.find(A_id) != target_sent.end()) {
            //            fmt::print(  "   Object {} already has sent communication to object {}.\n", B_id, A_id);
          } else {
            //            fmt::print(  "   Object {} doesn't have sent communication to object {}. Pushing to list of communications to add.\n", B_id, A_id);
            communications_to_add.push_back(
              std::make_tuple("sender", B_id, A_id, bytes));
          }
        } else {
          //          fmt::print("  /!\\ Didn't find sender object {} when searching for communication received by object {} of {} bytes.\n", B_id, A_id, bytes);
        }
      }
    }

    // loop through ranks and add communications
    // fmt::print("Updating communications for phase {}.\n", phase);
    for (auto& [rank_id, rank] : ranks_) {
      // fmt::print(" Checking objects in rank {}.\n", rank_id);
      auto& phaseWork = rank.getPhaseWork();
      auto& phaseWorkAtPhase = phaseWork.at(phase);
      auto& objects = phaseWorkAtPhase.getObjectWork();
      for (auto& [obj_id, obj_work] : objects) {
        // fmt::print("  Checking if object {} needs to be updated.\n", obj_id);
        // fmt::print("  Communications to update:\n");
        uint64_t i = 0;
        for (auto& [object_to_update, sender_id, recipient_id, bytes] :
             communications_to_add) {
          // fmt::print("    {} needs to be updated in {} -> {} communication of {} bytes.\n", object_to_update,
          //  sender_id, recipient_id, bytes);
          if (object_to_update == "sender" && sender_id == obj_id) {
            // fmt::print("    Sender to be updated is object on this rank. Updating.\n");
            rank.addObjectSentCommunicationAtPhase(
              phase, obj_id, recipient_id, bytes);
            communications_to_add.erase(communications_to_add.begin() + i);
          } else if (
            object_to_update == "recipient" && recipient_id == obj_id) {
            // fmt::print("    Recipient to be updated is object on this rank. Updating.\n");
            rank.addObjectReceivedCommunicationAtPhase(
              phase, obj_id, sender_id, bytes);
            communications_to_add.erase(communications_to_add.begin() + i);
          }
          if (communications_to_add.empty()) {
            return;
          }
          i++;
        }
      }
    }
  }

  /**
   * \brief Compute imbalance across ranks at phase
   *
   * \param[in] phase the phase
   *
   * \return the imbalance
   */
  double getImbalance(PhaseType phase) const {
    double load_sum = 0.;
    double max_load = 0.;

    for (uint64_t rank = 0; rank < this->ranks_.size(); rank++) {
      auto rank_max_load = this->getRank(rank).getLoad(phase);
      if (rank_max_load > max_load)
        max_load = rank_max_load;
      load_sum += this->getRank(rank).getLoad(phase);
    }
    double load_avg = load_sum / this->ranks_.size();
    double imbalance = std::numeric_limits<double>::quiet_NaN();
    if (load_avg != 0) {
      imbalance = (max_load / load_avg) - 1.;
    }
    return imbalance;
  }

  /* ------------------- Object QOI getters ------------------- */

  /**
   * \brief Get the id of an object at a given phase
   *
   * \param[in] object the current object
   *
   * \return the id
   */
  QOIVariantTypes getObjectID(ObjectWork object) const {
    return static_cast<int>(object.getID());
  }

  /**
   * \brief Get the rank id of an object at a given phase
   *
   * \param[in] object the current object
   *
   * \return the rank id
   */
  QOIVariantTypes getObjectRankID(ObjectWork object) const {
    auto obj_id = object.getID();
    auto obj_info = object_info_.at(obj_id);
    return obj_info.getHome();
  }

  /**
   * \brief Get the load of an object at a given phase
   *
   * \param[in] object the current object
   *
   * \return the load
   */
  QOIVariantTypes getObjectLoad(ObjectWork object) const {
    return object.getLoad();
  }

  /**
   * \brief Get the received volume of an object at a given phase
   *
   * \param[in] object the current object
   *
   * \return the received volume
   */
  QOIVariantTypes getObjectReceivedVolume(ObjectWork object) const {
    return object.getReceivedVolume();
  }

  /**
    * \brief Get the sent volume of an object at a given phase
    *
    * \param[in] object the current object
    *
    * \return the sent volume
    */
  QOIVariantTypes getObjectSentVolume(ObjectWork object) const {
    return object.getSentVolume();
  }

  /**
    * \brief Get the max volume of an object at a given phase
    *
    * \param[in] object the current object
    *
    * \return the max volume
    */
  QOIVariantTypes getObjectMaxVolume(ObjectWork object) const {
    return object.getMaxVolume();
  }

  /**
    * \brief Get the specified attribute or user_defined QOI of an object at a given phase
    *
    * \param[in] object the current object
    *
    * \return the requested attribute or user_defined QOI
    */
  QOIVariantTypes getObjectAttributeOrUserDefined(
    ObjectWork object, std::string object_qoi) const {
    auto obj_attributes = object.getAttributes();
    if (obj_attributes.count(object_qoi) > 0) {
      return obj_attributes.at(object_qoi);
    } else {
      auto obj_user_defined = object.getUserDefined();
      if (obj_user_defined.count(object_qoi) > 0) {
        return obj_user_defined.at(object_qoi);
      }
    }
    throw std::runtime_error("Invalid Object QOI: " + object_qoi);
  }

  /* ---------------------------------------------------------- */

  /* -------------------- Rank QOI getters -------------------- */

  /**
   * \brief Get id of a given rank
   *
   * \param[in] rank the rank
   * \param[in] phase the phase (unused for this QOI)
   *
   * \return the rank id
   */
  QOIVariantTypes getRankID(Rank rank) const {
    return static_cast<int>(rank.getRankID());
  }

  /**
   * \brief Get user-defined QOIs on a rank
   *
   * \param[in] rank the rank
   * \param[in] phase the phase
   * \param[in] key the key
   *
   * \return the value for a given user-defined key/value pair
   */
  QOIVariantTypes getRankUserDefined(
    Rank const& rank, PhaseType phase, std::string const& key
  ) const {
    return rank.getPhaseWork().at(phase).getUserDefined().at(key);
  }

  /**
   * \brief Check if a QOI exists in user-defined
   *
   * \param[in] key the key
   *
   * \return whether it exists
   */
  bool hasRankUserDefined(std::string const& key) const {
    for (auto const& [id, rank] : ranks_) {
      auto const num_phases = rank.getNumPhases();
      for (std::size_t i = 0; i < num_phases; i++) {
        auto const& ud = rank.getPhaseWork().at(i).getUserDefined();
        if (auto iter = ud.find(key); iter != ud.end()) {
          return true;
        }
      }
    }
    return false;
  }

  /**
   * \brief Get the first value for a QOI that matches
   *
   * \param[in] key the key
   *
   * \return the value
   */
  QOIVariantTypes getFirstRankUserDefined(std::string const& key) const {
    for (auto const& [id, rank] : ranks_) {
      auto const num_phases = rank.getNumPhases();
      for (std::size_t i = 0; i < num_phases; i++) {
        auto const& ud = rank.getPhaseWork().at(i).getUserDefined();
        if (auto iter = ud.find(key); iter != ud.end()) {
          return iter->second;
        }
      }
    }
    return QOIVariantTypes{};
  }

  /**
   * \brief Get all the user-defined keys for a given rank on a phase
   *
   * \param[in] rank the rank
   * \param[in] phase the phase
   *
   * \return vector of keys
   */
  std::vector<std::string> getRankUserDefinedKeys(
    Rank rank, PhaseType phase
  ) const {
    std::vector<std::string> keys;
    auto const& user_defined = rank.getPhaseWork().at(phase).getUserDefined();
    for (auto const& [key, _] : user_defined) {
      keys.push_back(key);
    }
    return keys;
  }

  /**
   * \brief Get load of a given rank
   *
   * \param[in] rank the rank
   * \param[in] phase the phase
   *
   * \return the rank load
   */
  QOIVariantTypes getRankLoad(Rank rank, PhaseType phase) const {
    return rank.getLoad(phase);
  }

  /**
   * \brief Get the received volume of a rank at a given phase
   *
   * \param[in] rank the rank
   * \param[in] phase the phase
   *
   * \return the received volume
   */
  QOIVariantTypes getRankReceivedVolume(Rank rank, PhaseType phase) const {
    auto received_volume = 0.;
    auto const& phase_objects = rank.getPhaseWork().at(phase).getObjectWork();
    for (auto const& [obj_id, obj_work] : phase_objects) {
      received_volume += obj_work.getReceivedVolume();
    }
    return received_volume;
  }

  /**
   * \brief Get the sent volume of a rank at a given phase
   *
   * \param[in] rank the rank
   * \param[in] phase the phase
   *
   * \return the sent volume
   */
  QOIVariantTypes getRankSentVolume(Rank rank, PhaseType phase) const {
    auto sent_volume = 0.;
    auto const& phase_objects = rank.getPhaseWork().at(phase).getObjectWork();
    for (auto const& [obj_id, obj_work] : phase_objects) {
      sent_volume += obj_work.getSentVolume();
    }
    return sent_volume;
  }

  /**
   * \brief Get the number of objects at a given phase for a given rank
   *
   * \param[in] rank the rank
   * \param[in] phase the phase
   *
   * \return the number of objects
   */
  QOIVariantTypes getRankNumObjects(Rank rank, PhaseType phase) const {
    auto num_objects = static_cast<int>(rank.getNumObjects(phase));
    return num_objects;
  }

  /**
   * \brief Get the number of migratable objects at a given phase for a given rank
   *
   * \param[in] rank the rank
   * \param[in] phase the phase
   *
   * \return the number of migratable objects
   */
  QOIVariantTypes
  getRankNumMigratableObjects(Rank rank, PhaseType phase) const {
    auto num_migratable_objects = 0;
    auto const& phase_objects = rank.getPhaseWork().at(phase).getObjectWork();
    for (auto const& [obj_id, _] : phase_objects) {
      if (object_info_.at(obj_id).isMigratable()) {
        num_migratable_objects++;
      }
    }
    return num_migratable_objects;
  }

  /**
   * \brief Get the total load of migratable objects at a given phase for a given rank
   *
   * \param[in] rank the rank
   * \param[in] phase the phase
   *
   * \return the total load of migratable objects
   */
  QOIVariantTypes getRankMigratableLoad(Rank rank, PhaseType phase) const {
    auto migratable_load = 0.;
    auto const& phase_objects = rank.getPhaseWork().at(phase).getObjectWork();
    for (auto const& [obj_id, obj_work] : phase_objects) {
      if (object_info_.at(obj_id).isMigratable()) {
        migratable_load += obj_work.getLoad();
      }
    }
    return migratable_load;
  }

  /**
   * \brief Get the total load of sentinel objects at a given phase for a given rank
   *
   * \param[in] rank the rank
   * \param[in] phase the phase
   *
   * \return the total load of sentinel objects
   */
  QOIVariantTypes getRankSentinelLoad(Rank rank, PhaseType phase) const {
    auto sentinel_load = 0.;
    auto const& phase_objects = rank.getPhaseWork().at(phase).getObjectWork();
    for (auto const& [obj_id, obj_work] : phase_objects) {
      if (object_info_.at(obj_id).isSentinel()) {
        sentinel_load += obj_work.getLoad();
      }
    }
    return sentinel_load;
  }

  /**
   * \brief Check if a rank attribute exists
   *
   * \param[in] rank_qoi the qoi name
   *
   * \return whether it exists
   */
  bool hasRankAttribute(Rank const& rank, std::string const& rank_qoi) const {
    return rank.getAttributes().find(rank_qoi) != rank.getAttributes().end();
  }

  /**
    * \brief Get the specified attribute of a rank at a given phase
    *
    * \param[in] rank the current rank
    * \param[in] rank_qoi the attribute
    *
    * \return the requested attribute
    */
  QOIVariantTypes getRankAttribute(Rank const& rank, std::string rank_qoi) const {
    if (
      auto iter = rank.getAttributes().find(rank_qoi);
      iter != rank.getAttributes().end()
    ) {
      return iter->second;
    } else {
      throw std::runtime_error("Invalid Rank QOI: " + rank_qoi);
    }
  }

  /* ---------------------------------------------------------- */

  /**
   * \brief Serializer for data
   *
   * \param[in] s the serializer
   */
  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | object_info_;
    s | ranks_;
  }

private:
  /// All the object info that doesn't change across phases
  std::unordered_map<ElementIDType, ObjectInfo> object_info_;

  /// Work for each rank across phases
  std::unordered_map<NodeType, Rank> ranks_;

  /// The current phase (or indication to use all phases)
  PhaseType selected_phase_ = std::numeric_limits<PhaseType>::max();
};

} /* end namespace vt::tv */

#endif /*INCLUDED_VT_TV_API_INFO_H*/
