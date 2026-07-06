# ascijson - Systems-Oriented Zero-STL JSON Toolkit

## Project Overview

**ascijson** is a systems-oriented C++ JSON extraction library designed for
environments where the STL is restricted, heap fragmentation is a risk, or
binary size is critical. It uses a **Linear-Scan Architecture** — treating JSON
as a read-only stream and extracting data via raw pointer navigation, with zero
heap allocations for tree structure.

### Goals
- Extract JSON values from strings and files without building a DOM tree
- Zero external dependencies
- Zero STL usage (`std::string`, `std::vector`, `std::map` forbidden)
- Caller-managed, fixed-size buffers only
- Comprehensive error handling via return values (no exceptions)
- Modern C++ (C++17) best practices within Zero-STL constraints

---

## Repository Information

**Repository Name:** `ascijson`
**Language:** C++17
**License:** MIT
**Type:** Static library (`src/tokenizer.cpp` + `src/json_writer.cpp` +
`include/json.hpp`)

---

## Estimated Time to Completion

| Phase | Duration | Cumulative Time |
|-------|----------|-----------------|
| Phase 1: Foundation        | 1-2 weeks | 2 weeks  |
| Phase 2: Core Tokenizer    | 2-3 weeks | 5 weeks  |
| Phase 3: Extraction API    | 2-3 weeks | 8 weeks  |
| Phase 4: Data Types & Hardening | 1-2 weeks | 10 weeks |
| Phase 5: Serialization & Optimization | 1-2 weeks | 12 weeks |
| Phase 6: Documentation     | 1 week    | 13 weeks |

**Total ETA: 11-13 weeks** (working 10-15 hours/week)

---

## Project Directory Structure

```
ascijson/
├── README.md                          # Quick start and installation
├── docs/
│   ├── INSPIRATION.md                 # Project vision and philosophy
│   ├── DESIGN.md                      # Architecture and design decisions
│   ├── CONTRIBUTING.md                # Style guide and contribution process
│   ├── CHANGELOG.md                   # Version history
│   ├── API_REFERENCE.md               # Full function-by-function API docs
│   └── ASCIJSON_PROJECT_PLAN.md       # Milestones, guidelines, motivation
├── CMakeLists.txt                     # Cross-platform build system
│
├── include/                           # Public headers
│   └── json.hpp                       # Public API, Error enum, Writer
│
├── src/                                # Implementation
│   ├── tokenizer.cpp                  # Linear-scan and skipping logic
│   └── json_writer.cpp                # Zero-STL JSON writer
│
├── tests/                             # Test suite
│   ├── test_framework.hpp             # Assert and Summary functions
│   ├── tokenizer_test.cpp             # Reader API and safety tests
│   ├── writer_test.cpp                # Writer API tests
│   └── fuzz.cpp                       # xorshift32-based fuzz tester
│
└── examples/                          # Example programs
    ├── basic_usage.cpp                # Minimal end-to-end usage
    ├── quotes_display/                # File-based random quote demo
    │   ├── main.cpp
    │   └── quotes.json
    ├── basic_numerics/                # GetNthInt / GetNthDouble demo
    │   ├── main.cpp
    │   └── portfolio.json
    ├── basic_boolnull/                # IsTrue / IsFalse / IsNull demo
    │   ├── main.cpp
    │   └── flags.json
    └── partlist.cpp                   # Writer: build a JSON array of records
```

---

## Milestone Roadmap

### 🎯 Milestone 1: Foundation (Completed)

- [x] Initialize Git repository.
- [x] Set up directory structure (`src`, `include`, `tests`, `examples`).
- [x] Configure CMake build system for Linux, macOS, and Windows (MSVC/MinGW).
- [x] Enforce Google C++ Style via `.clang-format`.

---

### 🎯 Milestone 2: Linear-Scan Tokenizer (Completed)

- [x] Implement Zero-STL whitespace skipping (`SkipWhitespace`).
- [x] Implement Zero-STL string comparison and length helpers
      (`StringsAreEqual`, `GetStringLength`).
- [x] Design value-aware cursor jumping — skipping strings, objects, and
      arrays (`SkipValue`).
- [x] Implement linear-scan key matching (`IsMatch`).
- [x] Implement fixed-buffer string copy helper (`CopyString`).
- [x] **Pivot**: Abandoned `std::variant` value hierarchy in favour of raw
      pointer navigation to maintain Zero-STL compliance.

---

### 🎯 Milestone 3: Extraction API (Completed)

- [x] `FindValue`: Return a raw pointer to the value of a named key.
- [x] `CountFields`: Count occurrences of a key at the current object level.
- [x] `CountArrayElements`: Determine array size without full parsing.
- [x] `GetNthElement`: Navigate directly to an array index via pointer.
- [x] `GetNthString`: Extract raw text into caller-provided fixed buffers.
- [x] Support `nullptr` field name in `GetNthString` for direct element
      extraction (used with `GetNthElement` for round-trip access).
