/*
//@HEADER
// *****************************************************************************
//
//                               object_info.h
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

#if !defined INCLUDED_VT_TV_API_OBJECT_INFO_H
#define INCLUDED_VT_TV_API_OBJECT_INFO_H

#include "vt-tv/api/types.h"

#include <vector>

namespace vt::tv {

/**
 * \struct ObjectInfo
 *
 * \brief Holds basic information about an object that does not change across
 * ranks or phases.
 */
struct ObjectInfo {

  /**
   * \brief Construct information about an object
   *
   * \param[in] in_id the unique global identifier for an object
   * \param[in] in_home the home rank for the object
   * \param[in] in_migratable whether it's migratable
   * \param[in] in_index the index for the object
   */
  ObjectInfo(
    ElementIDType in_id,
    NodeType in_home,
    bool in_migratable,
    std::vector<UniqueIndexBitType> const& in_index
  ) : id_(in_id),
      home_(in_home),
      migratable_(in_migratable),
      index_(in_index)
  { }

  /**
   * \brief Get the object's ID
   *
   * \return object ID
   */
  ElementIDType getID() const { return id_; }

private:
  /// Unique identifier across all ranks for the object
  ElementIDType id_ = 0;
  /// The rank the element started on
  NodeType home_ = 0;
  /// Whether it is migratable (if not, `home_` == `curr_`)
  bool migratable_ = false;
  /// Optional: index for the element
  std::vector<UniqueIndexBitType> index_;
};

} /* end namesapce vt::tv */

#endif /*INCLUDED_VT_TV_API_OBJECT_INFO_H*/
