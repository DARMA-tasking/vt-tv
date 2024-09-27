#include "config_validator.h"

namespace vt::tv::bindings::python {

  /**
   * Check if the configuration file is valid
   *
   * @return true if the configuration is valid
   */
  bool ConfigValidator::isValid()
  {
      bool is_valid = true;
      for (std::string parameter: required_parameters) {
        if (!config[parameter]) {
          is_valid = false;
          break;
        }
      }
      return is_valid;
  }


  /**
   * Get the list of missing parameters
   *
   * @return A string containing the list of the missing parameters
   */
  std::string ConfigValidator::getMissingRequiredParameters()
  {
    std::string parameters;
    for (std::string parameter: required_parameters) {
      if (!config[parameter]) {
        if (parameters.empty()) {
          parameters = parameter;
        } else {
          parameters = parameters + ", " + parameter;
        }
      }
    }
    return parameters;
  }

} /* end namespace vt::tv::bindings::python */
