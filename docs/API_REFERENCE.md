# ascijson — API Reference

> Zero-STL, single-header JSON tokenizer. All functions operate on raw
> `const char*` pointers via linear scan — no heap allocation, no exceptions.

---

## Table of Contents

- [Conventions](#conventions)
- [Lookup & Navigation](#lookup--navigation)
  - [FindValue](#findvalue)
  - [CountFields](#countfields)
  - [CountArrayElements](#countarrayelements)
  - [GetNthElement](#getnthelement)
- [String Extraction](#string-extraction)
  - [GetNthString](#getnthstring)
- [Numeric Extraction](#numeric-extraction)
  - [GetNthInt](#getnthint)
  - [GetNthDouble](#getnthdouble)
- [Boolean & Null Checks](#boolean--null-checks)
  - [IsTrue](#istrue)
  - [IsFalse](#isfalse)
  - [IsNull](#isnull)

---

## Conventions

**Namespace:** `ascijson`

**Input JSON:** All functions accept a `const char*` pointing at valid,
null-terminated UTF-8 JSON text. Passing a pointer into the middle of a
document (e.g. a value returned by `FindValue` or `GetNthElement`) is
explicitly supported and is the intended composition pattern.

**Null safety:** Every function returns a safe default (`false`, `0`,
`nullptr`) when passed a `nullptr` argument.

**Depth:** Functions operate on the **top level only** — they do not
descend into nested objects or arrays unless you explicitly pass a nested
pointer. This is by design; it prevents accidental key shadowing from
child objects.

---

## Lookup & Navigation

### `FindValue`

```cpp
const char* FindValue(const char* json, const char* key);
```

Returns a pointer to the **value** of the first occurrence of `key` in the
top-level object. The pointer lands on the first non-whitespace character
of the value, which may be `"`, `{`, `[`, a digit, `-`, or the first
character of a literal (`t`, `f`, `n`).

Returns `nullptr` if `json` or `key` is `nullptr`, the input is not a
`{…}` object, or the key is not found.

```cpp
const char* doc = R"({"price": 9.99, "tags": ["sale", "new"]})";

const char* price = FindValue(doc, "price");  // points at '9'
const char* tags  = FindValue(doc, "tags");   // points at '['
const char* none  = FindValue(doc, "stock");  // nullptr
```

---

### `CountFields`

```cpp
unsigned int CountFields(const char* json, const char* field_name);
```

Returns the number of times `field_name` appears as a key at the top
level of `json`. Nested objects are not scanned.

```cpp
const char* doc = R"({"id": 1, "id": 2, "meta": {"id": 99}})";

CountFields(doc, "id");    // 2  — nested "id" is ignored
CountFields(doc, "meta");  // 1
CountFields(doc, "gone");  // 0
```

---

### `CountArrayElements`

```cpp
unsigned int CountArrayElements(const char* array_json);
```

Returns the number of elements in the JSON array pointed to by
`array_json`. The pointer must land on the opening `[`.

Returns `0` for `nullptr`, a non-array input, or an empty array.

```cpp
const char* arr = "[1, 2, 3]";
CountArrayElements(arr);  // 3

// Typical usage via FindValue:
const char* items = FindValue(doc, "items");
unsigned int n = CountArrayElements(items);
```

---

### `GetNthElement`

```cpp
const char* GetNthElement(const char* array_json, unsigned int n);
```

Returns a pointer to the **Nth element** (zero-indexed) inside a JSON
array. The pointer lands on the first non-whitespace character of the
element — suitable for passing directly into other `ascijson` functions.

Returns `nullptr` if the array is `nullptr`, not an array, or `n` is
out of range.

```cpp
const char* arr = R"([{"k":"one"},{"k":"two"},{"k":"three"}])";

const char* el   = GetNthElement(arr, 1);  // points at '{"k":"two"}'
char buf[16]     = {};
GetNthString(el, "k", 0, buf, sizeof(buf)); // buf == "two"
```

---

## String Extraction

### `GetNthString`

```cpp
bool GetNthString(const char* json, const char* field_name, unsigned int n,
                  char* out_buffer, size_t buffer_size);
```

Copies the string value of the **Nth occurrence** (zero-indexed) of
`field_name` into `out_buffer`. The output is always null-terminated.

Returns `false` if any argument is `nullptr`/zero, the key is not found,
the Nth occurrence does not exist, or the value is not a JSON string.

```cpp
const char* doc = R"({"tag": "alpha", "tag": "beta"})";
char buf[32]    = {};

GetNthString(doc, "tag", 0, buf, sizeof(buf));  // buf == "alpha"
GetNthString(doc, "tag", 1, buf, sizeof(buf));  // buf == "beta"
GetNthString(doc, "tag", 2, buf, sizeof(buf));  // returns false
```

---

## Numeric Extraction

### `GetNthInt`

```cpp
bool GetNthInt(const char* json, const char* field_name, unsigned int index,
               int* out_value);
```

Finds `field_name` in `json` and writes its integer value into
`*out_value`. If the value is a JSON array, `index` selects the element;
for a scalar value `index` is ignored.

Parsing is manual ASCII-to-integer — no `std::stoi` or `atoi`. Supports
optional leading `-` for negative values. Stops at the first non-digit,
so a value like `3.14` will parse as `3`.

Returns `false` if the field is absent or the value contains no digits.

```cpp
const char* doc = R"({"count": 42, "offset": -7, "ids": [10, 20, 30]})";
int val = 0;

GetNthInt(doc, "count",  0, &val);  // val == 42
GetNthInt(doc, "offset", 0, &val);  // val == -7
GetNthInt(doc, "ids",    2, &val);  // val == 30
```

---

### `GetNthDouble`

```cpp
bool GetNthDouble(const char* json, const char* field_name, unsigned int index,
                  double* out_value);
```

Finds `field_name` in `json` and writes its floating-point value into
`*out_value`. Follows the same array-index semantics as `GetNthInt`.

Parsing is manual ASCII-to-double — no `std::stod` or `strtod`. Handles
an optional leading `-`, an integer part, and an optional fractional part
after `.`. Exponent notation (`e`/`E`) is not currently supported.

Returns `false` if the field is absent or the value contains no digits.

```cpp
const char* doc = R"({"price": 9.99, "ratio": -0.5, "pi": 3.14159})";
double val = 0.0;

GetNthDouble(doc, "price", 0, &val);  // val == 9.99
GetNthDouble(doc, "ratio", 0, &val);  // val == -0.5
GetNthDouble(doc, "pi",    0, &val);  // val == 3.14159
```

---

## Boolean & Null Checks

All three functions use `FindValue` internally and then verify the returned
pointer matches the expected literal **and** that the following character
is a valid value terminator (`,`, `}`, `]`, whitespace, or `\0`). This
prevents prefix false-positives such as `truecolor` matching `true`.

---

### `IsTrue`

```cpp
bool IsTrue(const char* json, const char* field_name);
```

Returns `true` if and only if `field_name` maps to the JSON literal
`true`.

Returns `false` for `false`, `null`, any numeric value, or the string
`"true"` (which is a JSON string, not a boolean).

```cpp
const char* doc = R"({"active": true, "label": "true", "count": 1})";

IsTrue(doc, "active");  // true
IsTrue(doc, "label");   // false — string, not literal
IsTrue(doc, "count");   // false — integer, not literal
```

---

### `IsFalse`

```cpp
bool IsFalse(const char* json, const char* field_name);
```

Returns `true` if and only if `field_name` maps to the JSON literal
`false`.

```cpp
const char* doc = R"({"enabled": false, "active": true, "val": null})";

IsFalse(doc, "enabled");  // true
IsFalse(doc, "active");   // false
IsFalse(doc, "val");      // false — null is not false
```

---

### `IsNull`

```cpp
bool IsNull(const char* json, const char* field_name);
```

Returns `true` if and only if `field_name` maps to the JSON literal
`null`.

A missing key also returns `false` — there is no distinction between
"key absent" and "key present with a non-null value" at this API level.
Use `FindValue` directly if you need to tell them apart.

```cpp
const char* doc = R"({"ref": null, "active": true, "count": 0})";

IsNull(doc, "ref");     // true
IsNull(doc, "active");  // false
IsNull(doc, "count");   // false — 0 is not null
IsNull(doc, "ghost");   // false — missing key
```

#### Distinguishing absent from null

```cpp
const char* val = FindValue(doc, field_name);
if (!val)              { /* key not present */ }
else if (IsNull(doc, field_name)) { /* explicitly null */ }
else                   { /* has a real value */ }
```
