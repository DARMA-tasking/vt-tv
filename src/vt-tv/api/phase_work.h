/*
//@HEADER
// *****************************************************************************
//
//                                 phase_work.h
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

#if !defined INCLUDED_VT_TV_API_PHASE_WORK_H
#define INCLUDED_VT_TV_API_PHASE_WORK_H

#include "vt-tv/api/types.h"
#include "vt-tv/api/object_work.h"

#include <unordered_map>

namespace vt::tv {

/**
 * \struct WorkDistribution
 *
 * \brief The work distribution for a phase or iteration
 */
struct WorkDistribution {
  WorkDistribution() = default;

  /**
   * \brief Construct work distribution
   *
   * \param[in] in_phase the phase for the work distribution
   * \param[in] in_objects objects' work for the phase
   * \param[in] in_user_defined the user-defined fields in json
   */
  WorkDistribution(
    PhaseType in_phase,
    std::unordered_map<ElementIDType, ObjectWork> in_objects,
    std::unordered_map<std::string, QOIVariantTypes> in_user_defined = {}
  )
    : phase_(in_phase),
      objects_(std::move(in_objects)),
      user_defined_(std::move(in_user_defined))
  { }

  /**
   * \brief Get the phase ID
   *
   * \return the phase ID
   */
  PhaseType getPhase() const { return phase_; }

  /**
   * \brief Get object work
   *
   * \return the object work
   */
  auto const& getObjectWork() const { return objects_; }

  /**
   * \brief Get the phase load (corresponds to rank load as a PhaseWork belongs to a Rank)
   *
   * \return the phase load
   */
  double getLoad() const {
    double load = 0.;
    for (auto const& [id, obj_work] : objects_) {
      load += obj_work.getLoad();
    }
    return load;
  }

  /**
   * \brief set communications for an object in this phase
   *
   * \return void
   */
  void setCommunications(ElementIDType o_id, ObjectCommunicator& c) {
    objects_.at(o_id).setCommunications(c);
  };

  /**
   * \brief add a received communication to an object in this phase
   *
   * \return void
   */
  void addObjectReceivedCommunication(
    ElementIDType o_id, ElementIDType from_id, double bytes) {
    objects_.at(o_id).addReceivedCommunications(from_id, bytes);
  };

  /**
   * \brief add a sent communication to an object in this phase
   *
   * \return void
   */
  void addObjectSentCommunication(
    ElementIDType o_id, ElementIDType to_id, double bytes) {
    objects_.at(o_id).addSentCommunications(to_id, bytes);
  };

  /**
   * \brief Get maximum bytes received or sent between objects at this phase
   */
  double getMaxVolume() const {
    double ov_max = 0.;

    for (auto const& [obj_id, obj_work] : this->objects_) {
      auto obj_max_v = obj_work.getMaxVolume();
      if (obj_max_v > ov_max)
        ov_max = obj_max_v;
    }
    return ov_max;
  }

  /**
   * \brief Get user-defined fields
   *
   * \return user-defined fields
   */
  auto const& getUserDefined() const { return user_defined_; }

  /**
   * \brief Serializer for data
   *
   * \param[in] s the serializer
   */
  template <typename SerializerT>
  void serialize(SerializerT& s) {
    s | phase_;
    s | objects_;
    s | user_defined_;
  }

private:
  /// Phase identifier
  PhaseType phase_ = 0;
  /// Object work for this phase
  std::unordered_map<ElementIDType, ObjectWork> objects_;
  // User-defined field---used to populate the rank-level info
  std::unordered_map<std::string, QOIVariantTypes> user_defined_;
};

/**
 * \struct LBIteration
 *
 * \brief An LB iteration within a phase where the distribution is changing
 */
struct LBIteration : WorkDistribution {
  LBIteration() = default;

  /**
   * \brief Construct a LB iteration
   *
   * \param[in] in_phase the phase
   * \param[in] in_lb_iteration the LB iteration
   * \param[in] in_objects objects' work for the phase
   * \param[in] in_user_defined the user-defined fields in json
   */
  LBIteration(
    PhaseType in_phase,
    LBIterationType in_lb_iteration,
    std::unordered_map<ElementIDType, ObjectWork> in_objects,
    std::unordered_map<std::string, QOIVariantTypes> in_user_defined = {}
  ) : WorkDistribution(in_phase, std::move(in_objects), std::move(in_user_defined)),
      lb_iteration_(in_lb_iteration)
  { }

  /**
   * \brief Get the LB iteration ID
   *
   * \return the LB iteration ID
   */
  LBIterationType getLBIterationID() const { return lb_iteration_; }

  /**
   * \brief Serializer for data
   *
   * \param[in] s the serializer
   */
  template <typename SerializerT>
  void serialize(SerializerT& s) {
    WorkDistribution::serialize(s);
    s | lb_iteration_;
  }

private:
  LBIterationType lb_iteration_ = 0; /**< The LB iteration */
};

/**
 * \struct PhaseWork
 *
 * \brief The work for a phase which may include multiple subsequent LB
 * iterations
 */
struct PhaseWork : WorkDistribution {
  PhaseWork() = default;

  /**
   * \brief Construct a phase work
   *
   * \param[in] in_phase the phase
   * \param[in] in_objects objects' work for the phase
   * \param[in] in_user_defined the user-defined fields in json
   */
  PhaseWork(
    PhaseType in_phase,
    std::unordered_map<ElementIDType, ObjectWork> in_objects,
    std::unordered_map<std::string, QOIVariantTypes> in_user_defined = {}
  ) : WorkDistribution(in_phase, std::move(in_objects), std::move(in_user_defined))
  { }

  /**
   * \brief Add an LB iteration
   *
   * \param[in] lb_iter_id the iteration ID
   * \param[in] lb_iter the work distribution for the iteration
   */
  void addLBIteration(LBIterationType lb_iter_id, LBIteration lb_iter) {
    lb_iters_.emplace(lb_iter_id, std::move(lb_iter));
  }

  /**
   * \brief Get all LB iterations
   *
   * \return the const LB iterations
   */
  auto const& getLBIterations() const { return lb_iters_; }

  /**
   * \brief Get a LB iteration
   *
   * \param[in] lb_iter_id the LB iteration ID
   *
   * \return ref to the lb iteration
   */
  LBIteration& getLBIteration(LBIterationType lb_iter_id) {
    return lb_iters_.find(lb_iter_id)->second;
  }

  /**
   * \brief Get a const LB iteration
   *
   * \param[in] lb_iter_id the LB iteration ID
   *
   * \return ref to the lb iteration
   */
  LBIteration const& getLBIteration(LBIterationType lb_iter_id) const {
    return lb_iters_.find(lb_iter_id)->second;
  }

  /**
   * \brief Serializer for data
   *
   * \param[in] s the serializer
   */
  template <typename SerializerT>
  void serialize(SerializerT& s) {
    WorkDistribution::serialize(s);
    s | lb_iters_;
  }

private:
  /// LB iterations for this phase
  std::map<LBIterationType, LBIteration> lb_iters_;
};



} /* end namespace vt::tv */

#endif /*INCLUDED_VT_TV_API_PHASE_WORK_H*/
