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

    inline void Summary() {
      std::cout << "\n--- Test Summary ---" << std::endl;
      std::cout << "Passed: " << g_pass_count << std::endl;
      std::cout << "Failed: " << g_fail_count << std::endl;
    }

  }
}

#endif  // ASCIJSON_TESTS_TEST_FRAMEWORK_HPP_
