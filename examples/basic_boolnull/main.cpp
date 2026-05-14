#include <iostream>
#include "../../include/json.hpp"

using namespace ascijson;

char* LoadFile(const char* path);

// Prints a three-state status label for a flag field:
//   true  -> "[ON]"
//   false -> "[OFF]"
//   null  -> "[UNSET]"
//   else  -> "[UNKNOWN]"
static void PrintFlag(const char* json, const char* field) {
  const char* status = "[UNKNOWN]";
  if      (IsTrue (json, field)) status = "[ON]";
  else if (IsFalse(json, field)) status = "[OFF]";
  else if (IsNull (json, field)) status = "[UNSET]";

  std::cout << "  " << field << ": " << status << std::endl;
}

int main() {
  const char* json_data = LoadFile("flags.json");
  if (!json_data) return EXIT_FAILURE;

  std::cout << "--- Feature Flag Report ---" << std::endl;
  PrintFlag(json_data, "dark_mode_enabled");
  PrintFlag(json_data, "maintenance_mode");
  PrintFlag(json_data, "beta_feature_enabled");
  PrintFlag(json_data, "deprecated_endpoint");
  PrintFlag(json_data, "experimental_parser");
  PrintFlag(json_data, "override_config");

  // Demonstrate graceful handling of a missing key
  PrintFlag(json_data, "nonexistent_flag");

  return EXIT_SUCCESS;
}

char* LoadFile(const char* path)
{
  FILE* f = nullptr;
#ifdef _MSC_VER
  fopen_s(&f, path, "rb");
#else
  f = fopen(path, "rb");
#endif
  if (!f) return nullptr;

  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  if (size < 0) { fclose(f); return nullptr; }
  fseek(f, 0, SEEK_SET);

  char* json_data = new char[size + 1];
  if ((long)fread(json_data, 1, size, f) != size) {
    fprintf(stderr, "Error: Could not read file completely\n");
    fclose(f); delete[] json_data; return nullptr;
  }
  fclose(f);
  json_data[size] = '\0';

  return json_data;
}