- [x] `Error` enum declared in `json.hpp`:
      `kNone`, `kInvalidJson`, `kFieldNotFound`, `kMemoryError`.

---

### 🎯 Milestone 4: Data Types & Hardening (Completed)

- [x] **Numeric Support**: Implement `GetNthInt` and `GetNthDouble` using
      manual ASCII-to-number conversion (no `std::stoi` / `std::stod`).
- [x] **Boolean/Null**: Add `IsTrue`, `IsFalse`, `IsNull` checks for
      `true`, `false`, and `null` literals.
- [x] **Error Propagation**: Wire the existing `Error` enum into public
      functions so callers can distinguish "not found" from "invalid JSON"
      (optional trailing `Error* out_error = nullptr` on every function).
- [x] **Fuzz Testing**: xorshift32-based fuzz tester (`tests/fuzz.cpp`),
      optional `BUILD_FUZZING` CMake target with ASan/UBSan on GCC/Clang.

---

### 🎯 Milestone 5: Serialization & Optimization (In Progress)

- [x] **Zero-STL Serializer**: `Writer` struct + free-function API
      (`InitWriter`, `BeginObject`/`EndObject`, `BeginArray`/`EndArray`,
      `WriteKey`, `WriteString`, `WriteInt`, `WriteDouble`, `WriteBool`,
      `WriteNull`) generating JSON directly into a caller-provided,
      fixed-size buffer. Includes `WriteWriterToFile` for cross-platform
      (binary-mode) file output.
- [ ] **SIMD Optimization**: Explore using SIMD instructions for ultra-fast
      whitespace and quote scanning.
- [ ] **Hardware Target Demo**: An example specifically designed to run on a
      simulated memory-constrained environment or embedded board.

---

## Public API Reference

Declared in `include/json.hpp`, implemented in `src/tokenizer.cpp` (reader)
and `src/json_writer.cpp` (writer).

```cpp
namespace ascijson {

// Error codes. kBufferOverflow and kInvalidState are Writer-specific;
// the reader functions use the first three.
enum class Error {
  kNone = 0,
  kInvalidJson,
  kFieldNotFound,
  kMemoryError,
  kBufferOverflow,
  kInvalidState
};

// --- Reader: every function below also takes an optional trailing
// Error* out_error = nullptr, omitted here for brevity. ---

// Count occurrences of a key at the top level of an object.
unsigned int CountFields(const char* json, const char* field_name);

// Return a raw pointer to the value of a named key, or nullptr.
const char* FindValue(const char* json, const char* key);

// Extract the Nth occurrence of a string value by key into a fixed buffer.
// Pass nullptr as field_name to extract directly from an element pointer.
bool GetNthString(const char* json, const char* field_name, unsigned int n,
                  char* out_buffer, size_t buffer_size);

// Count elements in a JSON array (pointer must point at '[').
unsigned int CountArrayElements(const char* array_json);

// Return a pointer to the Nth element inside a JSON array.
const char* GetNthElement(const char* array_json, unsigned int n);

// Extract an integer / double value from a field or array index.
bool GetNthInt(const char* json, const char* field_name, unsigned int index,
               int* out_value);
bool GetNthDouble(const char* json, const char* field_name, unsigned int index,
                  double* out_value);

// True iff the named field's value is the literal true / false / null.
bool IsTrue(const char* json, const char* field_name);
bool IsFalse(const char* json, const char* field_name);
bool IsNull(const char* json, const char* field_name);

// --- Writer: a stateful builder threaded through one Writer* handle,
// rather than independent stateless calls like the reader above. ---

void InitWriter(Writer* writer, char* buffer, size_t capacity);
bool BeginObject(Writer* writer);
bool EndObject(Writer* writer);
bool BeginArray(Writer* writer);
bool EndArray(Writer* writer);
bool WriteKey(Writer* writer, const char* key);
bool WriteString(Writer* writer, const char* value);
bool WriteInt(Writer* writer, int value);
bool WriteDouble(Writer* writer, double value, int precision = 6);
bool WriteBool(Writer* writer, bool value);
bool WriteNull(Writer* writer);
const char* WriterCStr(Writer* writer);
bool WriterIsValid(const Writer* writer);
bool WriteWriterToFile(const Writer* writer, const char* path);

}  // namespace ascijson
```

See `docs/API_REFERENCE.md` for full signatures, descriptions, and examples
for every function above.

**Usage notes:**
- Reader functions accept raw `const char*` — no heap allocation for tree
  structure. Writer functions serialize into a caller-provided fixed buffer
  — same zero-allocation guarantee, opposite direction.
