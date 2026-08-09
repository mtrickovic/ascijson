#ifndef ASCIJSON_INCLUDE_JSON_HPP_
#define ASCIJSON_INCLUDE_JSON_HPP_

#include <cstddef>

namespace ascijson {

// Error codes for the linear-scan process and the writer.
enum class Error {
  kNone = 0,
  kInvalidJson,
  kFieldNotFound,
  kMemoryError,
  kBufferOverflow,
  kInvalidState
};

// Returns the count of a specific field at the current level.
unsigned int CountFields(const char* json,
                         const char* field_name,
                         Error* out_error = nullptr);

// Extracts the string value of the Nth occurrence of a field.
// Returns true if found and copied, false otherwise.
bool GetNthString(const char* json,
                  const char* field_name,
                  unsigned int n,
                  char* out_buffer,
                  size_t buffer_size,
                  Error* out_error = nullptr);

// Counts elements in a JSON array (pointer to '[').
unsigned int CountArrayElements(const char* array_json,
                                Error* out_error = nullptr);

// Returns pointer to the Nth element inside a json array.
const char* GetNthElement(const char* array_json,
                          unsigned int n,
                          Error* out_error = nullptr);

// Returns pointer to the value of a named key in a top-level JSON object.
const char* FindValue(const char* json, const char* key,
                      Error* out_error = nullptr);

// Extracts an integer value from a JSON field or array index.
bool GetNthInt(const char* json,
               const char* field_name,
               unsigned int index,
               int* out_value,
               Error* out_error = nullptr);

// Extracts a double-precision float from a JSON field or array index.
bool GetNthDouble(const char* json,
                  const char* field_name,
                  unsigned int index,
                  double* out_value,
                  Error* out_error = nullptr);

// Returns true if the named field's value is the literal `true`.
// Returns false if the field is absent, not a boolean, or is `false`.
bool IsTrue(const char* json,
            const char* field_name,
            Error* out_error = nullptr);

// Returns true if the named field's value is the literal `false`.
// Returns false if the field is absent, not a boolean, or is `true`.
bool IsFalse(const char* json,
             const char* field_name,
             Error* out_error = nullptr);

// Returns true if the named field's value is the literal `null`.
// Returns false if the field is absent or holds any other value.
bool IsNull(const char* json,
            const char* field_name,
            Error* out_error = nullptr);

// ===== Writer =====
//
// A zero-allocation JSON writer that serializes into a caller-provided
// buffer. All Writer state is fixed-size (stack-allocatable); Writer
// never performs heap allocation and never throws.
//
// Example:
//
//   char buf[256];
//   Writer w;
//   InitWriter(&w, buf, sizeof(buf));
//   BeginObject(&w);
//   WriteKey(&w, "name");
//   WriteString(&w, "Intel i3-9100F");
//   WriteKey(&w, "price");
//   WriteDouble(&w, 65.0);
//   EndObject(&w);
//   const char* json = WriterCStr(&w);  // {"name":"Intel i3-9100F","price":65}

// Maximum nesting depth (objects/arrays) the Writer can track. This is a
// compile-time bound chosen to keep Writer allocation-free; increase if
// you need deeper structures.
constexpr size_t kMaxWriterDepth = 32;

struct Writer {
  enum class Container { kNone, kObject, kArray };

  char* buffer;
  size_t capacity;
  size_t length;

  Container container[kMaxWriterDepth];
  bool has_element[kMaxWriterDepth];
  size_t depth;
  bool awaiting_value;

  Error last_error;
};

// Initializes `writer` to serialize into `buffer` (of `capacity` bytes).
// `buffer` must outlive the Writer and remain valid for all subsequent
// calls; Writer never frees it.
void InitWriter(Writer* writer, char* buffer, size_t capacity);

// Begins a JSON object ('{'). Returns false (and sets writer's error
// state) on buffer overflow or if max nesting depth is exceeded.
bool BeginObject(Writer* writer);

// Ends the current JSON object ('}'). Returns false if not currently
// inside an object, or on buffer overflow.
bool EndObject(Writer* writer);

// Begins a JSON array ('['). Returns false on overflow or max depth.
bool BeginArray(Writer* writer);

// Ends the current JSON array (']'). Returns false if not currently
// inside an array, or on buffer overflow.
bool EndArray(Writer* writer);

// Writes an object field key. Must be called while directly inside an
// object, immediately before the corresponding value call (WriteString /
// WriteInt / WriteDouble / WriteBool / WriteNull / BeginObject /
// BeginArray).
bool WriteKey(Writer* writer, const char* key);

// Writes a JSON string value, adding quotes and escaping internally
// (quotes, backslashes, and control characters).
bool WriteString(Writer* writer, const char* value);

// Writes a JSON integer value.
bool WriteInt(Writer* writer, int value);

// Writes a JSON double value with up to `precision` digits after the
// decimal point (default 6); trailing zeros are trimmed. Always uses
// '.' as the decimal separator regardless of the current C locale.
bool WriteDouble(Writer* writer, double value, int precision = 6);

// Writes a JSON boolean value (`true` or `false`).
bool WriteBool(Writer* writer, bool value);

// Writes a JSON `null` value.
bool WriteNull(Writer* writer);

// Returns a null-terminated pointer to the JSON text written so far, or
// nullptr if the writer is in an error state.
const char* WriterCStr(Writer* writer);

// Returns true if no error has occurred and every opened object/array
// has been closed (depth == 0). Check this before trusting WriterCStr()
// for a complete document.
bool WriterIsValid(const Writer* writer);

// Writes the JSON text currently held by `writer` to a file at `path`,
// in binary mode (no CRLF translation — output is byte-identical across
// Windows and Linux for the same input). Returns false if the writer is
// not in a valid, fully-closed state (see WriterIsValid), or if the file
// could not be opened or fully written.
bool WriteWriterToFile(const Writer* writer, const char* path);

}  // namespace ascijson

#endif  // ASCIJSON_INCLUDE_JSON_HPP_
