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
      while (cursor && *cursor && (*cursor == ' ' || *cursor == '\n' ||
             *cursor == '\r' || *cursor == '\t')) {
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
            cursor = SkipValue(cursor); // Reuse logic to skip strings
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
      cursor++;   // Move past the opening quote

      size_t len = GetStringLength(field_name);
      return StringsAreEqual(cursor, field_name, len) && cursor[len] == '"';
    }

  }  // namespace

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

}  // namespace ascijson