- Functions return `false` or `nullptr` on failure; no exceptions are thrown.
- Buffers are caller-allocated; always pass `sizeof(buffer)` as the size.
- Pointers returned by `FindValue` and `GetNthElement` are interior pointers
  into the original buffer — do not free them.
- `Writer` tracks its own error state (`Writer::last_error`, inspectable via
  `WriterIsValid`) rather than taking a per-call `out_error`, since Writer
  calls are chained sequentially through one handle.

---

## Testing Strategy

**ascijson** uses a custom minimal test framework (`tests/test_framework.hpp`)
in the `ascijson::test` namespace. This keeps the test infrastructure as
portable and dependency-free as the library itself.

### test_framework.hpp — Actual API

```cpp
namespace ascijson {
namespace test {
  static int g_pass_count = 0;
  static int g_fail_count = 0;

  inline void Assert(const char* name, bool condition);
  inline void Summary();  // prints Passed/Failed counts
}
}
```

### Example Test

```cpp
#include "../include/json.hpp"
#include "test_framework.hpp"
#include <cstring>

using namespace ascijson;
using namespace ascijson::test;

void TestStringExtraction() {
  const char* json = R"({"name": "ascijson"})";
  char buffer[32] = {0};

  Assert("GetNthString: found",
         GetNthString(json, "name", 0, buffer, sizeof(buffer)));
  Assert("GetNthString: content", std::strcmp(buffer, "ascijson") == 0);
}

int main() {
  TestStringExtraction();
  Summary();
  return (g_fail_count == 0) ? 0 : 1;
}
```

### Current Test Coverage

**`tests/tokenizer_test.cpp`** (reader):
- **`CountFields`** — null safety, multiple matches, prefix
  isolation, nested isolation
- **`GetNthString`** — null safety, Nth occurrence, out-of-range,
  escaped quotes, non-string values
- **`FindValue`** — null safety, string/number/array values,
  pointer verification
- **`CountArrayElements`** — null, empty, numbers, strings,
  nested objects, via `FindValue`
- **`GetNthElement`** — null, out-of-range, pointer verification,
  round-trip with `GetNthString`

**`tests/writer_test.cpp`** (writer):
- Object and array construction, including nesting and empty containers
- String escaping (quotes, backslashes, control characters)
- Double formatting: trailing-zero trimming, precision, locale safety
- Buffer overflow handling
- Invalid state transitions (mismatched `Begin`/`End`, `WriteKey` outside
  an object)
- File round-trip via `WriteWriterToFile`

**`tests/fuzz.cpp`**: xorshift32-based fuzz tester exercising the reader
against a corpus of malformed/randomized JSON inputs. Built via the
`BUILD_FUZZING` CMake option, with ASan/UBSan enabled on GCC/Clang.

---

## Helpful Libraries & References

### Similar Libraries to Study (Don't Copy, Learn From!)

1. **nlohmann/json** — Most popular, feature-rich
2. **RapidJSON** — High performance
3. **json-c** — C library, simpler design
4. **simdjson** — Cutting-edge SIMD-based performance

### Learning Resources

- **JSON Specification:** [RFC 8259](https://tools.ietf.org/html/rfc8259)
- **C++ Reference:** cppreference.com
- **JSON Test Suite:** [JSONTestSuite](https://github.com/nst/JSONTestSuite)
- **Compiler Explorer:** godbolt.org

### Tools

- **Compiler:** GCC 7+, Clang 5+, or MSVC 2017+
- **Debugger:** GDB, LLDB, or Visual Studio debugger
- **Memory Checker:** Valgrind (Linux/macOS)
- **Sanitizers:** AddressSanitizer, UndefinedBehaviorSanitizer
- **Profiler:** perf, gprof, or Instruments

---

## Production-Ready Checklist

- [ ] Parses all valid JSON correctly
- [ ] Rejects all invalid JSON with clear errors
- [ ] No memory leaks (verified with Valgrind)
- [ ] No undefined behaviour (verified with sanitizers)
- [x] `Error` enum wired into all public functions
- [x] Numeric extraction (`GetNthInt`, `GetNthDouble`) implemented
- [ ] Good performance (benchmarked against alternatives)
- [ ] Comprehensive test coverage (>90%)
- [ ] Clear, complete documentation
- [x] Example programs demonstrating usage
- [x] Semantic versioning applied
- [ ] License clearly specified

---

## Next Steps After This Project

Once you complete ascijson, you'll have gained:

- ✅ Deep understanding of parsing and language processing
- ✅ Systems-level C++ (pointer arithmetic, fixed buffers, Zero-STL)
- ✅ Testing and quality assurance practices
- ✅ Git workflow proficiency
- ✅ Documentation skills
- ✅ Portfolio piece demonstrating end-to-end project completion

**Future Project Ideas:**
- HTTP client library
- Command-line argument parser
- Expression evaluator / calculator
- Configuration file parser (YAML/TOML)
- Mini scripting language interpreter
