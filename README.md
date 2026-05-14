# ascijson

[![License MIT][license-badge]][license-link]
[![C++17][cpp-badge]][cpp-link]
[![PRs Welcome][prs-badge]][prs-link]

---

## Project Docs

- [Inspiration & Future Vision](docs/INSPIRATION.md)
- [Long-Term Project Plan](docs/ASCIJSON_PROJECT_PLAN.md)

ascijson is a lightweight, zero-dependency JSON parsing library written in
C/C++.

It focuses on:
- ASCII string parsing
- Manual tokenization
- Heap-allocated deserialization
- Minimal API for extracting structured data from JSON files

This project is designed as both:
- A practical parsing library
- A learning-focused implementation of tokenization, parsing, and memory
  ownership

## Goals

- Implement a complete JSON tokenizer and parser from scratch
- Build safe, predictable heap-allocated deserialization
- Support fast field lookup for structured JSON data
- Avoid external dependencies
- Keep the API small, explicit, and testable

## Planned Features

- JSON tokenizer (ASCII-based)
- Escape sequence handling
- Object and array parsing
- Heap-allocated string extraction
- Field counting and indexed access
- Simple C API with C++ compatibility
- No dependencies beyond libc / STL (optional)

## Core API (WIP)

```cpp
namespace ascijson {
  // Returns the count of a specific field at the current level.
  unsigned int CountFields(const char* json, const char* field_name);

  // Extracts the string value of the Nth occurrence of a field.
  bool GetNthString(const char* json, const char* field_name, unsigned int n,
                    char* out_buffer, size_t buffer_size);

  // Counts elements in a JSON array.
  unsigned int CountArrayElements(const char* array_json);

  // Returns pointer to the Nth element inside a json array.
  const char* GetNthElement(const char* array_json, unsigned int n);
}
```

---

### Standard Build (Recommended)

This method automatically handles the library compilation and copies
`quotes.json` to your build directory so the demo works out of the box.

**Linux / macOS**
```bash
# Create a build directory
mkdir build && cd build

# Configure and build
cmake ..
cmake --build .

# Run the random quote generator
./quotes_display
```

**Windows (Visual Studio)**
```bat
mkdir build && cd build
cmake ..
cmake --build . --config Release
cd Release && quotes_display.exe
```

**Windows (MinGW / MSYS2)**
```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"
cmake --build .
.\quotes_display.exe
```

### Manual Compilation (Quick Check)

To build the example application using a C++17 compatible compiler (GCC 7+ or
MSVC):

**Linux / macOS**
```bash
g++ -std=c++17 -Wall -Wextra -I./include \
    examples/quotes_display/main.cpp src/tokenizer.cpp \
    -o quotes_display
```

**Windows (MSVC - Developer Command Prompt)**
```bat
cl /std:c++17 /EHsc /I./include ^
   examples\quotes_display\main.cpp src\tokenizer.cpp ^
   /Fe:quotes_display.exe
```

**Windows (MinGW)**
```bash
g++ -std=c++17 -Wall -Wextra -I./include \
    examples/quotes_display/main.cpp src/tokenizer.cpp \
    -o quotes_display.exe
```

> **Note:** Ensure `quotes.json` is in the same directory as the binary,
> or use the CMake build which copies it automatically.
---

### Example Usage

```c
const char* json = "{ \"text\": \"hello\", \"text\": \"world\" }";
unsigned int count = ascijson::CountFields(json, "text");

char buf[64];
if (ascijson::GetNthString(json, "text", 1, buf, sizeof(buf))) {
  printf("%s\n", buf);
}
```

---

### Project Status

Early development
Currently implementing:
- ASCII tokenizer
- String parsing
- Escape handling

## Learning Focus

This project exists to deeply understand:

- Manual memory management
- Tokenization and parsing algorithms
- ASCII string processing
- Serialization / deserialization design
- Building small, composable C APIs

## Project Layout

```text
ascijson/
 ├── include/
 │    └── json.hpp
 ├── src/
 │    ├── json.cpp
 │    ├── tokenizer.cpp
 │    └── parser.cpp
 ├── tests/
 ├── examples/
 └── README.md
```

[license-badge]: https://img.shields.io/badge/License-MIT-blue.svg
[license-link]:  LICENSE
[cpp-badge]:     https://img.shields.io/badge/C%2B%2B-17-blue.svg
[cpp-link]:      https://en.cppreference.com/w/cpp/17
[prs-badge]:     https://img.shields.io/badge/PRs-welcome-brightgreen.svg
[prs-link]:      docs/CONTRIBUTING.md
