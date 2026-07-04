// Example: building a small JSON array of records with the Writer API
// and saving it to disk.
//
// Demonstrates BeginArray/BeginObject/WriteKey/WriteString/WriteDouble/
// WriteBool/EndObject/EndArray, plus WriteWriterToFile for cross-platform
// (binary-mode) file output.

#include <cstdio>

#include "json.hpp"

using ascijson::BeginArray;
using ascijson::BeginObject;
using ascijson::EndArray;
using ascijson::EndObject;
using ascijson::InitWriter;
using ascijson::WriteBool;
using ascijson::WriteDouble;
using ascijson::WriteKey;
using ascijson::Writer;
using ascijson::WriterCStr;
using ascijson::WriterIsValid;
using ascijson::WriteString;
using ascijson::WriteWriterToFile;

// A single inventory entry.
struct Part {
  char category[20];
  char name[50];
  double price;
  char shop[20];
  char url[100];
  bool purchased;
};

// Serializes one Part as a JSON object. Returns false if the writer
// hits an error -- callers should bail on the first false, since Writer
// latches its error state and every subsequent call becomes a no-op.
bool WritePart(Writer* writer, const Part& part) {
  if (!BeginObject(writer)) return false;

  if (!WriteKey(writer, "category")) return false;
  if (!WriteString(writer, part.category)) return false;

  if (!WriteKey(writer, "name")) return false;
  if (!WriteString(writer, part.name)) return false;

  if (!WriteKey(writer, "price")) return false;
  if (!WriteDouble(writer, part.price, 2)) return false;  // 2dp: currency

  if (!WriteKey(writer, "shop")) return false;
  if (!WriteString(writer, part.shop)) return false;

  if (!WriteKey(writer, "url")) return false;
  if (!WriteString(writer, part.url)) return false;

  if (!WriteKey(writer, "purchased")) return false;
  if (!WriteBool(writer, part.purchased)) return false;

  return EndObject(writer);
}

// Serializes a list of Parts as a JSON array.
bool WritePartList(Writer* writer, const Part* parts, size_t count) {
  if (!BeginArray(writer)) return false;
  for (size_t i = 0; i < count; ++i) {
    if (!WritePart(writer, parts[i])) return false;
  }
  return EndArray(writer);
}

int main() {
  Part parts[] = {
      {"CPU", "Intel i3-9100F", 65.00, "Mindfactory",
       "https://example.com/i3-9100f", true},
      {"Cooling", "Noctua NH-L9x65", 45.00, "Alza",
       "https://example.com/nh-l9x65", false},
  };
  size_t part_count = sizeof(parts) / sizeof(parts[0]);

  char buffer[2048];
  Writer writer;
  InitWriter(&writer, buffer, sizeof(buffer));

  if (!WritePartList(&writer, parts, part_count) || !WriterIsValid(&writer)) {
    fprintf(stderr, "Error: failed to serialize part list (writer error).\n");
    return 1;
  }

  printf("Serialized JSON:\n%s\n\n", WriterCStr(&writer));

  if (!WriteWriterToFile(&writer, "partlist.json")) {
    fprintf(stderr, "Error: failed to write 'partlist.json'.\n");
    return 1;
  }

  printf("Wrote partlist.json (%zu bytes, %zu parts)\n", writer.length,
         part_count);
  return 0;
}
