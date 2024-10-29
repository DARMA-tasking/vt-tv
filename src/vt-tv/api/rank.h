/*
//@HEADER
// *****************************************************************************
//
//                                    rank.h
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

#if !defined INCLUDED_VT_TV_API_RANK_H
#define INCLUDED_VT_TV_API_RANK_H

#include "vt-tv/api/phase_work.h"

namespace vt::tv {

/**
 * \struct Rank
 *
 * \brief All the data for a given \c Rank
 */
struct Rank {
  Rank() = default;

  /**
   * \brief Construct a rank data
   *
   * \param[in] in_rank the rank
   * \param[in] in_phase_info all the phase info
   */
  Rank(
    NodeType in_rank,
    std::unordered_map<PhaseType, PhaseWork> in_phase_info,
    std::unordered_map<std::string, QOIVariantTypes> in_user_defined = {},
    std::unordered_map<std::string, QOIVariantTypes> in_attributes = {})
    : rank_(in_rank),
      phase_info_(std::move(in_phase_info)),
      user_defined_(std::move(in_user_defined)),
      attributes_(std::move(in_attributes)) { }

  /**
   * \brief Get the rank ID
   *
   * \return rank ID
   */
  NodeType getRankID() const { return rank_; }

  /**
   * \brief Get all the phase work
   *
   * \return the phase work
   */
  auto const& getPhaseWork() const { return phase_info_; }

  /**
   * \brief Get number of phases on this rank
   *
   * \return the number of phases
   */
  uint64_t getNumPhases() const { return phase_info_.size(); }

  /**
   * \brief Get total load of objects at given phase
   *
   * \return the load
   */
  double getLoad(PhaseType phase) const {
    return phase_info_.at(phase).getLoad();
  }

  /**
   * \brief Get user_defined fields
   *
   * \return user_defined fields
   */
  auto const& getUserDefined() const { return user_defined_; }

  /**
   * \brief Get attribute fields
   *
   * \return attribute fields
   */
  auto const& getAttributes() const { return attributes_; }

  /**
   * \brief Get number of objects at given phase on this rank
   *
   * \param[in] phase the phase
   *
   * \return the number of objects
   */
  uint64_t getNumObjects(PhaseType phase) const {
    return phase_info_.at(phase).getObjectWork().size();
  }

  /**
  * \brief add a received communication to an object at a given phase
  *
  * \return void
  */
  void addObjectReceivedCommunicationAtPhase(
    PhaseType phase_id,
    ElementIDType o_id,
    ElementIDType from_id,
    double bytes) {
    phase_info_.at(phase_id).addObjectReceivedCommunication(
      o_id, from_id, bytes);
  };

  /**
   * \brief add a sent communication to an object at a given phase
   *
   * \return void
   */
  void addObjectSentCommunicationAtPhase(
    PhaseType phase_id, ElementIDType o_id, ElementIDType to_id, double bytes) {
    phase_info_.at(phase_id).addObjectSentCommunication(o_id, to_id, bytes);
  };

  /**
   * \brief Serializer for data
   *
   * \param[in] s the serializer
   */
  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | rank_;
    s | phase_info_;
    s | user_defined_;
    s | attributes_;
  }

private:
  /// The rank ID
  NodeType rank_ = 0;
  /// Work for each phase
  std::unordered_map<PhaseType, PhaseWork> phase_info_;
  /// QOIs to be visualized
  std::unordered_map<std::string, QOIVariantTypes> user_defined_;
  std::unordered_map<std::string, QOIVariantTypes> attributes_;
};

} /* end namespace vt::tv */

#endif /*INCLUDED_VT_TV_API_RANK_H*/
