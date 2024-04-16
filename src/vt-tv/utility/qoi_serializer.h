/*
//@HEADER
// *****************************************************************************
//
//                            qoi_serializer.h
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

#if !defined INCLUDED_VT_TV_API_QOI_SERIALIZER_H
#define INCLUDED_VT_TV_API_QOI_SERIALIZER_H

#include "vt-tv/api/types.h"

#include <nlohmann/json.hpp>
#include <variant>

namespace nlohmann
{
  template <>
  struct adl_serializer<::vt::tv::QOIVariantTypes> {
    using VariantTypes = ::vt::tv::QOIVariantTypes;
    using ElementIDType = ::vt::tv::ElementIDType;

    // Produce compilation error if variant types were modified
    static_assert(std::is_same_v<int, std::variant_alternative_t<0, VariantTypes>>);
    static_assert(std::is_same_v<double, std::variant_alternative_t<1, VariantTypes>>);
    static_assert(std::is_same_v<std::string, std::variant_alternative_t<2, VariantTypes>>);
    static_assert(std::is_same_v<ElementIDType, std::variant_alternative_t<3, VariantTypes>>);

    static void to_json(json &j, const VariantTypes &value) {
      std::visit([&](auto const &arg)
                 { j = arg; },
                 value);
    }

    static void from_json(const json &j, VariantTypes &value) {
      if (j.is_number_unsigned()) {
        value = j.get<ElementIDType>();
      } else if (j.is_number_integer()) {
        value = j.get<int>();
      } else if (j.is_number_float()) {
        value = j.get<double>();
      } else if (j.is_string()) {
        value = j.get<std::string>();
      }
    }
  };

} /* end namespace nlohmann */

#endif /* INCLUDED_VT_TV_API_QOI_SERIALIZER_H */
