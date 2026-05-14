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
**Type:** Static library (`src/tokenizer.cpp` + `include/json.hpp`)

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
│   └── ASCIJSON_PROJECT_PLAN.md       # Milestones, guidelines, motivation
├── CMakeLists.txt                     # Cross-platform build system
│
├── include/                           # Public headers
│   └── json.hpp                       # Public API and Error enum
│
├── src/                               # Implementation
│   └── tokenizer.cpp                  # Linear-scan and skipping logic
│
├── tests/                             # Test suite
│   ├── test_framework.hpp             # Assert and Summary functions
│   └── tokenizer_test.cpp             # API and safety tests
│
└── examples/                          # Example programs
    └── quotes_display/                # File-based random quote demo
        ├── main.cpp
        └── quotes.json
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

### 🎯 Milestone 4: Data Types & Hardening (In Progress)

- [ ] **Numeric Support**: Implement `GetNthInt` and `GetNthDouble` using
      manual ASCII-to-number conversion (no `std::stoi` / `std::stod`).
- [ ] **Boolean/Null**: Add `IsTrue`, `IsFalse`, `IsNull` checks for
      `true`, `false`, and `null` literals.
- [ ] **Error Propagation**: Wire the existing `Error` enum into public
      functions so callers can distinguish "not found" from "invalid JSON".
- [ ] **Fuzz Testing**: Run against a corpus of malformed JSON inputs to
      verify no crashes or undefined behaviour.

---

### 🎯 Milestone 5: Serialization & Optimization (Planned)

- [ ] **Zero-STL Serializer**: Generate JSON directly into pre-allocated raw
      buffers from C++ primitives and structs.
- [ ] **SIMD Optimization**: Explore using SIMD instructions for ultra-fast
      whitespace and quote scanning.
- [ ] **Hardware Target Demo**: An example specifically designed to run on a
      simulated memory-constrained environment or embedded board.

---

## Public API Reference

Declared in `include/json.hpp`, implemented in `src/tokenizer.cpp`.

```cpp
namespace ascijson {

// Error codes (not yet wired into all functions — Milestone 4)
enum class Error { kNone = 0, kInvalidJson, kFieldNotFound, kMemoryError };

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

}  // namespace ascijson
```

**Usage notes:**
- All functions accept raw `const char*` — no heap allocation for tree
  structure.
- Functions return `false` or `nullptr` on failure; no exceptions are thrown.
- Buffers are caller-allocated; always pass `sizeof(buffer)` as the size.
- Pointers returned by `FindValue` and `GetNthElement` are interior pointers
  into the original buffer — do not free them.

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

### Current Test Coverage (`tests/tokenizer_test.cpp`)

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
- [ ] `Error` enum wired into all public functions
- [ ] Numeric extraction (`GetNthInt`, `GetNthDouble`) implemented
- [ ] Good performance (benchmarked against alternatives)
- [ ] Comprehensive test coverage (>90%)
- [ ] Clear, complete documentation
- [ ] Example programs demonstrating usage
- [ ] Semantic versioning applied
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
