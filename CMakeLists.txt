cmake_minimum_required(VERSION 3.10)
project(ascijson VERSION 0.1.0 LANGUAGES CXX)

# Standard C++17 compliance
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler flags for high quality (Google Style encouragement)
if (MSVC)
  add_compile_options(/W4 /WX)
else()
  add_compile_options(-Wall -Wextra -Wpedantic -Werror)
endif()

# 1. The Library Target
# Since we are in the 'Foundation' phase, we build as a static library.
# Later, this can be transitioned to an Interface library if you go header-only.
add_library(ascijson
  src/tokenizer.cpp
)

# Define public headers directory
target_include_directories(ascijson PUBLIC
  $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
  $<INSTALL_INTERFACE:include>
)

# 2. Testing (Enabled by default, can be toggled)
option(BUILD_TESTING "Build the test suite" ON)
if (BUILD_TESTING)
  enable_testing()

  add_executable(tokenizer_test tests/tokenizer_test.cpp)
  target_link_libraries(tokenizer_test PRIVATE ascijson)

  add_test(NAME TokenizerTests COMMAND tokenizer_test)
endif()

# 3. Examples
add_executable(basic_usage examples/basic_usage.cpp)
target_link_libraries(basic_usage PRIVATE ascijson)
