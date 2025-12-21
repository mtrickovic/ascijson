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

```c
unsigned int get_rand_num(unsigned int max_num);

unsigned int json_count_fields(const char* json, const char* field_name);

char* json_get_nth_quote(
    const char* json,
    unsigned int quote_index,
    const char* field_name
);
```

---

### Build (Simple Example)

To build the example application using a C++17 compatible compiler (GCC 7+ or Clang 5+):

```bash
g++ -std=c++17 -Wall -Wextra -g main.cpp src/json.cpp src/tokenizer.cpp -I./include -o ascijson_app
```
---

### Example Usage

```c
const char* json = "{ \"text\": \"hello\", \"text\": \"world\" }";

unsigned int count = json_count_fields(json, "text");

char* second = json_get_nth_quote(json, 1, "text");
printf("%s\n", second);
free(second);
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
