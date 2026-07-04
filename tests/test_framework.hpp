#ifndef ASCIJSON_TESTS_TEST_FRAMEWORK_HPP_
#define ASCIJSON_TESTS_TEST_FRAMEWORK_HPP_

#include <iostream>

namespace ascijson {
namespace test {

static int g_pass_count = 0;
static int g_fail_count = 0;

inline void Assert(const char* name, bool condition) {
  if (condition) {
    std::cout << "[ PASS ] " << name << std::endl;
    g_pass_count++;
  } else {
    std::cout << "[ FAIL ] " << name << std::endl;
    g_fail_count++;
  }
}

inline FILE* OpenFile(const char* path, const char* mode) {
#ifdef _MSC_VER
  FILE* fp = nullptr;
  return fopen_s(&fp, path, mode) == 0 ? fp : nullptr;
#else
  return fopen(path, mode);
#endif
}

inline void Summary() {
  std::cout << "\n--- Test Summary ---" << std::endl;
  std::cout << "Passed: " << g_pass_count << std::endl;
  std::cout << "Failed: " << g_fail_count << std::endl;
}

}  // namespace test
}  // namespace ascijson

#endif  // ASCIJSON_TESTS_TEST_FRAMEWORK_HPP_
