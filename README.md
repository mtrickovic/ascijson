# ascijson — Systems-Oriented Zero-STL JSON Toolkit

[![License MIT][license-badge]][license-link]
[![C++17][cpp-badge]][cpp-link]
[![PRs Welcome][prs-badge]][prs-link]

## Table of Contents

- [API Reference](docs/API_REFERENCE.md)
- [The Pivot](#the-pivot-from-modern-c-to-systems-first)
- [Key Features](#key-features)
- [Quick Start](#quick-start)
  - [Installation](#installation)
  - [Basic Usage: The "Find & Extract" Pattern](#basic-usage-the-find--extract-pattern)
  - [Compile and Run](#compile-and-run)
- [Core API](#core-api)
  - [Field Counting](#field-counting)
  - [Value Lookup](#value-lookup)
  - [String Extraction](#string-extraction)
  - [Array Navigation](#array-navigation)
  - [Notes](#notes)
- [Building from Source](#building-from-source)
  - [Prerequisites](#prerequisites)
  - [Build Instructions](#build-instructions)
  - [Without CMake](#without-cmake)
- [Testing](#testing)
  - [Run Tests](#run-tests)
  - [Fuzz Testing](#fuzz-testing)
- [Project Status & Roadmap](#project-status--roadmap)
- [Contributing](#contributing)
- [License](#license)
- [Acknowledgments](#acknowledgments)
- [Support](#support)
- [Project Stats](#project-stats)

## 🚀 The Pivot: From "Modern C++" to "Systems-First"
While the original inspiration was a general-purpose parser, **ascijson** has
evolved into a high-performance **linear-scan tokenizer**. It is specifically
designed for environments where the Standard Template Library (STL) is
forbidden, heap fragmentation is a risk, or binary size is critical.

Unlike traditional JSON libraries that build a heavy Document Object Model (DOM)
tree in memory, **ascijson** uses a **Linear-Scan Architecture**. It treats JSON
as a read-only stream, allowing you to extract specific data points with nearly
zero memory overhead and no heap allocations for the tree structure itself.

### Updated Core Philosophy
- **Zero-STL**: No dependency on `std::string`, `std::vector`, or `std::map`.
- **Linear-Scan Architecture**: Data is extracted by navigating pointers
                                directly on the raw buffer. No heavy DOM tree is
                                built.
- **Memory Predictability**: Operates entirely with fixed-size buffers or
                             stack-based pointers.

---

## Key Features
- **Zero-STL / Zero-Dependency**: Does not use `std::string`, `std::vector`, or
                                  `std::variant`. Compatible with restricted
                                  embedded environments and custom allocators.
- **Linear-Scan Technology**:     Extremely fast data extraction by navigating
                                  pointers directly on the raw buffer.
- **Memory Efficient**:           Ideal for high-performance applications where
                                  heap fragmentation must be avoided.
- **Portable & Rigid**:           Adheres to the Google C++ Style Guide and
                                  passes strict warning checks (`-Werror`,
                                  `-Wall`, `-Wextra`).
- **Nesting Support**:            Advanced pointer arithmetic allows you to
                                  "dive" into nested objects and arrays without
                                  recursive overhead.

---

## Quick Start

### Installation

Building as a static library via CMake ensures optimal link-time optimization
and easy asset management (like copying JSON files to your build directory).

```bash
git clone https://www.github.com/mtrickovic/ascijson.git
mkdir build && cd build
cmake ..
cmake --build .
```

Or add as a Git submodule:

```bash
git submodule add https://github.com/mtrickovic/ascijson.git external/ascijson
```

### Basic Usage: The "Find & Extract" Pattern

Instead of "parsing" the whole file into a heavy object, locate the
value you want and extract it directly into a pre-allocated buffer.

```cpp
#include "include/json.hpp"
#include <iostream>

int main() {
  // Parse JSON from string
  const char* json_ptr = R"({
    "name": "Alice",
    "age": "30",
    "active": true,
    "scores": ["98", "87", "92"]
  })";

  ascijson::Error err;

  // --- Extract a string field ---
  char name[32] = {0};
  if (ascijson::GetNthString(json_ptr, "name", 0, name, sizeof(name), &err)) {
    std::cout << "Name: " << name << std::endl;
  }

  // --- Navigate into an array ---
  const char* scores_array = ascijson::FindValue(json_ptr, "scores", &err);
  if (scores_array) {
    unsigned int count = ascijson::CountArrayElements(scores_array, &err);
    std::cout << "Scores: ";

    for (unsigned int i = 0; i < count; ++i) {
      const char* element = ascijson::GetNthElement(scores_array, i, &err);
      char score_buf[8] = {0};

      // Pass nullptr as field_name to extract the value at this element pointer
      if (ascijson::GetNthString(element,
                                 nullptr,
                                 0,
                                 score_buf,
                                 sizeof(score_buf),
                                 &err)) {
        std::cout << score_buf << " ";
      }
    }
    std::cout << std::endl;
  }
  return EXIT_SUCCESS;
}
```

### Compile and Run

```bash
g++ -std=c++17 -Wall -Wextra -I./include example.cpp src/tokenizer.cpp \
    -o example
./example
```

---

## Core API

All functions accept an optional `Error* out_error` as their last parameter.
Pass `nullptr` to ignore errors, or a pointer to an `ascijson::Error` to
inspect the result. Possible values: `kNone`, `kInvalidJson`, `kFieldNotFound`,
`kMemoryError`.

### Field Counting
```cpp
// Count occurrences of a key at the top level of an object
unsigned int CountFields(const char* json,
                         const char* field_name,
                         Error* out_error = nullptr);
```
### Value Lookup
```cpp
// Returns a pointer to the value of a named key, or nullptr if not found
const char* FindValue(const char* json,
                      const char* key,
                      Error* out_error = nullptr);
```

### String Extraction
```cpp
// Extract the Nth occurrence of a string value by key into a fixed buffer
// Pass nullptr as field_name to extract directly from an element pointer
bool GetNthString(const char* json,
                  const char* field_name,
                  unsigned int n,
                  char* out_buffer,
                  size_t buffer_size,
                  Error* out_error = nullptr);
```

### Array Navigation
```cpp
// Count elements in a JSON array (pointer must point at '[')
unsigned int CountArrayElements(const char* array_json,
                                Error* out_error = nullptr);

// Returns a pointer to the Nth element inside a JSON array
const char* GetNthElement(const char* array_json,
                          unsigned int n,
                          Error* out_error = nullptr);
```

### Int Parsing
```cpp
bool GetNthInt(const char* json,
               const char* field_name,
               unsigned int index,
               int* out_value,
               Error* out_error = nullptr);
```

### Double Parsing
```cpp
bool GetNthDouble(const char* json,
                  const char* field_name,
                  unsigned int index,
                  double* out_value,
                  Error* out_error = nullptr);
```

### `IsTrue` Parsing
```cpp
bool IsTrue(const char* json,
            const char* field_name,
            Error* out_error = nullptr);
```

### `IsFalse` Parsing
```cpp
bool IsFalse(const char* json,
             const char* field_name,
             Error* out_error = nullptr);
```

### `IsNull` Parsing
```cpp
bool IsNull(const char* json,
            const char* field_name,
            Error* out_error = nullptr);
```

###
### Notes
- All functions accept raw `const char*` — no heap allocation for the tree
- Functions return `false` or `nullptr` on failure, no exceptions thrown
- Buffers are caller-allocated; always pass `sizeof(buffer)` as the size
- `FindValue` and `GetNthElement` return interior pointers into the
  original buffer — do not free them
- `IsTrue`, `IsFalse`, and `IsNull` are mutually exclusive for any given field

---

## Building from Source

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.10+ (optional, for building tests and examples)

### Build Instructions

**Linux / macOS**
```bash
git clone https://github.com/mtrickovic/ascijson.git
cd ascijson
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
cd build
./tokenizer_test
./basic_usage && ./quotes_display && ./basic_numerics && ./basic_boolnull
```

**Windows (Visual Studio)**
```bat
mkdir build && cd build
cmake ..
cmake --build . --config Release
cd Release && tokenizer_test.exe
```

**Windows (MinGW / MSYS2)**
```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
.\tokenizer_test.exe
```

### Without CMake

#### Ex: Basic Usage
```bash
g++ -std=c++17 -Wall -Wextra -I./include examples/basic_usage.cpp \
    src/tokenizer.cpp -o basic_usage
./basic_usage
```

**Windows (MSVC)**
```bat
cl /std:c++17 /EHsc /I./include examples\basic_usage.cpp src\tokenizer.cpp /Fe:basic_usage.exe
```

#### Ex: Quotes Display
```bash
g++ -std=c++17 -Wall -Wextra -I./include examples/quotes_display/main.cpp \
    src/tokenizer.cpp -o quotes_display
cp ./examples/quotes_display/quotes.json . && ./quotes_display
```

**Windows (MSVC)**
```bat
cl /std:c++17 /EHsc /I./include examples\quotes_display\main.cpp src\tokenizer.cpp /Fe:quotes_display.exe
copy examples\quotes_display\quotes.json . && quotes_display.exe
```

#### Ex: Basic Numerics
```bash
g++ -std=c++17 -Wall -Wextra -I./include examples/basic_numerics/main.cpp \
    src/tokenizer.cpp -o basic_numerics
cp ./examples/basic_numerics/portfolio.json . && ./basic_numerics
```

**Windows (MSVC)**
```bat
cl /std:c++17 /EHsc /I./include examples\basic_numerics\main.cpp src\tokenizer.cpp /Fe:basic_numerics.exe
copy examples\basic_numerics\portfolio.json . && basic_numerics.exe
```

#### Ex: Basic Bool Null
```bash
g++ -std=c++17 -Wall -Wextra -I./include examples/basic_boolnull/main.cpp \
    src/tokenizer.cpp -o basic_boolnull
cp ./examples/basic_boolnull/flags.json . && ./basic_boolnull
```

**Windows (MSVC)**
```bat
cl /std:c++17 /EHsc /I./include examples\basic_boolnull\main.cpp src\tokenizer.cpp /Fe:basic_boolnull.exe
copy examples\basic_boolnull\flags.json . && basic_boolnull.exe
```

---

## Testing

The project includes a test suite covering:
- Null and safety guard checks for all public API functions
- Field counting with single, multiple, and missing keys
- Nested object and array isolation
- Prefix protection (e.g. `"user"` vs `"username"`)
- String extraction by index and out-of-range handling
- Direct element extraction via `nullptr` field name
- Round-trip: `GetNthElement` + `GetNthString` on object arrays

### Run Tests

```bash
# From project root
mkdir -p build && cd build
cmake ..
cmake --build .
ctest --output-on-failure
```

### Fuzz Testing

A standalone random fuzzer is included in `tests/fuzz.cpp`. It requires no
special compiler support and works with both g++ and MSVC.

**Build with CMake (recommended — enables ASan + UBSan on g++ automatically):**
```bash
cmake -DBUILD_FUZZING=ON ..
cmake --build .
```

**Build without CMake:**
```bash
# g++ (with sanitizers for deeper crash detection)
g++ -std=c++17 -O2 -fsanitize=address,undefined \
    -I./include tests/fuzz.cpp src/tokenizer.cpp -o fuzz
```

```bat
# MSVC
cl /std:c++17 /O2 /I./include tests\fuzz.cpp src\tokenizer.cpp /Fe:fuzz.exe
```

**Run:**
```bash
./fuzz         # random seed
./fuzz 12345   # fixed seed — fully reproducible
```

The seed is printed at startup, so any crash can be reproduced exactly by
re-running with the same seed.

---

## Project Status & Roadmap

### Current Version: 0.2.0 (Early Development)

**Completed:**
- Linear-scan tokenizer with zero-STL, zero-dependency design
- Public API: `CountFields`, `GetNthString`, `FindValue`,
  `CountArrayElements`, `GetNthElement`
- `GetNthString` with `nullptr` field name for direct element extraction
- `GetNthInt` and `GetNthDouble` for numeric extraction
- `IsTrue`, `IsFalse`, `IsNull` for boolean and null field checks
- Error reporting via `Error` enum and `out_error` propagation across all
  public API functions
- Standalone random fuzzer with reproducible seeds (`tests/fuzz.cpp`)
- Test suite covering all public API functions with null safety,
  edge cases, and round-trip tests
- `quotes_display` example — reads JSON file, picks random quote
- CMake build for Linux, macOS, and Windows (MSVC + MinGW)
- Google Style `.clang-format` applied across all files
- Cross-platform MSVC warning fixes (`fopen_s`, `time_t` cast)

**Known Limitations:**
- ~~No number extraction (all values treated as strings)~~
- No escape sequence handling beyond `\"`
- No unicode support
- Buffer overflow risk on values exceeding fixed buffer sizes
- No error reporting — functions return `false` or `nullptr` silently
- Unicode escape sequences (`\uXXXX`) are replaced with `?` — no UTF-8 output

**Not Planned:**
- Full RFC 8259 compliance
- DOM tree / full parse
- STL-style API

---

## Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](docs/CONTRIBUTING.md)
for:

- Code style guidelines
- How to submit pull requests
- Testing requirements
- Issue reporting guidelines

---

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE)
file for details.

```
MIT License

Copyright (c) 2025 Marko Trickovic

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

---

## Acknowledgments

- [RFC 8259](https://tools.ietf.org/html/rfc8259) - The JSON Data Interchange Format
- [nlohmann/json](https://github.com/nlohmann/json) - Inspiration for API design
- [RapidJSON](https://github.com/Tencent/rapidjson) - Inspiration for performance techniques
- [JSON Test Suite](https://github.com/nst/JSONTestSuite) - Comprehensive test cases

---

## Support

- **Bug Reports:** [GitHub Issues](https://github.com/mtrickovic/ascijson/issues)
- **Feature Requests:** [GitHub Discussions](https://github.com/mtrickovic/ascijson/discussions)
- **Email:** marko@trickovic.dev
- **LinkedIn:** [Marko Trickovic](https://linkedin.com/in/markotrickovic)

---

## Project Stats

![GitHub stars](https://img.shields.io/github/stars/mtrickovic/ascijson?style=social)
![GitHub forks](https://img.shields.io/github/forks/mtrickovic/ascijson?style=social)
![GitHub watchers](https://img.shields.io/github/watchers/mtrickovic/ascijson?style=social)

---

<div align="center">

**[API Reference](docs/API_REFERENCE.md)** • **[Examples](examples/)** • **[Contributing](docs/CONTRIBUTING.md)** • **[Changelog](docs/CHANGELOG.md)**

Made with care by [Marko Trickovic](https://github.com/mtrickovic)

</div>

[license-badge]: https://img.shields.io/badge/License-MIT-blue.svg
[license-link]:  LICENSE
[cpp-badge]:     https://img.shields.io/badge/C%2B%2B-17-blue.svg
[cpp-link]:      https://en.cppreference.com/w/cpp/17
[prs-badge]:     https://img.shields.io/badge/PRs-welcome-brightgreen.svg
[prs-link]:      docs/CONTRIBUTING.md
