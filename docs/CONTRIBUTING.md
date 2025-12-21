# Contributing to ascijson

We welcome contributions! To keep the codebase maintainable, we follow the
[Google C++ Style Guide](https://google.github.io/styleguide/cppguide.html).

## Code Style Highlights

- **Line Length:** The maximum line length is 80 characters.
- **Indentation:** Use 2 spaces only. No tabs.
- **Naming:**
    - Type names (Classes, Structs): `CamelCase` (e.g., `JsonParser`).
    - Variables: `snake_case` (e.g., `json_buffer`).
    - Constants: `kCamelCase` (e.g., `kMaxTokenLength`).
    - Functions: `CamelCase` (Google standard) or `snake_case` (if following
      existing C-style API).
- **Files:** Every `.h` file should have a `#define` guard to prevent
  multiple inclusion. The format is `<PROJECT>_<PATH>_<FILE>_H_`.

## Memory Management

- Use `new` and `delete` for dynamic allocations.
- Avoid `malloc` and `free` to stay consistent with C++ patterns.
- Ensure every allocation has a clear ownership model to prevent leaks.

## Submission Process

1.  **Format:** Run a linter or `clang-format` with Google style before
    committing.
2.  **Tests:** All new features must include unit tests in the `tests/`
    directory.
3.  **PRs:** Provide a clear description of the problem your change solves.
