/*
//@HEADER
// *****************************************************************************
//
//                     decompression_input_container.impl.h
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

#if !defined INCLUDED_VT_TV_UTILITY_JSON_DECOMPRESSION_INPUT_CONTAINER_IMPL_H
#define INCLUDED_VT_TV_UTILITY_JSON_DECOMPRESSION_INPUT_CONTAINER_IMPL_H

namespace vt::tv::utility {

template <typename StreamLike>
DecompressionInputContainer::DecompressionInputContainer(
  AnyStreamTag, StreamLike stream, std::size_t in_chunk_size)
  : chunk_size_(in_chunk_size) {
  d_ = std::make_unique<DecompressorStreamType<StreamLike>>(std::move(stream));
  output_buf_ = std::make_unique<uint8_t[]>(chunk_size_);
  len_ = d_->read(output_buf_.get(), chunk_size_);
}

} /* end namespace vt::tv::utility */

#endif /*INCLUDED_VT_TV_UTILITY_JSON_DECOMPRESSION_INPUT_CONTAINER_IMPL_H*/
