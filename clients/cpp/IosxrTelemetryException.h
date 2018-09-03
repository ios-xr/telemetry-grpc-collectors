#pragma once

#include <stdexcept>

#include <folly/Format.h>

namespace iosxr {

class IosxrTelemetryException : public std::runtime_error {
 public:
  explicit IosxrTelemetryException(const std::string& exception)
      : std::runtime_error(
            folly::sformat("IosxrTelemetry exception: {} ", exception)) {}
};
} // namespace iosxr 
