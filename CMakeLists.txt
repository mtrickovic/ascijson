cmake_minimum_required(VERSION 3.10)
project(ascijson VERSION 0.3.0 LANGUAGES CXX)

# Standard C++17 compliance
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Generate compile_commands.json for clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

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
  src/json_writer.cpp
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

  add_executable(writer_test tests/writer_test.cpp)
  target_link_libraries(writer_test PRIVATE ascijson)

  add_test(NAME WriterTests COMMAND writer_test)
endif()

# 3. Fuzzing (Disabled by default)
option(BUILD_FUZZING "Build the fuzz tester" OFF)
if (BUILD_FUZZING)
  add_executable(fuzz tests/fuzz.cpp)
  target_link_libraries(fuzz PRIVATE ascijson)

  # Enable ASan + UBSan on GCC/Clang for deeper crash detection
  if (NOT MSVC)
    target_compile_options(fuzz PRIVATE -fsanitize=address,undefined)
    target_link_options(fuzz PRIVATE -fsanitize=address,undefined)
  endif()
endif()

# 4. Examples
add_executable(basic_usage examples/basic_usage.cpp)
target_link_libraries(basic_usage PRIVATE ascijson)
add_executable(quotes_display examples/quotes_display/main.cpp)
target_link_libraries(quotes_display PRIVATE ascijson)
add_executable(basic_numerics examples/basic_numerics/main.cpp)
target_link_libraries(basic_numerics PRIVATE ascijson)
add_executable(basic_boolnull examples/basic_boolnull/main.cpp)
target_link_libraries(basic_boolnull PRIVATE ascijson)
add_executable(partlist examples/partlist.cpp)
target_link_libraries(partlist PRIVATE ascijson)

# Copy quotes.json next to the executable after build
add_custom_command(TARGET quotes_display POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/quotes_display/quotes.json
    $<TARGET_FILE_DIR:quotes_display>/quotes.json
  COMMENT "Copying quotes.json to build dir"
)

# Copy portfolio.json next to the executable after build
add_custom_command(TARGET basic_numerics POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/basic_numerics/portfolio.json
    $<TARGET_FILE_DIR:basic_numerics>/portfolio.json
  COMMENT "Copying portfolio.json to build dir"
)

# Copy flags.json next to the executable after build
add_custom_command(TARGET basic_boolnull POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E copy
    ${CMAKE_CURRENT_SOURCE_DIR}/examples/basic_boolnull/flags.json
    $<TARGET_FILE_DIR:basic_boolnull>/flags.json
  COMMENT "Copying flags.json to build dir"
)
