#ifndef ASCIJSON_INCLUDE_JSON_HPP_
#define ASCIJSON_INCLUDE_JSON_HPP_

#include <cstddef>

namespace ascijson {

// Error codes for the linear-scan process
enum class Error { kNone = 0, kInvalidJson, kFieldNotFound, kMemoryError };

// Returns the count of a specific field at the current level.
unsigned int CountFields(const char* json, const char* field_name,
                         Error* out_error = nullptr);

// Extracts the string value of the Nth occurrence of a field.
// Returns true if found and copied, false otherwise.
bool GetNthString(const char* json, const char* field_name, unsigned int n,
                  char* out_buffer, size_t buffer_size,
                  Error* out_error = nullptr);

// Counts elements in a JSON array (pointer to '[').
unsigned int CountArrayElements(const char* array_json,
                                Error* out_error = nullptr);

// Returns pointer to the Nth element inside a json array.
const char* GetNthElement(const char* array_json, unsigned int n,
                          Error* out_error = nullptr);

// Returns pointer to the value of a named key in a top-level JSON object.
const char* FindValue(const char* json, const char* key,
                      Error* out_error = nullptr);

// Extracts an integer value from a JSON field or array index.
bool GetNthInt(const char* json, const char* field_name, unsigned int index,
               int* out_value, Error* out_error = nullptr);

// Extracts a double-precision float from a JSON field or array index.
bool GetNthDouble(const char* json, const char* field_name, unsigned int index,
                  double* out_value, Error* out_error = nullptr);

// Returns true if the named field's value is the literal `true`.
// Returns false if the field is absent, not a boolean, or is `false`.
bool IsTrue(const char* json, const char* field_name,
            Error* out_error = nullptr);

// Returns true if the named field's value is the literal `false`.
// Returns false if the field is absent, not a boolean, or is `true`.
bool IsFalse(const char* json, const char* field_name,
             Error* out_error = nullptr);

// Returns true if the named field's value is the literal `null`.
// Returns false if the field is absent or holds any other value.
bool IsNull(const char* json, const char* field_name,
            Error* out_error = nullptr);

}  // namespace ascijson

#endif  // ASCIJSON_INCLUDE_JSON_HPP_
