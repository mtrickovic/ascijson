#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include "../../include/json.hpp"

using namespace ascijson;

// Simple helper to load file into a char json_datafer
char* LoadFile(const char* path);

int main() {
  const char* json_data = LoadFile("quotes.json");
  if (json_data == nullptr) return EXIT_FAILURE;

  // Step 1: find the "quotes" array inside the top-level object
  const char* quotes_array = FindValue(json_data, "quotes");
  if (!quotes_array) { delete[] json_data; return 1; }

  // Step 2: count how many quotes are in the array
  unsigned int total = CountArrayElements(quotes_array);
  if (total == 0) { delete[] json_data; return 1; }

  std::srand(static_cast<unsigned int>(std::time(nullptr)));
  unsigned int random_idx = std::rand() % total;

  // Step 3: get a pointer to the Nth { "text":..., "author":... } object
  const char* entry = GetNthElement(quotes_array, random_idx);
  if (!entry) { delete[] json_data; return 1; }

  // Step 4: extract fields from that object - "text" is now at top level of
  // entry
  char quote[256] = {};
  char author[128] = {};
  if (GetNthString(entry, "text",   0, quote,  sizeof(quote)) &&
      GetNthString(entry, "author", 0, author, sizeof(author))) {
    printf("\"%s\"\n", quote);
    printf("  - %s\n", author);
  }

  delete[] json_data;
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
