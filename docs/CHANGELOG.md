# Changelog
All notable changes to **ascijson** will be documented in this file.

The format is based on [Keep a Changelog][keepachangelog-link],
and this project adheres to [Semantic Versioning][semver-link].

---

## [Unreleased]

### Planned
- SIMD optimization for whitespace and quote scanning
- Hardware target demo for memory-constrained / embedded environments

---

## [0.3.0] - 2026-07-06

### Added
- **Zero-STL Writer**: New `Writer` struct and free-function API
  (`InitWriter`, `BeginObject`/`EndObject`, `BeginArray`/`EndArray`,
  `WriteKey`, `WriteString`, `WriteInt`, `WriteDouble`, `WriteBool`,
  `WriteNull`) for serializing JSON into a caller-provided, fixed-size
  buffer. Zero heap allocation, zero exceptions, matching the existing
  reader's philosophy.
- `WriteWriterToFile` — writes a completed `Writer`'s buffer to disk in
  binary mode (no CRLF translation), keeping output byte-identical
  across Windows and Linux.
- Locale-safe double formatting: `WriteDouble` always emits `.` as the
  decimal separator regardless of the active C locale, with configurable
  precision and automatic trailing-zero trimming.
- String escaping for control characters, quotes, and backslashes in
  `WriteString`.
- New error codes: `kBufferOverflow` (buffer or max nesting depth
  exceeded), `kInvalidState` (e.g. mismatched `EndObject`/`EndArray`).
- **Test Suite** (`tests/writer_test.cpp`): exact-string assertions for
  objects, arrays, nesting, escaping, double formatting/trimming, buffer
  overflow, invalid state transitions, and file round-trip — using the
  existing `ascijson::test::Assert`/`Summary` framework.
- **Example App** (`examples/partlist.cpp`): builds an in-memory list of
  records and writes them out as a JSON array.

### Known Limitations
- Fixed maximum nesting depth (`kMaxWriterDepth = 32`); deeper structures
  require increasing this compile-time constant.
- Output must fit entirely within the caller-provided buffer before
  writing to disk — no incremental/streaming write to an open `FILE*`.
- No pretty-printing; output is always compact (no indentation).

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

[Unreleased]: https://github.com/mtrickovic/ascijson/compare/v0.3.0...HEAD
[0.3.0]: https://github.com/mtrickovic/ascijson/compare/v0.2.1...v0.3.0
[0.1.0]: https://github.com/mtrickovic/ascijson/releases/tag/v0.1.0

[keepachangelog-link]:     https://keepachangelog.com/en/1.0.0/
[semver-link]:  https://semver.org/spec/v2.0.0.html
