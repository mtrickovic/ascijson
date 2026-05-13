#ifndef ASCIJSON_INCLUDE_JSON_HPP_
#define ASCIJSON_INCLUDE_JSON_HPP_

#include <cstddef>

namespace ascijson {

  // Error codes for the linear-scan process
  enum class Error {
    kNone = 0,
    kInvalidJson,
    kFieldNotFound,
    kMemoryError
  };

  // Core API for the foundational phase
  unsigned int CountFields(const char* json, const char* field_name);

  char* GetNthQuote(const char* json,
                    unsigned int quote_index,
                    const char* field_name);

} // namespace ascijson

#endif  // ASCIJSON_INCLUDE_JSON_H_
