#include "tv.h"

namespace nb = nanobind;
using namespace nb::literals;

NB_MODULE(tv, m) {
  m.def("process_json", &process_json);
}
