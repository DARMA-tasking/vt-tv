#include "config_validator.h"

namespace vt::tv::bindings::python {

  bool ConfigValidator::isValid()
  {
      bool is_valid = true;
      for (std::string requiredParameter: requiredParameters) {
        if (!config[requiredParameter]) {
          is_valid = false;
          break;
        }
      }
      return is_valid;
  }

} /* end namespace vt::tv::bindings::python */
