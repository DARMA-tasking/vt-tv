/*
//@HEADER
// *****************************************************************************
//
//                               tv.h
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

#if !defined INCLUDED_VT_TV_BINDINGS_PYTHON_JSON_INTERFACE_H
#define INCLUDED_VT_TV_BINDINGS_PYTHON_JSON_INTERFACE_H

#include <string>

#include INCLUDE_FMT_FORMAT
#include "vt-tv/render/render.h"
#include "vt-tv/api/types.h"
#include "vt-tv/api/info.h"
#include "vt-tv/utility/decompression_input_container.h"
#include "vt-tv/utility/input_iterator.h"
#include "vt-tv/utility/qoi_serializer.h"
#include "vt-tv/utility/json_reader.h"

#include <nlohmann/json.hpp>
#include <yaml-cpp/yaml.h>

#include <nanobind/nanobind.h>
#include <nanobind/stl/string.h>
#include <nanobind/stl/vector.h>

#include <filesystem>
#include <map>

#ifdef VT_TV_OPENMP_ENABLED
#if VT_TV_OPENMP_ENABLED
    #include <omp.h>
#endif
#endif

namespace vt::tv::bindings::python {

void tvFromJson(const std::vector<std::string>&, const std::string&, uint64_t);

} /* end namespace vt::tv::bindings::python */

#endif /*INCLUDED_VT_TV_BINDINGS_PYTHON_JSON_INTERFACE_H*/
