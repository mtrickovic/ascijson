# ascijson

> A modern, header-only C++ JSON parser library designed for correctness,
simplicity, and ease of use.

[![License MIT][license-badge]][license-link]
[![C++17][cpp-badge]][cpp-link]
[![PRs Welcome][prs-badge]][prs-link]

---

## Features

- **RFC 8259 Compliant** - Fully standards-compliant JSON parsing
- **Header-Only** - Just include and use, no linking required
- **Modern C++17** - RAII, move semantics, and type safety
- **Zero Dependencies** - No external libraries needed
- **Intuitive API** - STL-like interface that feels natural
- **Production Ready** - Comprehensive error handling and testing
- **Pretty Printing** - Beautiful, customizable JSON output

---

## Quick Start

### Installation

Simply copy the `include/ascijson/` directory to your project:

```bash
git clone https://github.com/yourusername/ascijson.git
cp -r ascijson/include/ascijson /path/to/your/project/include/
```

Or add as a Git submodule:

```bash
git submodule add https://github.com/yourusername/ascijson.git external/ascijson
```

### Basic Usage

```cpp
#include <ascijson/json.hpp>
#include <iostream>

int main() {
    // Parse JSON from string
    auto data = ascijson::parse(R"({
        "name": "Alice",
        "age": 30,
        "active": true,
        "scores": [98, 87, 92]
    })");
    
    // Access values
    std::string name = data["name"].as_string();
    int age = data["age"].as_int();
    bool active = data["active"].as_bool();
    
    // Iterate over arrays
    for (const auto& score : data["scores"].as_array()) {
        std::cout << score.as_int() << " ";
    }
    
    // Modify and create new values
    data["age"] = 31;
    data["tags"] = ascijson::array{"developer", "learner"};
    
    // Serialize back to JSON
    std::string json_str = data.stringify();
    std::cout << json_str << std::endl;
    
    return 0;
}
```

### Compile and Run

```bash
g++ -std=c++17 -I./include example.cpp -o example
./example
```

---

## Documentation

### Core API

#### Parsing

```cpp
// Parse from string
auto value = ascijson::parse(R"({"key": "value"})");

// Parse from file
auto value = ascijson::parse_file("data.json");

// Parse from stream
std::ifstream file("data.json");
auto value = ascijson::parse(file);
```

#### Value Types

```cpp
// Check types
if (value.is_string()) { /* ... */ }
if (value.is_number()) { /* ... */ }
if (value.is_bool()) { /* ... */ }
if (value.is_null()) { /* ... */ }
if (value.is_array()) { /* ... */ }
if (value.is_object()) { /* ... */ }

// Convert to C++ types
std::string s = value.as_string();
int i = value.as_int();
double d = value.as_double();
bool b = value.as_bool();
```

#### Creating Values

```cpp
using namespace ascijson;

// Primitives
value null_val = nullptr;
value bool_val = true;
value num_val = 42;
value str_val = "hello";

// Arrays
value arr = array{1, 2, 3, 4, 5};
arr.push_back(6);
arr[0] = 10;

// Objects
value obj = object{
    {"name", "Bob"},
    {"age", 25},
    {"active", true}
};
obj["city"] = "New York";
```

#### Serialization

```cpp
// Compact output
std::string compact = value.stringify();

// Pretty-printed output
std::string pretty = value.stringify(true);
std::string custom = value.stringify(true, 4);  // 4-space indent

// Write to file
value.write_file("output.json", true);  // pretty-print enabled
```

#### Error Handling

```cpp
try {
    auto value = ascijson::parse(R"({"invalid": json})");
} catch (const ascijson::parse_error& e) {
    std::cerr << "Parse error at line " << e.line() 
              << ", column " << e.column() << ": "
              << e.what() << std::endl;
}

try {
    int x = value["nonexistent"].as_int();
} catch (const ascijson::type_error& e) {
    std::cerr << "Type error: " << e.what() << std::endl;
}
```

---

## Building from Source

### Prerequisites

- C++17 compatible compiler (GCC 7+, Clang 5+, MSVC 2017+)
- CMake 3.10+ (optional, for building tests and examples)

### Build Instructions

```bash
# Clone the repository
git clone https://github.com/yourusername/ascijson.git
cd ascijson

# Build with CMake (optional)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build

# Run tests
./build/tests/test_runner

# Run examples
./build/examples/basic_usage
```

### Without CMake

```bash
# The library is header-only, so you can just include it
g++ -std=c++17 -I./include examples/basic_usage.cpp -o example
./example
```

---

## Testing

The project includes a comprehensive test suite covering:

- All JSON value types (null, bool, number, string, array, object)
- Parser correctness with valid JSON inputs
- Error detection with invalid JSON inputs
- Edge cases (deep nesting, large numbers, unicode, escape sequences)
- Round-trip testing (parse -> modify -> stringify -> parse)
- Performance benchmarks

