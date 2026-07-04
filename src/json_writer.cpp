#include <cinttypes>
#include <clocale>
#include <cstdio>

#include "json.hpp"

namespace ascijson {

namespace {

// Bounds-checked raw append. Always reserves 1 byte of capacity for the
// null terminator WriterCStr() will eventually write, so a successful
// sequence of AppendRaw() calls guarantees WriterCStr() can terminate.
bool AppendRaw(Writer* writer, const char* data, size_t len) {
  if (writer->last_error != Error::kNone) {
    return false;
  }
  if (writer->capacity == 0) {
    writer->last_error = Error::kBufferOverflow;
    return false;
  }

  size_t usable = writer->capacity - 1;
  if (writer->length > usable || len > usable - writer->length) {
    writer->last_error = Error::kBufferOverflow;
    return false;
  }

  for (size_t i = 0; i < len; ++i) {
    writer->buffer[writer->length + i] = data[i];
  }
  writer->length += len;
  return true;
}

bool AppendEscapedString(Writer* writer, const char* value) {
  if (!AppendRaw(writer, "\"", 1)) {
    return false;
  }

  for (const char* p = value; *p != '\0'; ++p) {
    unsigned char c = static_cast<unsigned char>(*p);
    switch (c) {
      case '"':
        if (!AppendRaw(writer, "\\\"", 2)) return false;
        break;
      case '\\':
        if (!AppendRaw(writer, "\\\\", 2)) return false;
        break;
      case '\n':
        if (!AppendRaw(writer, "\\n", 2)) return false;
        break;
      case '\t':
        if (!AppendRaw(writer, "\\t", 2)) return false;
        break;
      case '\r':
        if (!AppendRaw(writer, "\\r", 2)) return false;
        break;
      case '\b':
        if (!AppendRaw(writer, "\\b", 2)) return false;
        break;
      case '\f':
        if (!AppendRaw(writer, "\\f", 2)) return false;
        break;
      default:
        if (c < 0x20) {
          char buf[7];
          snprintf(buf, sizeof(buf), "\\u%04x", c);
          if (!AppendRaw(writer, buf, 6)) return false;
        } else {
          char ch = static_cast<char>(c);
          if (!AppendRaw(writer, &ch, 1)) return false;
        }
    }
  }

  return AppendRaw(writer, "\"", 1);
}

// Inserts a leading comma if this value isn't the first member/element
// at the current depth. No-op if a key was just written via WriteKey()
// (the key already handled the comma for its value).
void BeforeValue(Writer* writer) {
  if (writer->awaiting_value) {
    writer->awaiting_value = false;
    return;
  }
  if (writer->depth > 0) {
    if (writer->has_element[writer->depth - 1]) {
      AppendRaw(writer, ",", 1);
    }
    writer->has_element[writer->depth - 1] = true;
  }
}

// Cross-platform fopen wrapper: avoids MSVC's fopen_s deprication
// warning without blanket-disabling secure-CRT warnings project-wide.
FILE* OpenFile(const char* path, const char* mode) {
#if defined(_MSC_VER)
  FILE* fp = nullptr;
  fopen_s(&fp, path, mode);
  return fp;
#else
  return fopen(path, mode);
#endif
}

}  // namespace

void InitWriter(Writer* writer, char* buffer, size_t capacity) {
  writer->buffer = buffer;
  writer->capacity = capacity;
  writer->length = 0;
  writer->depth = 0;
  writer->awaiting_value = false;
  writer->last_error = Error::kNone;
}

bool BeginObject(Writer* writer) {
  BeforeValue(writer);
  if (writer->depth >= kMaxWriterDepth) {
    writer->last_error = Error::kBufferOverflow;
    return false;
  }
  if (!AppendRaw(writer, "{", 1)) {
    return false;
  }
  writer->container[writer->depth] = Writer::Container::kObject;
  writer->has_element[writer->depth] = false;
  writer->depth++;
  return true;
}

bool EndObject(Writer* writer) {
  if (writer->depth == 0 ||
      writer->container[writer->depth - 1] != Writer::Container::kObject) {
    writer->last_error = Error::kInvalidState;
    return false;
  }
  writer->depth--;
  return AppendRaw(writer, "}", 1);
}

bool BeginArray(Writer* writer) {
  BeforeValue(writer);
  if (writer->depth >= kMaxWriterDepth) {
    writer->last_error = Error::kBufferOverflow;
    return false;
  }
  if (!AppendRaw(writer, "[", 1)) {
    return false;
  }
  writer->container[writer->depth] = Writer::Container::kArray;
  writer->has_element[writer->depth] = false;
  writer->depth++;
  return true;
}

bool EndArray(Writer* writer) {
  if (writer->depth == 0 ||
      writer->container[writer->depth - 1] != Writer::Container::kArray) {
    writer->last_error = Error::kInvalidState;
    return false;
  }
  writer->depth--;
  return AppendRaw(writer, "]", 1);
}

bool WriteKey(Writer* writer, const char* key) {
  if (writer->last_error != Error::kNone) {
    return false;
  }
  if (writer->depth == 0 ||
      writer->container[writer->depth - 1] != Writer::Container::kObject) {
    writer->last_error = Error::kInvalidState;
    return false;
  }

  if (writer->has_element[writer->depth - 1]) {
    if (!AppendRaw(writer, ",", 1)) return false;
  }
  writer->has_element[writer->depth - 1] = true;

  if (!AppendEscapedString(writer, key)) return false;
  if (!AppendRaw(writer, ":", 1)) return false;

  writer->awaiting_value = true;
  return true;
}

bool WriteString(Writer* writer, const char* value) {
  BeforeValue(writer);
  return AppendEscapedString(writer, value);
}

bool WriteInt(Writer* writer, int value) {
  BeforeValue(writer);
  char buf[16];
  int n = snprintf(buf, sizeof(buf), "%d", value);
  if (n < 0 || static_cast<size_t>(n) >= sizeof(buf)) {
    writer->last_error = Error::kMemoryError;
    return false;
  }
  return AppendRaw(writer, buf, static_cast<size_t>(n));
}

bool WriteDouble(Writer* writer, double value, int precision) {
  BeforeValue(writer);

  char buf[64];
  int n = snprintf(buf, sizeof(buf), "%.*f", precision, value);
  if (n < 0 || static_cast<size_t>(n) >= sizeof(buf)) {
    writer->last_error = Error::kMemoryError;
    return false;
  }

  // Force '.' as the decimal separator regardless of the current C
  // locale (some locales use ',' — invalid in JSON).
  const struct lconv* lc = localeconv();
  if (lc != nullptr && lc->decimal_point != nullptr &&
      lc->decimal_point[0] != '\0' && lc->decimal_point[0] != '.') {
    for (int i = 0; i < n; ++i) {
      if (buf[i] == lc->decimal_point[0]) {
        buf[i] = '.';
        break;
      }
    }
  }

  // Trim trailing zeros (and a trailing lone '.') for cleaner output:
  // 65.0 -> "65", 65.50 -> "65.5".
  if (precision > 0) {
    int end = n;
    while (end > 0 && buf[end - 1] == '0') {
      --end;
    }
    if (end > 0 && buf[end - 1] == '.') {
      --end;
    }
    n = end;
  }

  return AppendRaw(writer, buf, static_cast<size_t>(n));
}

bool WriteBool(Writer* writer, bool value) {
  BeforeValue(writer);
  return value ? AppendRaw(writer, "true", 4) : AppendRaw(writer, "false", 5);
}

bool WriteNull(Writer* writer) {
  BeforeValue(writer);
  return AppendRaw(writer, "null", 4);
}

const char* WriterCStr(Writer* writer) {
  if (writer->last_error != Error::kNone) {
    return nullptr;
  }
  if (writer->length >= writer->capacity) {
    return nullptr;
  }
  writer->buffer[writer->length] = '\0';
  return writer->buffer;
}

bool WriterIsValid(const Writer* writer) {
  return writer->last_error == Error::kNone && writer->depth == 0;
}

bool WriteWriterToFile(const Writer* writer, const char* path) {
  if (!WriterIsValid(writer)) {
    return false;
  }

  FILE* fp = OpenFile(path, "wb");  // binary mode: no CRLF translation
  if (fp == nullptr) {
    return false;
  }

  size_t written = fwrite(writer->buffer, 1, writer->length, fp);
  fclose(fp);

  return written == writer->length;
}

}  // namespace ascijson
