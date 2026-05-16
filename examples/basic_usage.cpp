#include <iostream>
#include "json.hpp"

int main() {

  // A raw string literal makes JSON examples much cleaner in C++
  const char* json = R"({
    "project": "ascijson",
    "status": "active",
    "version": "0.1.0",
    "dependencies": 0
  })";

  ascijson::Error err;

  // Using out linear-scan field counter
  unsigned int project_fields = ascijson::CountFields(json, "project", &err);
  if (err != ascijson::Error::kNone) {
    std::cout << "Your JSON is invalid" << std::endl;
    return EXIT_FAILURE;
  }
  unsigned int version_fields = ascijson::CountFields(json, "version", &err);
  if (err != ascijson::Error::kNone) {
    std::cout << "Your JSON is invalid" << std::endl;
    return EXIT_FAILURE;
  }

  std::cout << "--- ascijson Basic Example ---" << std::endl;
  std::cout << "Project field found: " << (project_fields > 0 ? "Yes" : "No")
            << std::endl;
  std::cout << "Version field count: " << version_fields << std::endl;

  return 0;
}
