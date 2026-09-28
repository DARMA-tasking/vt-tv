#include "tv.h"

namespace vt::tv::bindings::python {

void tvFromJson(
  const std::vector<std::string>& input_json_per_rank_list,
  const std::string& input_yaml_params_str,
  uint64_t num_ranks) {
  utility::ParseRender pr(
    input_json_per_rank_list,
    input_yaml_params_str,
    num_ranks
  );
  pr.parseAndRender();
}

namespace nb = nanobind;
using namespace nb::literals;

NB_MODULE(vttv, m) {
  m.def("tvFromJson", &tvFromJson);
}

} /* end namespace vt::tv::bindings::python */