Run tests:

```bash
cd tests
./run_tests.sh
```

---

## Design Decisions

### Why Header-Only?

- **Easy integration** - No build system complications
- **Compiler optimization** - Full visibility for inlining
- **Template-friendly** - Natural fit for modern C++

### Why std::variant?

- **Type safety** - Compile-time guarantees
- **Memory efficiency** - No heap allocations for primitives
- **Modern C++** - Idiomatic and expressive

### Why RFC 8259?

- **Standard compliance** - Works with all JSON producers/consumers
- **Clear specification** - Unambiguous implementation guide
- **Wide adoption** - Maximum compatibility

See [docs/DESIGN.md](DESIGN.md) for detailed architecture documentation.

---

## Examples

### Configuration File Parser

```cpp
#include <ascijson/json.hpp>

struct Config {
    std::string server_address;
    int port;
    bool debug_mode;
    
    static Config from_file(const std::string& path) {
        auto json = ascijson::parse_file(path);
        return Config{
            json["server"]["address"].as_string(),
            json["server"]["port"].as_int(),
            json["debug"].as_bool()
        };
    }
};

int main() {
    Config config = Config::from_file("config.json");
    std::cout << "Server: " << config.server_address 
              << ":" << config.port << std::endl;
}
```

### REST API Response Handling

```cpp
#include <ascijson/json.hpp>

void process_api_response(const std::string& response) {
    auto json = ascijson::parse(response);
    
    if (json["status"].as_string() == "success") {
        for (const auto& user : json["data"]["users"].as_array()) {
            std::cout << "User: " << user["name"].as_string() 
                      << " (" << user["id"].as_int() << ")\n";
        }
    } else {
        std::cerr << "Error: " << json["message"].as_string() << std::endl;
    }
}
```

### Building JSON Programmatically

```cpp
#include <ascijson/json.hpp>

ascijson::value create_user_profile(const std::string& name, int age) {
    using namespace ascijson;
    
    return object{
        {"name", name},
        {"age", age},
        {"created_at", "2025-01-01T00:00:00Z"},
        {"preferences", object{
            {"theme", "dark"},
            {"notifications", true}
        }},
        {"tags", array{"developer", "early-adopter"}}
    };
}

int main() {
    auto user = create_user_profile("Alice", 28);
    std::cout << user.stringify(true) << std::endl;
}
```

See [examples/](examples/) directory for more complete examples.

---

## Project Status & Roadmap

### Current Version: 1.0.0 (Stable)

**Completed Features:**
- Full JSON parsing (RFC 8259 compliant)
- JSON serialization with pretty-printing
- Comprehensive error handling with line/column info
- STL-like API with iterators
- UTF-8 string support
- Zero external dependencies
- Extensive test suite (>90% coverage)

**Future Enhancements (v1.x):**
- JSON Pointer (RFC 6901) support
- JSON Patch (RFC 6902) operations  
- Schema validation hooks
- Custom allocator support
- Streaming parser for large files

**Not Planned (use specialized libraries):**
- JSON5 / JSONC (comments, trailing commas)
- Binary formats (BSON, MessagePack)
- Schema validation (use external validator)

---

## Contributing

Contributions are welcome! Please read [CONTRIBUTING.md](CONTRIBUTING.md)
for:

- Code style guidelines
- How to submit pull requests
- Testing requirements
- Issue reporting guidelines

### Development Setup

```bash
git clone https://github.com/yourusername/ascijson.git
cd ascijson
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
```

---

## License

This project is licensed under the MIT License - see the [LICENSE](../LICENSE)
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

- **Bug Reports:** [GitHub Issues](https://github.com/yourusername/ascijson/issues)
- **Feature Requests:** [GitHub Discussions](https://github.com/yourusername/ascijson/discussions)
- **Email:** your.email@example.com
- **LinkedIn:** [Your Profile](https://linkedin.com/in/yourprofile)

---

## Project Stats

![GitHub stars](https://img.shields.io/github/stars/yourusername/ascijson?style=social)
![GitHub forks](https://img.shields.io/github/forks/yourusername/ascijson?style=social)
![GitHub watchers](https://img.shields.io/github/watchers/yourusername/ascijson?style=social)

---

<div align="center">

**[Documentation](docs/)** • **[Examples](examples/)** • **[Contributing](docs/CONTRIBUTING.md)** • **[Changelog](CHANGELOG.md)**

Made with care by [Marko Trickovic](https://github.com/yourusername)

</div>

[license-badge]: https://img.shields.io/badge/License-MIT-blue.svg
[license-link]:  LICENSE
[cpp-badge]:     https://img.shields.io/badge/C%2B%2B-17-blue.svg
[cpp-link]:      https://en.cppreference.com/w/cpp/17
[prs-badge]:     https://img.shields.io/badge/PRs-welcome-brightgreen.svg
[prs-link]:      docs/CONTRIBUTING.md
