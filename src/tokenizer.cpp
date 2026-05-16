#include "json.hpp"

namespace ascijson {

namespace {

inline void SetError(Error* out_error, Error err) {
  if (out_error) *out_error = err;
}

// Internal helper for string comparison (Zero-STL)
bool StringsAreEqual(const char* s1, const char* s2, size_t n) {
  for (size_t i = 0; i < n; ++i) {
    if (s1[i] != s2[i] || s1[i] == '\0') return false;
  }
  return true;
}

// Internal helper for string length (Zero-STL)
size_t GetStringLength(const char* s) {
  size_t len = 0;
  while (s[len] != '\0') len++;
  return len;
}

// Internal helper to skip whitespace (Google Style: internal linkage)
const char* SkipWhitespace(const char* cursor) {
  while (cursor && *cursor &&
         (*cursor == ' ' || *cursor == '\n' || *cursor == '\r' ||
          *cursor == '\t')) {
    cursor++;
  }
  return cursor;
}

// Safely skips a string literal and updates the cursor point
const char* SkipString(const char* cursor, Error* out_error) {
  if (!cursor || *cursor != '"') {
    SetError(out_error, Error::kInvalidJson);
    return nullptr;
  }
  cursor++; // Move past opening '"'
  while (*cursor != '\0' && *cursor != '"') {
    if (*cursor == '\\') {
      cursor++;
      if (*cursor == '\0') {
        SetError(out_error, Error::kInvalidJson);
        return nullptr;
      }
    }
    cursor++;
  }
  if (*cursor != '"') {
    SetError(out_error, Error::kInvalidJson);
    return nullptr;
  }
  return cursor + 1; // Return character immediately following closing '"'
}

// Moves the cursor past the current JSON value (string, object, array, or
// primitive)
const char* SkipValue(const char* cursor, Error* out_error) {
  cursor = SkipWhitespace(cursor);
  if (!cursor || *cursor == '\0') {
    SetError(out_error, Error::kInvalidJson);
    return nullptr;
  }

  if (*cursor == '"') {
    return SkipString(cursor, out_error);
  }

  if (*cursor == '{' || *cursor == '[') {
    // Skip Object or Array using brace counting
    char open = *cursor;
    char close = (open == '{') ? '}' : ']';
    int depth = 1;
    cursor++;

    while (cursor && *cursor != '\0' && depth > 0) {
      if (*cursor == '"') {
        cursor = SkipString(cursor, out_error);
        continue;
      }
      if (*cursor == open) depth++;
      else if (*cursor == close) depth--;

      if (depth > 0) {
        cursor++;
      }
    }

    if (depth != 0 || !cursor || *cursor == '\0') {
      SetError(out_error, Error::kInvalidJson);
      return nullptr;
    }
    return cursor + 1; // Step past the closing structure token
  }

  // Primitive parsing loop (number, true, false, null)
  while (*cursor != '\0' && *cursor != ',' && *cursor != '}' &&
         *cursor != ']' && *cursor != ' ' && *cursor != '\n' &&
         *cursor != '\r' && *cursor != '\t') {
    cursor++;
  }
  return cursor;
}

// Checks if the cursor points to the start of the specific key
bool IsMatch(const char* cursor, const char* field_name) {
  if (*cursor != '"') return false;
  cursor++;  // Move past the opening quote

  size_t len = GetStringLength(field_name);
  return StringsAreEqual(cursor, field_name, len) && cursor[len] == '"';
}

// Internal helper: only visible in this file
void CopyString(const char* source, char* dest, size_t dest_size) {
  size_t i = 0;
  while (source[i] != '"' && source[i] != '\0' && i < (dest_size - 1)) {
    dest[i] = source[i];
    i++;
  }
  dest[i] = '\0';
}

// Checks whether the cursor points at a specific literal token
// (e.g. "true", "false", "null") as a complete value - not a prefix.
bool IsLiteral(const char* cursor, const char* literal, size_t len) {
  if (!cursor) return false;
  for (size_t i = 0; i < len; ++i) {
    if (cursor[i] != literal[i]) return false;
  }
  // Guard against prefix matches: next char must be a value terminator.
  char next = cursor[len];
  return (next == ','  || next == '}' || next == ']' ||
          next == ' '  || next == '\n' || next == '\r' ||
          next == '\t' || next == '\0');
}


}  // namespace

unsigned int CountArrayElements(const char* json, Error* out_error) {
  SetError(out_error, Error::kNone);
  if (!json) return 0;

  const char* cursor = SkipWhitespace(json);
  if (*cursor != '[') {
    SetError(out_error, Error::kInvalidJson);
    return 0;
  }
  cursor++;

  cursor = SkipWhitespace(cursor);
  if (*cursor == ']') return 0;  // empty array

  unsigned int count = 1;
  while (cursor && *cursor != '\0' && *cursor != ']') {
    cursor = SkipValue(cursor, out_error);
    if (!cursor) return 0; // Propagate tracking error
    cursor = SkipWhitespace(cursor);
    if (*cursor == ',') {
      count++;
      cursor++;
    } else if (*cursor != ']') {
      SetError(out_error, Error::kInvalidJson);
      return 0;
    }
  }
  return count;
}

const char* GetNthElement(const char* json, unsigned int n, Error* out_error) {
  SetError(out_error, Error::kInvalidJson);
  if (!json) return nullptr;

  const char* cursor = SkipWhitespace(json);
  if (*cursor != '[') {
    SetError(out_error, Error::kInvalidJson);
    return nullptr;
  }
  cursor++;

  unsigned int idx = 0;
  while (cursor && *cursor != '\0') {
    cursor = SkipWhitespace(cursor);
    if (*cursor == ']' || *cursor == '\0') break;
    if (idx == n) return cursor;  // pointer to the Nth { ... }

    cursor = SkipValue(cursor, out_error);
    if (!cursor) return nullptr;
    cursor = SkipWhitespace(cursor);
    if (*cursor == ',') cursor++;
    idx++;
  }
  SetError(out_error, Error::kFieldNotFound);
  return nullptr;
}

// Returns a pointer to the value of a named key in a top-level object.
const char* FindValue(const char* json, const char* key, Error* out_error) {
  SetError(out_error, Error::kNone);
  if (!json || !key) return nullptr;

  const char* cursor = SkipWhitespace(json);
  if (!cursor || *cursor != '{') {
    SetError(out_error, Error::kInvalidJson);
    return nullptr;
  }
  cursor++;

  while (cursor && *cursor != '\0' && *cursor != '}') {
    cursor = SkipWhitespace(cursor);
    if (*cursor == '"') {
      bool matched = IsMatch(cursor, key);
      cursor = SkipValue(cursor, out_error);  // skip key string
      if (!cursor) return nullptr;
      cursor = SkipWhitespace(cursor);
      if (*cursor == ':') {
        cursor++;
        cursor = SkipWhitespace(cursor);
        if (matched) return cursor;  // return pointer to value
        cursor = SkipValue(cursor, out_error);
        if (!cursor) return nullptr;
      } else {
        SetError(out_error, Error::kInvalidJson);
        return nullptr;
      }
    } else {
      SetError(out_error, Error::kInvalidJson);
      return nullptr;
    }
    cursor = SkipWhitespace(cursor);
    if (*cursor == ',') cursor++;
  }

  if (!cursor || *cursor != '}') {
    SetError(out_error, Error::kInvalidJson);
    return nullptr;
  }

  SetError(out_error, Error::kFieldNotFound);
  return nullptr;
}

unsigned int CountFields(const char* json,
                         const char* field_name,
                         Error *out_error) {
  SetError(out_error, Error::kNone);
  if (!json || !field_name) return 0;

  unsigned int count = 0;
  const char* cursor = SkipWhitespace(json);

  if (*cursor != '{') {
    SetError(out_error, Error::kInvalidJson);
    return 0;
  }
  cursor++;

  while (cursor && *cursor != '\0' && *cursor != '}') {
    cursor = SkipWhitespace(cursor);
    if (*cursor == '"') {
      if (IsMatch(cursor, field_name)) {
        count++;
      }
      // Skip the key
      cursor = SkipValue(cursor, out_error);
      if (!cursor) return 0;
      cursor = SkipWhitespace(cursor);

      // Skip the colon and the value
      if (*cursor == ':') {
        cursor++;
        cursor = SkipValue(cursor, out_error);
        if (!cursor) return 0;
      } else {
        SetError(out_error, Error::kInvalidJson);
        return 0;
      }
    } else {
      SetError(out_error, Error::kInvalidJson);
      return 0;
    }
    cursor = SkipWhitespace(cursor);
    if (*cursor == ',') cursor++;
  }
  return count;
}

bool GetNthString(const char* json,
                  const char* field_name,
                  unsigned int n,
                  char* out_buffer,
                  size_t buffer_size,
                  Error* out_error) {
  SetError(out_error, Error::kInvalidJson);
  if (!json || !out_buffer || buffer_size == 0) {
    SetError(out_error, Error::kMemoryError);
    return false;
  }

  // If no field name, treat json as pointing directly at a string value
  if (!field_name) {
    const char* cursor = SkipWhitespace(json);
    if (*cursor != '"') {
      SetError(out_error, Error::kInvalidJson);
      return false;
    }
    cursor++;
    CopyString(cursor, out_buffer, buffer_size);
    return true;
  }

  const char* cursor = SkipWhitespace(json);
  if (*cursor != '{' && *cursor != '[') {
    SetError(out_error, Error::kInvalidJson);
    return false;
  }
  cursor++;

  unsigned int match_count = 0;
  while (cursor && *cursor != '\0') {
    cursor = SkipWhitespace(cursor);
    if (*cursor == '}' || *cursor == ']') break;

    if (*cursor == '"') {
      if (IsMatch(cursor, field_name)) {
        if (match_count == n) {
          // Found the Nth occurrence! Now find the value.
          cursor = SkipValue(cursor, out_error);  // Move the past key
          if (!cursor) return false;
          cursor = SkipWhitespace(cursor);
          if (*cursor == ':') {
            cursor++;
            cursor = SkipWhitespace(cursor);
            if (*cursor == '"') {
              cursor++;  // Move inside the quotes of the value
              CopyString(cursor, out_buffer, buffer_size);
              return true;
            }
          }
        }
        match_count++;
      }
      // Not the Nth one or not the right key, skip the key and the value
      cursor = SkipValue(cursor, out_error);
      if (!cursor) return false;
      cursor = SkipWhitespace(cursor);
      if (*cursor == ':') {
        cursor++;
        cursor = SkipValue(cursor, out_error);
        if (!cursor) return false;
      }
    } else {
      SetError(out_error, Error::kInvalidJson);
      return false;
    }

    cursor = SkipWhitespace(cursor);
    if (*cursor == ',') cursor++;
  }

  SetError(out_error, Error::kFieldNotFound);
  return false;
}

bool GetNthInt(const char* json,
               const char* field_name,
               unsigned int index,
               int* out_value,
               Error* out_error) {
  const char* val_ptr = FindValue(json, field_name, out_error);
  if (!val_ptr) return false;

  // If it's an array, jump to the Nth element
  if (*val_ptr == '[') {
    val_ptr = GetNthElement(val_ptr, index, out_error);
  }
  if (!val_ptr) return false;

  val_ptr = SkipWhitespace(val_ptr);

  int result = 0;
  int sign = 1;

  if (*val_ptr == '-') {
    sign = -1;
    val_ptr++;
  }

  bool has_digits = false;
  while (*val_ptr >= '0' && *val_ptr <= '9') {
    result = result * 10 + (*val_ptr - '0');
    val_ptr++;
    has_digits = true;
  }

  if (has_digits) {
    if (out_value) *out_value = result * sign;
    SetError(out_error, Error::kNone);
    return true;
  }
  SetError(out_error, Error::kInvalidJson);
  return false;
}

bool GetNthDouble(const char* json,
                  const char* field_name,
                  unsigned int index,
                  double* out_value,
                  Error* out_error) {
  const char* val_ptr = FindValue(json, field_name, out_error);
  if (!val_ptr) return false;

  if (*val_ptr == '[') {
    val_ptr = GetNthElement(val_ptr, index, out_error);
  }
  if (!val_ptr) return false;

  val_ptr = SkipWhitespace(val_ptr);
  double result = 0.0;
  double sign = 1.0;

  if (*val_ptr == '-') {
    sign = -1.0;
    val_ptr++;
  }

  // Integer Part
  bool has_digits = false;
  while (*val_ptr >= '0' && *val_ptr <= '9') {
    result = result * 10.0 + (*val_ptr - '0');
    val_ptr++;
    has_digits = true;
  }

  // Fractional Part
  if (*val_ptr == '.') {
    val_ptr++;
    double divisor = 10.0;
    while (*val_ptr >= '0' && *val_ptr <= '9') {
      result += (*val_ptr - '0') / divisor;
      divisor *= 10.0;
      val_ptr++;
      has_digits = true;
    }
  }

  if (has_digits) {
    if (out_value) *out_value = result * sign;
    SetError(out_error, Error::kNone);
    return true;
  }
  SetError(out_error, Error::kInvalidJson);
  return false;
}

bool IsTrue(const char* json, const char* field_name, Error* out_error) {
  const char* val = FindValue(json, field_name, out_error);
  if (!val) return false;
  return IsLiteral(val, "true", 4);
}

bool IsFalse(const char* json, const char* field_name, Error* out_error) {
  const char* val = FindValue(json, field_name, out_error);
  if (!val) return false;
  return IsLiteral(val, "false", 5);
}

bool IsNull(const char* json, const char* field_name, Error* out_error) {
  const char* val = FindValue(json, field_name, out_error);
  if (!val) return false;
  return IsLiteral(val, "null", 4);
}


}  // namespace ascijson
