# Changelog
All notable changes to **ascijson** will be documented in this file.

The format is based on [Keep a Changelog][keepachangelog-link],
and this project adheres to [Semantic Versioning][semver-link].

---

## [Unreleased]

### Planned
- `GetNthInt` and `GetNthDouble` — numeric extraction via manual conversion
  (no `std::stoi`)
- Boolean and null literal detection (`true`, `false`, `null`)
- Fuzz testing against malformed JSON inputs
- Zero-STL serializer — generate JSON directly into pre-allocated raw buffers
- SIMD optimization for whitespace and quote scanning
- Hardware target demo for memory-constrained / embedded environments

---

## [0.1.0] - 2026-05-14

### Added
- **Linear-Scan Engine**: Tokenizer that navigates raw buffers using pointer
  arithmetic instead of building a DOM tree — zero heap allocations for tree
  structure.
- **Zero-STL Public API**:
  - `FindValue` — locate a value pointer by key name
  - `CountFields` — count key occurrences at the top level of an object
  - `CountArrayElements` — determine array size without full parsing
  - `GetNthElement` — navigate directly to an array index via pointer
  - `GetNthString` — extract raw text into caller-provided fixed buffers;
    supports `nullptr` field name for direct element extraction
- **Architecture Pivot**: Abandoned `std::variant` value hierarchy in favour
  of raw pointer navigation to maintain Zero-STL compliance.
- **Cross-Platform CMake Build**: Supports Linux, macOS, and Windows
  (MSVC/MinGW) with `-Werror -Wall -Wextra` enforced.
- **Google C++ Style**: `.clang-format` applied across all source files.
- **Test Suite** (`tests/tokenizer_test.cpp`):
  - Null and safety guard checks for all public API functions
  - Field counting with single, multiple, and missing keys
  - Nested object and array isolation
  - Prefix protection (e.g. `"user"` vs `"username"`)
  - String extraction by index and out-of-range handling
  - Round-trip: `GetNthElement` + `GetNthString` on object arrays
- **Example App** (`examples/quotes_display`): File-based random quote demo
  showcasing JSON extraction from disk.

### Fixed
- Resolved MSVC warnings for `fopen` (`fopen_s`) and `time_t` casts.
- Standardized namespace and project naming to `ascijson`.

### Known Limitations
- No number extraction — all values currently treated as strings
- No escape sequence handling beyond `\"`
- No unicode support
- No error reporting — functions return `false` or `nullptr` silently
- Buffer overflow risk on values exceeding fixed buffer sizes

### Not Planned
- Full RFC 8259 compliance
- DOM tree / full parse
- STL-style API

---

[Unreleased]: https://github.com/mtrickovic/ascijson/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/mtrickovic/ascijson/releases/tag/v0.1.0

[keepachangelog-link]:     https://keepachangelog.com/en/1.0.0/
[semver-link]:  https://semver.org/spec/v2.0.0.html
