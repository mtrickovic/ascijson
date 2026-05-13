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

  // Returns the count of a specific field at the current level.
  unsigned int CountFields(const char* json, const char* field_name);

  // Extracts the string value of the Nth occurrence of a field.
  // Returns true if found and copied, false otherwise.
  bool GetNthString(const char* json, const char* field_name, unsigned int n,
                    char* out_buffer, size_t buffer_size);

}  // namespace ascijson

#endif  // ASCIJSON_INCLUDE_JSON_HPP_
