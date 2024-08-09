/*
//@HEADER
// *****************************************************************************
//
//                              compressor.impl.h
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

#if !defined INCLUDED_VT_TV_UTILITY_COMPRESS_COMPRESSOR_IMPL_H
#define INCLUDED_VT_TV_UTILITY_COMPRESS_COMPRESSOR_IMPL_H

#include "vt-tv/utility/compressor.h"

#include <cassert>

namespace vt::tv::utility {

template <typename StreamLike>
bool Compressor::write(StreamLike& s, uint8_t const* buffer, std::size_t const size) {
  constexpr auto finish_writing = false;
  return writeImpl(s, buffer, size, finish_writing);
}

template <typename StreamLike>
bool Compressor::writeImpl(
  StreamLike& s, uint8_t const* buffer, std::size_t const size, bool finish_
) {
  assert(enc_ && "Must have a valid compressor");

  uint8_t const* cur = buffer;
  std::size_t rem = size;

  while (rem > 0 or (finish_ and not BrotliEncoderIsFinished(enc_))) {
    std::size_t avail_out = buf_size_;
    uint8_t* next_out = out_buf_.get();

    auto op = finish_ ? BROTLI_OPERATION_FINISH : BROTLI_OPERATION_PROCESS;
    bool ret = BrotliEncoderCompressStream(
      enc_, op, &rem, &cur, &avail_out, &next_out, nullptr
    );
    if (!ret) {
      assert(false && "Failed to stream compression data out\n");
      return false;
    }
    s.write(reinterpret_cast<char*>(out_buf_.get()), buf_size_ - avail_out);
  }

  return true;
}

template <typename StreamLike>
bool Compressor::finish(StreamLike& s) {
  constexpr auto finish_writing = true;
  return writeImpl(s, nullptr, 0, finish_writing);
}

} /* end namespace vt::tv::utility */

#endif /*INCLUDED_VT_TV_UTILITY_COMPRESS_COMPRESSOR_IMPL_H*/
