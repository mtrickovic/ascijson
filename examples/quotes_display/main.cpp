#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include "../../include/json.hpp"

int main() {
  FILE* f = nullptr;
#ifdef _MSC_VER
  fopen_s(&f, "quotes.json", "rb");
#else
  f = fopen("quotes.json", "rb");
#endif
  if (!f) return 1;

  fseek(f, 0, SEEK_END);
  long size = ftell(f);
  if (size < 0) { fclose(f); return 1; }
  fseek(f, 0, SEEK_SET);

  char* buf = new char[size + 1];
  if ((long)fread(buf, 1, size, f) != size) {
    fprintf(stderr, "Error: Could not read file completely\n");
    fclose(f); delete[] buf; return 1;
  }
  fclose(f);
  buf[size] = '\0';

  // Step 1: find the "quotes" array inside the top-level object
  const char* quotes_array = ascijson::FindValue(buf, "quotes");
  if (!quotes_array) { delete[] buf; return 1; }

  // Step 2: count how many quotes are in the array
  unsigned int total = ascijson::CountArrayElements(quotes_array);
  if (total == 0) { delete[] buf; return 1; }

  std::srand(static_cast<unsigned int>(std::time(nullptr)));
  unsigned int random_idx = std::rand() % total;

  // Step 3: get a pointer to the Nth { "text":..., "author":... } object
  const char* entry = ascijson::GetNthElement(quotes_array, random_idx);
  if (!entry) { delete[] buf; return 1; }

  // Step 4: extract fields from that object - "text" is now at top level of
  // entry
  char quote[256] = {};
  char author[128] = {};
  if (ascijson::GetNthString(entry, "text",   0, quote,  sizeof(quote)) &&
      ascijson::GetNthString(entry, "author", 0, author, sizeof(author))) {
    printf("\"%s\"\n", quote);
    printf("  - %s\n", author);
  }

  delete[] buf;
  return 0;
}
