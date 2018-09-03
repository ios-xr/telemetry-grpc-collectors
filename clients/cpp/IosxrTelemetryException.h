#pragma once

#include <stdexcept>

namespace iosxr {

class IosxrTelemetryException : public std::runtime_error {
 public:
  explicit IosxrTelemetryException(const std::string& exception)
      : std::runtime_error(std::string("IosxrTelemetry exception: ") + exception) {}
};
} // namespace iosxr 
