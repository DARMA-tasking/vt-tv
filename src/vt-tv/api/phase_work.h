/*
//@HEADER
// *****************************************************************************
//
//                                phase_work.h
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

#if !defined INCLUDED_VT_TV_API_PHASE_WORK_H
#define INCLUDED_VT_TV_API_PHASE_WORK_H

#include "vt-tv/api/types.h"
#include "vt-tv/api/object_work.h"

#include <unordered_map>

namespace vt::tv {

/**
 * \struct PhaseWork
 *
 * \brief The work for a given phase
 */
struct PhaseWork {

  /**
   * \brief Construct phase work
   *
   * \param[in] in_phase the phase
   * \param[in] in_objects objects' work for the phase
   */
  PhaseWork(
    PhaseType in_phase,
    std::unordered_map<ElementIDType, ObjectWork> in_objects
  ) : phase_(in_phase),
      objects_(std::move(in_objects))
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
   * \brief set communications for an object in this phase
   *
   * \return void
   */
  void setCommunications(ElementIDType o_id, ObjectCommunicator& c) { objects_.at(o_id).setCommunications(c); };

private:
  /// Phase identifier
  PhaseType phase_ = 0;
  /// Object work for this phase
  std::unordered_map<ElementIDType, ObjectWork> objects_;
};

} /* end namesapce vt::tv */

#endif /*INCLUDED_VT_TV_API_PHASE_WORK_H*/
