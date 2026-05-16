#include <iostream>
#include <iomanip>
#include "../../include/json.hpp"

using namespace ascijson;

// Simple helper to load file into a char buffer
char* LoadFile(const char* path);

int main()
{
  const char* json_data = LoadFile("portfolio.json");
  if (!json_data) return EXIT_FAILURE;

  std::cout << "--- Portfolio Analysis ---" << std::endl;

  Error err;

  // 1. Extract root-level integers and doubles
  int asset_count = 0;
  double multiplier = 0.0;

  if (GetNthInt(json_data, "total_assets", 0, &asset_count, &err)) {
    std::cout << "Assets Managed: " << asset_count << std::endl;
  } else {
    std::cerr << "Warning: could not read total_assets" << std::endl;
  }

  if (GetNthDouble(json_data, "performance_multiplier", 0, &multiplier, &err)) {
    std::cout << "Performance: "
              << (multiplier > 0 ? "+" : "")
              << multiplier
              << "%"
              << std::endl;
  } else {
    std::cerr << "Warning: could not read performance_multiplier" << std::endl;
  }

  // 2. Navigate the holdings array
  const char* holdings = FindValue(json_data, "holdings", &err);
  if (!holdings) {
    std::cerr << "Error: holdings array not found" << std::endl;
    delete[] json_data;
    return EXIT_FAILURE;
  }

  unsigned int count = CountArrayElements(holdings, &err);
  if (err != Error::kNone) {
    std::cerr << "Error: could not count holdings" << std::endl;
    delete[] json_data;
    return EXIT_FAILURE;
  }

  for (unsigned int i = 0; i < count; ++i) {
    const char* item = GetNthElement(holdings, i, &err);
    if (!item) {
      std::cerr << "Warning: could not read holding [" << i << "]" << std::endl;
      continue;
    }

    char ticker[8] = {0};
    int shares = 0;
    double price = 0.0;

    if (!GetNthString(item, "ticker", 0, ticker, sizeof(ticker), &err))
      std::cerr << "Warning: missing ticker in holdings ["
                << i
                << "]"
                << std::endl;
    if (!GetNthInt(item, "shares", 0, &shares, &err))
      std::cerr << "Warning: missing shares in holdings ["
                << i
                << "]"
                << std::endl;
    if (!GetNthDouble(item, "price", 0, &price, &err))
      std::cerr << "Warning: missing price in holding ["
                << i
                << "]"
                << std::endl;

    std::cout << "Holding [" << ticker << "]: "
              << shares << " shares @ $"
              << std::fixed << std::setprecision(2) << price << std::endl;
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
