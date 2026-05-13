#include "test_framework.hpp"
#include "../include/json.hpp"

using namespace ascijson;
using namespace ascijson::test;

int main() {
  std::cout << "--- ascijson Tokenizer Stress Tests ---\n" << std::endl;

  // 1. Safety & Null Checks
  Assert("Handle null JSON pointer", CountFields(nullptr, "key") == 0);
  Assert("Handle null key pointer",  CountFields("{}", nullptr) == 0);
  Assert("Handle empty string",      CountFields("", "key") == 0);

  // 2. Multi-occurrence & Non-existent
  const char* flat = "{\"a\": 1, \"b\": 2, \"a\": 3}";
  Assert("Count 'a' (multiple)", CountFields(flat, "a") == 2);
  Assert("Count 'z' (missing)",  CountFields(flat, "z") == 0);

  // 3. Nested Object Isolation
  // Should ignore the 'a' inside the nested object 'b'
  const char* nested = "{\"a\": 1, \"b\": {\"a\": 2}, \"c\": 3}";
  Assert("Ignore nested object keys", CountFields(nested, "a") == 1);

  // 4. Complex Array/Object Nesting
  // Should ignore 'id' inside the metadata object and the tags array
  const char* complex = R"({
    "id": 1,
    "metadata": {
      "tags": ["id", "test"],
      "id": 99
    },
    "id": 2
  })";
  Assert("Ignore keys in nested arrays/objects",
         CountFields(complex, "id") == 2);

  // 5. Prefix Protection
  // Ensures 'user' doesn't match 'username'
  const char* prefix = "{\"username\": \"alice\", \"user\": \"bob\"}";
  Assert("Prevent prefix matching", CountFields(prefix, "user") == 1);

  Summary();
  return (g_fail_count > 0) ? 1 : 0;
}
