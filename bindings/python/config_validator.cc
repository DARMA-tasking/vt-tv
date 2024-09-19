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
      for (std::string requiredParameter: requiredParameters) {
        if (!config[requiredParameter]) {
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
    int i = 0;
    std::string parameters;
    for (std::string requiredParameter: requiredParameters) {
      if (!config[requiredParameter]) {
        if (i == 0 ) {
          parameters = parameters + requiredParameter; 
        } else {
          parameters = parameters + ", " + requiredParameter; 
        }
        i++;
      }
    }
    return parameters;
  }

} /* end namespace vt::tv::bindings::python */
