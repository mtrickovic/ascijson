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

  // Using out linear-scan field counter
  unsigned int project_fields = ascijson::CountFields(json, "project");
  unsigned int version_fields = ascijson::CountFields(json, "version");

  std::cout << "--- ascijson Basic Example ---" << std::endl;
  std::cout << "Project field found: " << (project_fields > 0 ? "Yes" : "No")
            << std::endl;
  std::cout << "Version field count: " << version_fields << std::endl;

  return 0;
}
