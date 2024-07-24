/*
//@HEADER
// *****************************************************************************
//
//                           test_json_reader.cc
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

#include <yaml-cpp/yaml.h>

#include <vt-tv/api/types.h>
#include <vt-tv/api/rank.h>
#include <vt-tv/api/object_work.h>
#include <vt-tv/api/object_info.h>
#include <vt-tv/api/info.h>

#include <vt-tv/utility/json_reader.h>

#include <string>

namespace vt::tv::tests::unit {

/**
 * Utility methods
 */
class Util {
    public:

        /**
         * \brief Execute a command on the underlying system and returns exit code and output
         * \throws {@link std::runtime_error} if an error occurs while opening the process
         */
        static std::tuple<int, std::string> exec(const char* cmd) {
            std::array<char, 128> buffer;
            std::string output;
            int status = 100;

            FILE* pipe = popen(cmd, "r");
            if (!pipe) throw std::runtime_error("popen() failed!");
            try {
                while (fgets(buffer.data(), static_cast<int>(buffer.size()), pipe) != nullptr) {
                    output += buffer.data();
                }
            } catch (...) {
                status = WEXITSTATUS(pclose(pipe));
                throw;
            }
            status = WEXITSTATUS(pclose(pipe));

            return std::make_tuple(status, output);
    }
};

}