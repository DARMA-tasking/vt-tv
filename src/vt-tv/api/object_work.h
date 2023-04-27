/*
//@HEADER
// *****************************************************************************
//
//                               object_work.h
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

#if !defined INCLUDED_VT_TV_API_OBJECT_WORK_H
#define INCLUDED_VT_TV_API_OBJECT_WORK_H

#include "vt-tv/api/types.h"

#include <unordered_map>
#include <vector>
#include <variant>
#include <string>

namespace vt::tv {

/**
 * \struct ObjectWork
 *
 * \brief Holds work for an object for a given phase
 */
struct ObjectWork {
  /// Possible user-defined types for a task
  using VariantType = std::variant<int, double, std::string>;

  /**
   * \brief Construct \c ObjectWork for a given phase
   *
   * \param[in] in_id the object identifier
   * \param[in] in_whole_phase_load the load for the whole phase
   * \param[in] in_subphase_loads the subphase load breakdown
   * \param[in] in_user_defined the user-defined fields in json
   */
  ObjectWork(
    ElementIDType in_id,
    TimeType in_whole_phase_load,
    std::unordered_map<SubphaseType, TimeType> in_subphase_loads,
    std::unordered_map<std::string, VariantType> in_user_defined = {}
  ) : id_(in_id),
      whole_phase_load_(in_whole_phase_load),
      subphase_loads_(std::move(in_subphase_loads)),
      user_defined_(std::move(in_user_defined))
  { }

  /**
   * \brief Get element ID
   *
   * \return the element ID
   */
  ElementIDType getID() const { return id_; }

  /**
   * \brief Get the load (in time) for the whole phase
   *
   * \return the load
   */
  TimeType getLoad() const { return whole_phase_load_; }

  /**
   * \brief Get the subphase loads
   *
   * \return the subphase loads
   */
  auto const& getSubphaseLoads() { return subphase_loads_; }

private:
  /// Element ID
  ElementIDType id_ = 0;
  /// Load for the object for a given phaseElementIDType
  TimeType whole_phase_load_ = 0.;
  /// Load broken down into subphases
  std::unordered_map<SubphaseType, TimeType> subphase_loads_;
  // User-defined field---used to populate the memory block
  std::unordered_map<std::string, VariantType> user_defined_;

  /// @todo: add communications
};

} /* end namesapce vt::tv */

#endif /*INCLUDED_VT_TV_API_OBJECT_WORK_H*/
