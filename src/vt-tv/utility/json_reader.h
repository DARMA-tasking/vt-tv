/*
//@HEADER
// *****************************************************************************
//
//                               json_reader.h
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

#if !defined INCLUDED_VT_TV_UTILITY_JSON_READER_H
#define INCLUDED_VT_TV_UTILITY_JSON_READER_H

#include "vt-tv/api/types.h"
#include "vt-tv/api/info.h"

#include <nlohmann/json.hpp>

#include <string>
#include <memory>

namespace vt::tv::utility {

/**
 * \struct JSONReader
 *
 * \brief Reader for JSON files in the LBDataType format.
 */
struct JSONReader {

  /**
   * \brief Construct the reader
   *
   * \param[in] in_filename the file name to read
   */
  JSONReader(NodeType in_rank, std::string const& in_filename)
    : rank_(in_rank),
      filename_(in_filename)
  { }

  /**
   * \brief Check if the file is compressed or not
   *
   * \return whether the file is compressed
   */
  bool isCompressed() const;

  /**
   * \brief Read the JSON file
   */
  void readFile();

  /**
   * \brief Parse the json into vt-tv's data structure Info, with a single rank
   * filled out
   */
  std::unique_ptr<Info> parseFile();

private:
  NodeType rank_ = 0;
  std::string filename_;
  std::unique_ptr<nlohmann::json> json_ = nullptr;
};

} /* end namespace vt::tv::utility */

#endif /*INCLUDED_VT_TV_UTILITY_JSON_READER_H*/
