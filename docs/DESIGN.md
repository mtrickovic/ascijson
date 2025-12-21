# Design Architecture

## Linear-Scan Tokenization
Unlike traditional parsers that build a full Document Object Model (DOM), tree,
ascijson uses a linear-scan approach. This allows for:
- Minimal memory overhead.
- Faster extraction of specific fields in large files.
- Predictable performance for embedded systems.

## Memory Management
To maintain zero dependencies and a small binary footprint:
- We use `new` and `delete` for dynamic allocations.
- No STL containers (std::vector, std::map) are used.
- All heap-allocated strings must be freed by the caller.

## Type System
JSON values are represented using a lightweight tagged union or variant
structure to ensure type safety without the overhead of heavy class
hierarchies.
