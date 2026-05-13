#include "test_framework.hpp"
#include "../include/json.hpp"

using namespace ascijson;
using namespace ascijson::test;

void TestBasicCounting() {
  const char* json = "{\"a\": 1, \"b\": 2, \"a\": 3}";
  Assert("Count 'a' in flat object", CountFields(json, "a") == 2);
  Assert("Count 'b' in flat object", CountFields(json, "b") == 1);
  Assert("Count non-existent", CountFields(json, "z") == 0);
}

void TestNestedObjects() {
  // This tests if SkipValue correctly ignores 'a' inside the nested object 'b'
  const char* json = "{\"a\": 1, \"b\": {\"a\": 2}, \"c\": 3}";
  Assert("Count top-level 'a' only", CountFields(json, "a") == 1);
}

void TestComplexNesting() {
  const char* json = R"({
    "id": 1,
    "metadata": {
      "tags": ["id", "test"],
      "id": 99
    },
    "id": 2
  })";
  Assert("Ignore 'id' inside nested objects/arrays", CountFields(json, "id") == 2);
}

int main() {
  std::cout << "Running Tokenizer Tests...\n" << std::endl;

  TestBasicCounting();
  TestNestedObjects();
  TestComplexNesting();

  Summary();
  return (g_fail_count > 0) ? 1 : 0;
}
