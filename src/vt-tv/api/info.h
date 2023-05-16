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
  auto& getObjectInfo() const { return object_info_; }

  /**
   * \brief Get work for a given rank
   *
   * \param[in] rank the rank
   *
   * \return all the rank work
   */
  Rank const& getRank(NodeType rank) const { return ranks_.at(rank); }

  /**
   * \brief Get all objects for a given rank and phase
   *
   * \param[in] rank_id the rank
   * \param[in] phase the phase
   *
   * \return the objects
   */
  std::unordered_map<ElementIDType, ObjectWork> getRankObjects(NodeType rank_id, PhaseType phase) {
    std::unordered_map<ElementIDType, ObjectWork> objects;

    // Get Rank info for specified rank
    auto& rank_info = this->getRank(rank_id);

    // Get history of phases for this rank
    auto& phase_history_at_rank = rank_info.getPhaseWork();

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
   * \param[in] n_ranks the total number of ranks
   *
   * \return the objects
   */
  std::unordered_map<ElementIDType, ObjectWork> getPhaseObjects(PhaseType phase, uint64_t n_ranks) {
    // fmt::print("Phase: {}\n", phase);

    // Map of objects at given phase
    std::unordered_map<ElementIDType, ObjectWork> objects_at_phase;

    // Go through all ranks and get all objects at given phase
    for (uint64_t rank = 0; rank < n_ranks; rank++) {
      // fmt::print("  Rank: {}\n",rank);
      // Get Rank info for specified rank
      auto& rank_info = this->getRank(rank);

      // Get history of phases for this rank
      auto& phase_history = rank_info.getPhaseWork();

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
   * \brief Get all objects for all ranks and phases
   *
   * \param[in] n_ranks the total number of ranks
   *
   * \return the objects
   */
  std::unordered_map<ElementIDType, ObjectWork> getAllObjects(uint64_t n_ranks) {

    // Map of objects at given phase
    std::unordered_map<ElementIDType, ObjectWork> objects;

    // Go through all ranks and get all objects at given phase
    for (uint64_t rank = 0; rank < n_ranks; rank++) {
      // fmt::print("Rank: {}\n",rank);
      // Get Rank info for specified rank
      auto& rank_info = this->getRank(rank);

      // Get history of phases for this rank
      auto& phase_history = rank_info.getPhaseWork();

      // Go through history of all phases
      for (auto const& [phase, phase_work] : phase_history) {
        // fmt::print("|->  Phase: {}\n", phase);
        // Get all objects at this phase
        auto const& object_work_at_phase = phase_work.getObjectWork();

        for (auto const& [elm_id, obj_work] : object_work_at_phase) {
          // fmt::print("|    |-> Object Id: {}\n", elm_id);
          objects.insert(std::make_pair(elm_id, obj_work));
        }
      }
    }
    fmt::print("Size of all objects: {}", objects.size());
    return objects;
  }

  /**
   * \brief Get number of ranks stored here
   *
   * \return number of ranks
   */
  std::size_t getNumRanks() const { return ranks_.size(); }

private:
  /// All the object info that doesn't change across phases
  std::unordered_map<ElementIDType, ObjectInfo> object_info_;

  /// Work for each rank across phases
  std::unordered_map<NodeType, Rank> ranks_;
};

} /* end namesapce vt::tv */

#endif /*INCLUDED_VT_TV_API_INFO_H*/
