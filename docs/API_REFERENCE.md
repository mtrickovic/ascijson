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
- [Writer](#writer)
  - [InitWriter](#initwriter)
  - [BeginObject / EndObject](#beginobject--endobject)
  - [BeginArray / EndArray](#beginarray--endarray)
  - [WriteKey](#writekey)
  - [WriteString](#writestring)
  - [WriteInt](#writeint)
  - [WriteDouble](#writedouble)
  - [WriteBool](#writebool)
  - [WriteNull](#writenull)
  - [WriterCStr](#writercstr)
  - [WriterIsValid](#writerisvalid)
  - [WriteWriterToFile](#writewritertofile)

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

**Error reporting:** Every reader function accepts an optional trailing
`Error* out_error = nullptr`. Pass a non-null pointer to receive more
detail on failure (e.g. distinguishing "key not found" from "malformed
JSON"); pass `nullptr` (the default) if you only care about the
`bool`/`nullptr` return value. The `Writer` API instead stores its error
state internally (`Writer::last_error`, inspectable via `WriterIsValid`),
since Writer calls are chained sequentially through one handle rather
than being independent per-call — see the [Writer](#writer) section.

**Error codes:**

```cpp
enum class Error {
  kNone = 0,
  kInvalidJson,
  kFieldNotFound,
  kMemoryError,
  kBufferOverflow,  // Writer: buffer full or max nesting depth exceeded
  kInvalidState     // Writer: e.g. EndObject() without a matching BeginObject()
};
```

`kBufferOverflow` and `kInvalidState` are Writer-specific; the reader
functions use the first three.

---

## Lookup & Navigation

### `FindValue`

```cpp
const char* FindValue(const char* json, const char* key,
                      Error* out_error = nullptr);
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
unsigned int CountFields(const char* json, const char* field_name,
                         Error* out_error = nullptr);
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
unsigned int CountArrayElements(const char* array_json,
                                Error* out_error = nullptr);
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
const char* GetNthElement(const char* array_json, unsigned int n,
                          Error* out_error = nullptr);
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
                  char* out_buffer, size_t buffer_size,
                  Error* out_error = nullptr);
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
               int* out_value, Error* out_error = nullptr);
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
                  double* out_value, Error* out_error = nullptr);
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
bool IsTrue(const char* json, const char* field_name,
            Error* out_error = nullptr);
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
bool IsFalse(const char* json, const char* field_name,
             Error* out_error = nullptr);
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
bool IsNull(const char* json, const char* field_name,
            Error* out_error = nullptr);
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

---

## Writer

> Unlike the reader functions above (stateless, one call per query), the
> Writer is a small stateful builder: you call `InitWriter` once, then
> make a sequence of `Begin*`/`Write*`/`End*` calls through the same
> `Writer*` handle to assemble a JSON document into a caller-provided
> buffer. See [Error reporting](#conventions) above for why Writer uses
> `Writer::last_error` instead of a per-call `out_error` parameter.

### `InitWriter`

```cpp
void InitWriter(Writer* writer, char* buffer, size_t capacity);
```

Initializes `writer` to serialize into `buffer` (of `capacity` bytes).
`buffer` must outlive the `Writer` and remain valid for every subsequent
call — `Writer` never allocates or frees memory itself.

```cpp
char buf[256];
Writer w;
InitWriter(&w, buf, sizeof(buf));
```

---

### `BeginObject` / `EndObject`

```cpp
bool BeginObject(Writer* writer);
bool EndObject(Writer* writer);
```

Writes `{` / `}`. `EndObject` returns `false` if the writer isn't
currently inside an object (e.g. called without a matching
`BeginObject`, or while inside an array instead).

```cpp
BeginObject(&w);
WriteKey(&w, "id");
WriteInt(&w, 1);
EndObject(&w);
// {"id":1}
```

---

### `BeginArray` / `EndArray`

```cpp
bool BeginArray(Writer* writer);
bool EndArray(Writer* writer);
```

Writes `[` / `]`. Commas between elements are inserted automatically.
`EndArray` returns `false` if the writer isn't currently inside an array.

```cpp
BeginArray(&w);
WriteInt(&w, 1);
WriteInt(&w, 2);
EndArray(&w);
// [1,2]
```

---

### `WriteKey`

```cpp
bool WriteKey(Writer* writer, const char* key);
```

Writes an object field key (quoted, escaped, followed by `:`). Must be
called while directly inside an object, immediately before the
corresponding value call. Returns `false` if called outside an object
(e.g. directly inside an array, or at the top level).

```cpp
BeginObject(&w);
WriteKey(&w, "name");
WriteString(&w, "example");
EndObject(&w);
// {"name":"example"}
```

---

### `WriteString`

```cpp
bool WriteString(Writer* writer, const char* value);
```

Writes a JSON string value. Quotes, backslashes, and control characters
(`\n`, `\t`, `\r`, `\b`, `\f`, and other bytes below `0x20`) are escaped
automatically.

```cpp
WriteString(&w, "line one\nline two");
// "line one\nline two"   (escaped in the output)
```

---

### `WriteInt`

```cpp
bool WriteInt(Writer* writer, int value);
```

Writes a JSON integer value.

```cpp
WriteInt(&w, -7);
// -7
```

---

### `WriteDouble`

```cpp
bool WriteDouble(Writer* writer, double value, int precision = 6);
```

Writes a JSON double with up to `precision` digits after the decimal
point; trailing zeros are trimmed (`65.0` → `65`, `65.50` → `65.5`).
Always uses `.` as the decimal separator regardless of the active C
locale — some locales use `,`, which would otherwise produce invalid
JSON.

```cpp
WriteDouble(&w, 65.5, 2);  // 65.5
WriteDouble(&w, 65.0);     // 65
```

---

### `WriteBool`

```cpp
bool WriteBool(Writer* writer, bool value);
```

Writes the JSON literal `true` or `false`.

---

### `WriteNull`

```cpp
bool WriteNull(Writer* writer);
```

Writes the JSON literal `null`.

---

### `WriterCStr`

```cpp
const char* WriterCStr(Writer* writer);
```

Returns a null-terminated pointer to the JSON text written so far, or
`nullptr` if the writer is in an error state. Does not require the
document to be "complete" (all containers closed) — check
`WriterIsValid` separately if you need that guarantee.

```cpp
BeginObject(&w);
WriteKey(&w, "ok");
WriteBool(&w, true);
EndObject(&w);

const char* json = WriterCStr(&w);  // {"ok":true}
```

---

### `WriterIsValid`

```cpp
bool WriterIsValid(const Writer* writer);
```

Returns `true` if no error has occurred **and** every opened
object/array has been closed (`depth == 0`). Check this before trusting
`WriterCStr`/`WriteWriterToFile` for a complete document — a writer can
have `last_error == kNone` while still having unclosed containers.

---

### `WriteWriterToFile`

```cpp
bool WriteWriterToFile(const Writer* writer, const char* path);
```

Writes the JSON text held by `writer` to a file at `path`, in binary
mode (no CRLF translation — output is byte-identical across Windows and
Linux for the same input). Returns `false` if `writer` is not
`WriterIsValid`, or if the file could not be opened or fully written.

```cpp
if (WriterIsValid(&w)) {
  WriteWriterToFile(&w, "output.json");
}
```
