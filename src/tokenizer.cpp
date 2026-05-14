#include "json.hpp"

namespace ascijson {

namespace {

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

// Moves the cursor past the current JSON value (string, object, etc.)
const char* SkipValue(const char* cursor) {
  cursor = SkipWhitespace(cursor);
  if (!cursor || *cursor == '\0') return nullptr;

  if (*cursor == '"') {
    // Skip String
    cursor++;
    while (*cursor != '\0' && *cursor != '"') {
      if (*cursor == '\\' && *(cursor + 1) != '\0') cursor++;
      cursor++;
    }
    if (*cursor == '"') cursor++;
  } else if (*cursor == '{' || *cursor == '[') {
    // Skip Object or Array using brace counting
    char open = *cursor;
    char close = (open == '{') ? '}' : ']';
    int depth = 1;
    cursor++;
    while (*cursor != '\0' && depth > 0) {
      if (*cursor == '"') {
        // Skip strings inside objects/arrays to avoid finding '}' in a str
        cursor = SkipValue(cursor);  // Reuse logic to skip strings
        continue;
      }
      if (*cursor == open) depth++;
      if (*cursor == close) depth--;
      cursor++;
    }
  } else {
    // Skip primitive (number, booleans, null)
    // Scan until comma, closing brace, or whitespace
    while (*cursor != '\0' && *cursor != ',' && *cursor != '}' &&
           *cursor != ']' && *cursor != ' ' && *cursor != '\n' &&
           *cursor != '\r' && *cursor != '\t') {
      cursor++;
    }
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

}  // namespace

unsigned int CountArrayElements(const char* json) {
  if (!json) return 0;
  const char* cursor = SkipWhitespace(json);
  if (*cursor != '[') return 0;
  cursor++;

  cursor = SkipWhitespace(cursor);
  if (*cursor == ']') return 0;  // empty array

  unsigned int count = 1;
  while (*cursor != '\0' && *cursor != ']') {
    cursor = SkipValue(cursor);
    cursor = SkipWhitespace(cursor);
    if (*cursor == ',') {
      count++;
      cursor++;
    }
  }
  return count;
}

const char* GetNthElement(const char* json, unsigned int n) {
  if (!json) return nullptr;
  const char* cursor = SkipWhitespace(json);
  if (*cursor != '[') return nullptr;
  cursor++;

  unsigned int idx = 0;
  while (*cursor != '\0') {
    cursor = SkipWhitespace(cursor);
    if (*cursor == ']' || *cursor == '\0') break;
    if (idx == n) return cursor;  // pointer to the Nth { ... }
    cursor = SkipValue(cursor);
    cursor = SkipWhitespace(cursor);
    if (*cursor == ',') cursor++;
    idx++;
  }
  return nullptr;
}

// Returns a pointer to the value of a named key in a top-level object.
const char* FindValue(const char* json, const char* key) {
  if (!json || !key) return nullptr;
  const char* cursor = SkipWhitespace(json);
  if (!cursor || *cursor != '{') return nullptr;
  cursor++;

  while (*cursor != '\0' && *cursor != '}') {
    cursor = SkipWhitespace(cursor);
    if (*cursor == '"') {
      bool matched = IsMatch(cursor, key);
      cursor = SkipValue(cursor);  // skip key string
      cursor = SkipWhitespace(cursor);
      if (*cursor == ':') {
        cursor++;
        cursor = SkipWhitespace(cursor);
        if (matched) return cursor;  // return pointer to value
        cursor = SkipValue(cursor);
      }
    } else {
      cursor++;
    }
    cursor = SkipWhitespace(cursor);
    if (*cursor == ',') cursor++;
  }
  return nullptr;
}

unsigned int CountFields(const char* json, const char* field_name) {
  if (!json || !field_name) return 0;

  unsigned int count = 0;
  const char* cursor = SkipWhitespace(json);

  if (*cursor != '{') return 0;
  cursor++;

  while (*cursor != '\0' && *cursor != '}') {
    cursor = SkipWhitespace(cursor);
    if (*cursor == '"') {
      if (IsMatch(cursor, field_name)) {
        count++;
      }
      // Skip the key
      cursor = SkipValue(cursor);
      cursor = SkipWhitespace(cursor);

      // Skip the colon and the value
      if (*cursor == ':') {
        cursor++;
        cursor = SkipValue(cursor);
      }
    } else {
      cursor++;
    }
    cursor = SkipWhitespace(cursor);
    if (*cursor == ',') cursor++;
  }

  return count;
}

bool GetNthString(const char* json, const char* field_name, unsigned int n,
                  char* out_buffer, size_t buffer_size) {
  if (!json || !out_buffer || buffer_size == 0) return false;

  // If no field name, treat json as pointing directly at a string value
  if (!field_name) {
    const char* cursor = SkipWhitespace(json);
    if (*cursor != '"') return false;
    cursor++;
    CopyString(cursor, out_buffer, buffer_size);
    return true;
  }

  const char* cursor = SkipWhitespace(json);
  if (*cursor != '{' && *cursor != '[') return false;
  cursor++;

  unsigned int match_count = 0;

  while (*cursor != '\0') {
    cursor = SkipWhitespace(cursor);
    if (*cursor == '}' || *cursor == ']') break;

    if (*cursor == '"') {
      if (IsMatch(cursor, field_name)) {
        if (match_count == n) {
          // Found the Nth occurrence! Now find the value.
          cursor = SkipValue(cursor);  // Move the past key
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
      cursor = SkipValue(cursor);
      cursor = SkipWhitespace(cursor);
      if (*cursor == ':') {
        cursor++;
        cursor = SkipValue(cursor);
      }
    } else {
      cursor++;
    }

    cursor = SkipWhitespace(cursor);
    if (*cursor == ',') cursor++;
  }

  return false;
}

}  // namespace ascijson
