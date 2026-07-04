#include <cstring>

#include "../include/json.hpp"
#include "test_framework.hpp"

using namespace ascijson;
using namespace ascijson::test;

int main() {
  std::cout << "--- ascijson Tokenizer Stress Tests ---\n" << std::endl;

  // -------------------------------------------------------
  // CountFields
  // -------------------------------------------------------
  std::cout << "-- CountFields --\n";

  // Null / empty safety
  Assert("CF: null json", CountFields(nullptr, "key") == 0);
  Assert("CF: null key", CountFields("{}", nullptr) == 0);
  Assert("CF: empty string", CountFields("", "key") == 0);
  Assert("CF: empty object", CountFields("{}", "a") == 0);

  // Basic counting
  const char* flat = "{\"a\": 1, \"b\": 2, \"a\": 3}";
  Assert("CF: multiple matches", CountFields(flat, "a") == 2);
  Assert("CF: single match", CountFields(flat, "b") == 1);
  Assert("CF: missing key", CountFields(flat, "z") == 0);

  // Prefix protection — 'user' must not match 'username'
  const char* prefix = "{\"username\": \"alice\", \"user\": \"bob\"}";
  Assert("CF: prefix isolation", CountFields(prefix, "user") == 1);

  // Nested isolation — should NOT descend into child objects
  const char* nested = "{\"a\": 1, \"b\": {\"a\": 2}, \"c\": 3}";
  Assert("CF: ignore nested keys", CountFields(nested, "a") == 1);

  // Nested arrays/objects
  const char* complex = R"({
    "id": 1,
    "metadata": { "tags": ["id", "test"], "id": 99 },
    "id": 2
  })";
  Assert("CF: ignore array/object contents", CountFields(complex, "id") == 2);

  // Key whose value is an array
  const char* with_array = "{\"items\": [1,2,3], \"items\": [4]}";
  Assert("CF: key with array value", CountFields(with_array, "items") == 2);

  // -------------------------------------------------------
  // GetNthString
  // -------------------------------------------------------
  std::cout << "\n-- GetNthString --\n";

  const char* strings = R"({"x": "hello", "y": "world", "x": "again"})";
  char buf[64] = {};

  Assert("GNS: null json", !GetNthString(nullptr, "x", 0, buf, sizeof(buf)));
  Assert("GNS: null key", !GetNthString(strings, nullptr, 0, buf, sizeof(buf)));
  Assert("GNS: null buffer",
         !GetNthString(strings, "x", 0, nullptr, sizeof(buf)));
  Assert("GNS: zero buf size", !GetNthString(strings, "x", 0, buf, 0));

  memset(buf, 0, sizeof(buf));
  Assert("GNS: first match", GetNthString(strings, "x", 0, buf, sizeof(buf)));
  Assert("GNS: first value", strcmp(buf, "hello") == 0);

  memset(buf, 0, sizeof(buf));
  Assert("GNS: second match", GetNthString(strings, "x", 1, buf, sizeof(buf)));
  Assert("GNS: second value", strcmp(buf, "again") == 0);

  memset(buf, 0, sizeof(buf));
  Assert("GNS: only match", GetNthString(strings, "y", 0, buf, sizeof(buf)));
  Assert("GNS: only value", strcmp(buf, "world") == 0);

  Assert("GNS: out of range",
         !GetNthString(strings, "x", 99, buf, sizeof(buf)));
  Assert("GNS: missing key", !GetNthString(strings, "z", 0, buf, sizeof(buf)));

  // Non-string value should return false
  const char* numval = "{\"n\": 42}";
  Assert("GNS: non-string val",
         !GetNthString(numval, "n", 0, buf, sizeof(buf)));

  // Escaped quote inside string value
  const char* escaped = "{\"msg\": \"say \\\"hi\\\"\"}";
  memset(buf, 0, sizeof(buf));
  Assert("GNS: escaped quotes",
         GetNthString(escaped, "msg", 0, buf, sizeof(buf)));

  // Escape sequence handling
  const char* escapes =
      R"({"a":"hello\nworld","b":"tab\there","c":"back\\slash",)"
      R"("d":"quote\"end","e":"caf\u00e9"})";

  memset(buf, 0, sizeof(buf));
  Assert("ESC: newline", GetNthString(escapes, "a", 0, buf, sizeof(buf)) &&
                             strcmp(buf, "hello\nworld") == 0);

  memset(buf, 0, sizeof(buf));
  Assert("ESC: tab", GetNthString(escapes, "b", 0, buf, sizeof(buf)) &&
                         strcmp(buf, "tab\there") == 0);

  memset(buf, 0, sizeof(buf));
  Assert("ESC: backslash", GetNthString(escapes, "c", 0, buf, sizeof(buf)) &&
                               strcmp(buf, "back\\slash") == 0);

  memset(buf, 0, sizeof(buf));
  Assert("ESC: quote", GetNthString(escapes, "d", 0, buf, sizeof(buf)) &&
                           strcmp(buf, "quote\"end") == 0);

  memset(buf, 0, sizeof(buf));
  Assert("ESC: unicode placeholder",
         GetNthString(escapes, "e", 0, buf, sizeof(buf)) &&
             strcmp(buf, "caf?") == 0);

  // -------------------------------------------------------
  // FindValue
  // -------------------------------------------------------
  std::cout << "\n-- FindValue --\n";

  const char* obj = R"({"name": "alice", "age": 30, "scores": [1,2,3]})";
  Assert("FV: null json", FindValue(nullptr, "name") == nullptr);
  Assert("FV: null key", FindValue(obj, nullptr) == nullptr);
  Assert("FV: missing key", FindValue(obj, "missing") == nullptr);
  Assert("FV: string value", FindValue(obj, "name") != nullptr);
  Assert("FV: number value", FindValue(obj, "age") != nullptr);
  Assert("FV: array value", FindValue(obj, "scores") != nullptr);

  // Verify the pointer actually points at the value
  const char* name_val = FindValue(obj, "name");
  Assert("FV: points at value", name_val && *name_val == '"');

  const char* arr_val = FindValue(obj, "scores");
  Assert("FV: points at array", arr_val && *arr_val == '[');

  // -------------------------------------------------------
  // CountArrayElements
  // -------------------------------------------------------
  std::cout << "\n-- CountArrayElements --\n";

  Assert("CAE: null", CountArrayElements(nullptr) == 0);
  Assert("CAE: not an array", CountArrayElements("{\"a\":1}") == 0);
  Assert("CAE: empty array", CountArrayElements("[]") == 0);
  Assert("CAE: one element", CountArrayElements("[1]") == 1);
  Assert("CAE: three numbers", CountArrayElements("[1, 2, 3]") == 3);
  Assert("CAE: three strings", CountArrayElements("[\"a\",\"b\",\"c\"]") == 3);
  Assert("CAE: nested objects",
         CountArrayElements("[{\"x\":1},{\"x\":2}]") == 2);

  // Array inside real JSON (via FindValue)
  const char* doc = R"({"items": ["one", "two", "three"]})";
  const char* items = FindValue(doc, "items");
  Assert("CAE: via FindValue", items && CountArrayElements(items) == 3);

  // -------------------------------------------------------
  // GetNthElement
  // -------------------------------------------------------
  std::cout << "\n-- GetNthElement --\n";

  const char* arr = "[\"alpha\", \"beta\", \"gamma\"]";
  Assert("GNE: null json", GetNthElement(nullptr, 0) == nullptr);
  Assert("GNE: not an array", GetNthElement("{}", 0) == nullptr);
  Assert("GNE: out of range", GetNthElement(arr, 99) == nullptr);
  Assert("GNE: first element", GetNthElement(arr, 0) != nullptr);
  Assert("GNE: third element", GetNthElement(arr, 2) != nullptr);

  // Verify pointer lands on the right character
  const char* el0 = GetNthElement(arr, 0);
  Assert("GNE: el0 is string", el0 && *el0 == '"');

  // Round-trip: GetNthElement + GetNthString on an object array
  const char* objarr = R"([{"k":"one"},{"k":"two"},{"k":"three"}])";
  const char* el2 = GetNthElement(objarr, 2);
  memset(buf, 0, sizeof(buf));
  Assert("GNE: roundtrip get",
         el2 && GetNthString(el2, "k", 0, buf, sizeof(buf)));
  Assert("GNE: roundtrip val", strcmp(buf, "three") == 0);

  // -------------------------------------------------------
  // Test Numeric Precision
  // -------------------------------------------------------
  std::cout << "\n-- TestNumericPrecision --\n";

  const char* json = R"({
    "int_val": 12345,
    "neg_int": -99,
    "pi": 3.14159,
    "zero_point": 0.5
  })";

  int i_val = 0;
  int n_val = 0;
  double d_val = 0.0;

  // Test Integer Parsing
  Assert("Positive Int Conversion",
         GetNthInt(json, "int_val", 0, &i_val) && i_val == 12345);
  Assert("Negative Int Conversion",
         GetNthInt(json, "neg_int", 0, &n_val) && n_val == -99);

  // Test Double Parsing
  Assert("Double Precision (PI)", GetNthDouble(json, "pi", 0, &d_val) &&
                                      (d_val > 3.1415 && d_val < 3.1416));
  Assert("Leading Zero Double",
         GetNthDouble(json, "zero_point", 0, &d_val) && d_val == 0.5);

  // -------------------------------------------------------
  // IsTrue / IsFalse / IsNull
  // -------------------------------------------------------
  std::cout << "\n-- IsTrue --\n";

  const char* flags = R"({
    "active":      true,
    "disabled":    false,
    "unset":       null,
    "count":       1,
    "label":       "true",
    "truthy_ish":  truecolor
  })";

  // Affirmative cases
  Assert("IT: basic true", IsTrue(flags, "active"));

  // Negative cases - wrong literal
  Assert("IT: false is not true", !IsTrue(flags, "disabled"));
  Assert("IT: null is not true", !IsTrue(flags, "unset"));

  // Must not match numeric 1 or string "true"
  Assert("IT: int is not true", !IsTrue(flags, "count"));
  Assert("IT: string is not true", !IsTrue(flags, "label"));

  // Prefix guard - "truecolor" must not match "true"
  Assert("IT: prefix guard", !IsTrue(flags, "truthy_ish"));

  // Safety
  Assert("IT: null json", !IsTrue(nullptr, "active"));
  Assert("IT: null key", !IsTrue(flags, nullptr));
  Assert("IT: missing key", !IsTrue(flags, "ghost"));

  // -------------------------------------------------------
  std::cout << "\n-- IsFalse --\n";

  Assert("IF: basic false", IsFalse(flags, "disabled"));

  Assert("IF: true is not false", !IsFalse(flags, "active"));
  Assert("IF: null is not false", !IsFalse(flags, "unset"));
  Assert("IF: int is not false", !IsFalse(flags, "count"));
  Assert("IF: string is not false", !IsFalse(flags, "label"));

  Assert("IF: null json", !IsFalse(nullptr, "disabled"));
  Assert("IF: null key", !IsFalse(flags, nullptr));
  Assert("IF: missing key", !IsFalse(flags, "ghost"));

  // -------------------------------------------------------
  std::cout << "\n-- IsNull --\n";

  Assert("IN: basic null", IsNull(flags, "unset"));

  Assert("IN: true is not null", !IsNull(flags, "active"));
  Assert("IN: false is not null", !IsNull(flags, "disabled"));
  Assert("IN: int is not null", !IsNull(flags, "count"));
  Assert("IN: string is not null", !IsNull(flags, "label"));

  Assert("IN: null json", !IsNull(nullptr, "unset"));
  Assert("IN: null key", !IsNull(flags, nullptr));
  Assert("IN: missing key", !IsNull(flags, "ghost"));

  // Multiple nulls in same object - verify FindValue finds the right one
  const char* multi_null = R"({"a": null, "b": true, "c": null})";
  Assert("IN: first of two nulls", IsNull(multi_null, "a"));
  Assert("IN: second of two nulls", IsNull(multi_null, "c"));
  Assert("IN: non-null between", !IsNull(multi_null, "b"));

  // Nested isolation - should not descend into child objects
  const char* nested_flag = R"({"outer": false, "child": {"outer": true}})";
  Assert("IN: nested isolation IT", !IsTrue(nested_flag, "outer"));
  Assert("IN: nested isolation IF", IsFalse(nested_flag, "outer"));

  // -------------------------------------------------------
  Summary();
  return (g_fail_count > 0) ? 1 : 0;
}
