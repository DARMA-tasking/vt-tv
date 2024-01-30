/*
//@HEADER
// *****************************************************************************
//
//                                  info.h
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
    std::unordered_map<NodeType, Rank> in_ranks
  ) : object_info_(std::move(in_object_info)),
      ranks_(std::move(in_ranks))
  { }

  Info() = default;

  /**
   * \brief Add more information about a new rank
   *
   * \param[in] object_info object information to merge with existing data
   * \param[in] r the rank work
   */
  void addInfo(
    std::unordered_map<ElementIDType, ObjectInfo> object_info, Rank r
  ) {
    for (auto x : object_info) {
      object_info_.try_emplace(x.first, std::move(x.second));
    }

    assert(ranks_.find(r.getRankID()) == ranks_.end() && "Rank must not exist");
    ranks_.try_emplace(r.getRankID(), std::move(r));
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
    uint64_t n_phases = this->ranks_.at(0).getNumPhases();
    for (NodeType rank_id = 1; rank_id < static_cast<NodeType>(this->ranks_.size()); rank_id++) {
      if (ranks_.at(rank_id).getNumPhases() != n_phases) {
        throw std::runtime_error("Number of phases must be consistent across ranks");
      }
    }
    return n_phases;
  }

  /**
   * \brief Get load of a given rank
   *
   * \param[in] rank the rank
   *
   * \return the rank load
   */
  double getRankLoad(NodeType rank, PhaseType phase) const { return ranks_.at(rank).getLoad(phase); }

  /**
   * \brief Get loads of all ranks at given phase
   *
   * \return a map of loads per rank
   */
  std::unordered_map<NodeType, double> getAllRankLoadsAtPhase(PhaseType phase) const {
    std::unordered_map<NodeType, double> rank_loads;

    for (auto const& [rank_id, rank] : this->ranks_) {
      rank_loads.insert(std::make_pair(rank_id, rank.getLoad(phase)));
    }

    return rank_loads;
  }

  /**
   * \brief Get loads of a given rank across all phases
   *
   * \return a map of loads per rank
   */
  std::unordered_map<PhaseType, double> getAllLoadsAtRank(NodeType rank_id) const {
    std::unordered_map<PhaseType, double> rank_loads;

    auto const& rank = this->ranks_.at(rank_id);
    auto const& phase_work = rank.getPhaseWork();
    for (auto const& [phase, _] : phase_work) {
      rank_loads.insert(std::make_pair(phase, rank.getLoad(phase)));
    }

    return rank_loads;
  }

  /**
   * \brief Get all objects for a given rank and phase
   *
   * \param[in] rank_id the rank
   * \param[in] phase the phase
   *
   * \return the objects
   */
  std::unordered_map<ElementIDType, ObjectWork> getRankObjects(NodeType rank_id, PhaseType phase) const {
    std::unordered_map<ElementIDType, ObjectWork> objects;

    // Get Rank info for specified rank
    auto const& rank_info = ranks_.at(rank_id);

    // Get history of phases for this rank
    auto const& phase_history_at_rank = rank_info.getPhaseWork();

    // Get phase work at specified phase
    auto const& phase_work_at_rank = phase_history_at_rank.find(phase);

    // Get all objects at specified phase
    auto const& object_work_at_phase_at_rank = phase_work_at_rank->second.getObjectWork();

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
  std::unordered_map<ElementIDType, ObjectWork> getPhaseObjects(PhaseType phase) const {
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
    for (PhaseType phase = 0; phase < n_phases; phase++) {
      auto const& objects = this->getPhaseObjects(phase);
      for (auto const& [obj_id, obj_work] : objects) {
        auto obj_max_v = obj_work.getMaxVolume();
        if (obj_max_v > ov_max) ov_max = obj_max_v;
      }
    }

    return ov_max;
  }

  /**
   * \brief Create mapping of all objects in all ranks for a given phase (made for allowing changes to these objects)
   *
   * \param[in] phase the phase
   *
   * \return the objects
   */
  std::unordered_map<ElementIDType, ObjectWork> createPhaseObjectsMapping(PhaseType phase) {
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
      auto phaseObjects = createPhaseObjectsMapping(phase);
      for (auto& [id1, objectWork1] : phaseObjects) {
        auto sent1 = objectWork1.getSent();
        auto received1 = objectWork1.getReceived();
        for (auto& [id2, objectWork2] : phaseObjects) {
          if (id1 != id2) {
            auto sent2 = objectWork2.getSent();
            auto received2 = objectWork2.getReceived();

            // Handle communications from object 2 to object 1
            auto its2 = sent2.equal_range(id1);
            for (auto it = its2.first; it != its2.second; ++it) {
              objectWork1.addReceivedCommunications(id2, it->second);
            }

            auto itr2 = received2.equal_range(id1);
            for (auto it = itr2.first; it != itr2.second; ++it) {
              objectWork1.addSentCommunications(id2, it->second);
            }

            // Handle communications from object 1 to object 2
            auto its1 = sent1.equal_range(id2);
            for (auto it = its1.first; it != its1.second; ++it) {
              objectWork2.addReceivedCommunications(id1, it->second);
            }

            auto itr1 = received1.equal_range(id2);
            for (auto it = itr1.first; it != itr1.second; ++it) {
              objectWork2.addSentCommunications(id1, it->second);
            }
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
      if (rank_max_load > max_load) max_load = rank_max_load;
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
   * \brief Get the received volume of an object at a given phase
   *
   * \param[in] obj_id the object id
   * \param[in] phase the phase
   *
   * \return the received volume
   */
   double getObjectReceivedVolume(ElementIDType obj_id, PhaseType phase) const {
     auto const& objects = this->getPhaseObjects(phase);
     auto const& obj = objects.at(obj_id);
     double received_volume = obj.getReceivedVolume();
     return received_volume;
   }

   /**
    * \brief Get the sent volume of an object at a given phase
    *
    * \param[in] obj_id the object id
    * \param[in] phase the phase
    *
    * \return the sent volume
    */
    double getObjectSentVolume(ElementIDType obj_id, PhaseType phase) const {
      auto const& objects = this->getPhaseObjects(phase);
      auto const& obj = objects.at(obj_id);
      double sent_volume = obj.getSentVolume();
      return sent_volume;
    }

  /* ---------------------------------------------------------- */

  /* -------------------- Rank QOI getters -------------------- */

  /**
   * \brief Get the received volume of a rank at a given phase
   *
   * \param[in] rank_id the rank id
   * \param[in] phase the phase
   *
   * \return the received volume
   */
  double getRankReceivedVolume(NodeType rank_id, PhaseType phase) const {
    auto const& rank = this->ranks_.at(rank_id);
    double received_volume = 0.;
    auto const& phase_objects = rank.getPhaseWork().at(phase).getObjectWork();
    for (auto const& [obj_id, obj_work] : phase_objects) {
      received_volume += obj_work.getReceivedVolume();
    }
    return received_volume;
  }

  /**
   * \brief Get the sent volume of a rank at a given phase
   *
   * \param[in] rank_id the rank id
   * \param[in] phase the phase
   *
   * \return the sent volume
   */
  double getRankSentVolume(NodeType rank_id, PhaseType phase) const {
    auto const& rank = this->ranks_.at(rank_id);
    double sent_volume = 0.;
    auto const& phase_objects = rank.getPhaseWork().at(phase).getObjectWork();
    for (auto const& [obj_id, obj_work] : phase_objects) {
      sent_volume += obj_work.getSentVolume();
    }
    return sent_volume;
  }

  /**
   * \brief Get the number of objects at a given phase for a given rank
   *
   * \param[in] rank_id the rank
   * \param[in] phase the phase
   *
   * \return the number of objects
   */
  uint64_t getRankNumObjects(NodeType rank_id, PhaseType phase) const {
    auto const& rank = this->ranks_.at(rank_id);
    uint64_t num_objects = rank.getNumObjects(phase);
    return num_objects;
  }

  /**
   * \brief Get the number of migratable objects at a given phase for a given rank
   *
   * \param[in] rank_id the rank
   * \param[in] phase the phase
   *
   * \return the number of migratable objects
   */
  uint64_t getRankNumMigratableObjects(NodeType rank_id, PhaseType phase) const {
    auto const& rank = this->ranks_.at(rank_id);
    uint64_t num_migratable_objects = 0;
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
   * \param[in] rank_id the rank
   * \param[in] phase the phase
   *
   * \return the total load of migratable objects
   */
  double getRankMigratableLoad(NodeType rank_id, PhaseType phase) const {
    auto const& rank = this->ranks_.at(rank_id);
    double migratable_load = 0.;
    auto const& phase_objects = rank.getPhaseWork().at(phase).getObjectWork();
    for (auto const& [obj_id, obj_work] : phase_objects) {
      if (object_info_.at(obj_id).isMigratable()) {
        migratable_load += obj_work.getLoad();
      }
    }
    return migratable_load;
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
};

} /* end namespace vt::tv */

#endif /*INCLUDED_VT_TV_API_INFO_H*/
