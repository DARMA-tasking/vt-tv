/*
//@HEADER
// *****************************************************************************
//
//                             json_generator.h
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

#if !defined INCLUDED_VT_TV_UTILITY_JSON_GENERATOR_H
#define INCLUDED_VT_TV_UTILITY_JSON_GENERATOR_H

#include "vt-tv/api/types.h"
#include "vt-tv/api/info.h"

#include <nlohmann/json.hpp>

#include <memory>

namespace vt::tv::utility {

/**
 * \struct JSONGenerator
 *
 * \brief Generates JSON from vt-tv data structures
 */
struct JSONGenerator {

  /**
   * \brief Construct the reader
   *
   * \param[in] in_filename the file name to read
   */
  JSONGenerator(Info const& in_info, NodeType in_rank, PhaseType in_phase)
    : info_(in_info),
      rank_(in_rank),
      phase_(in_phase)
  { }

  /**
   * \brief Generate JSON for given rank and phase
   *
   * \return the json
   */
  std::unique_ptr<nlohmann::json> generateJSON() const;

protected:
  /**
   * \internal \brief Output the meta-data for a given object
   *
   * \param[in] j where in the json to output it
   * \param[in] id the id of the object
   */
  void outputObjectMetaData(nlohmann::json& j, ElementIDType id) const;

private:
  Info const& info_;
  NodeType rank_ = 0;
  PhaseType phase_ = 0;
};

} /* end namesapce vt::tv::utility */

#endif /*INCLUDED_VT_TV_UTILITY_JSON_GENERATOR_H*/
