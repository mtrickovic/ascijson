#include <cstdio>
#include <cstring>

#include "../include/json.hpp"
#include "test_framework.hpp"

using namespace ascijson;
using namespace ascijson::test;

void TestSimpleObject() {
  char buf[256];
  Writer w;
  InitWriter(&w, buf, sizeof(buf));

  BeginObject(&w);
  WriteKey(&w, "name");
  WriteString(&w, "Intel i3-9100F");
  WriteKey(&w, "price");
  WriteDouble(&w, 65.0);
  WriteKey(&w, "purchased");
  WriteBool(&w, true);
  EndObject(&w);

  Assert("SimpleObject: valid", WriterIsValid(&w));
  Assert("SimpleObject: content",
         std::strcmp(WriterCStr(&w),
                     "{\"name\":\"Intel i3-9100F\",\"price\":65,"
                     "\"purchased\":true}") == 0);
}

void TestArrayOfObjects() {
  char buf[512];
  Writer w;
  InitWriter(&w, buf, sizeof(buf));

  BeginArray(&w);

  BeginObject(&w);
  WriteKey(&w, "category");
  WriteString(&w, "CPU");
  WriteKey(&w, "price");
  WriteDouble(&w, 65.0, 2);
  EndObject(&w);

  BeginObject(&w);
  WriteKey(&w, "category");
  WriteString(&w, "Cooling");
  WriteKey(&w, "price");
  WriteDouble(&w, 45.5, 2);
  EndObject(&w);

  EndArray(&w);

  Assert("ArrayOfObjects: valid", WriterIsValid(&w));
  Assert("ArrayOfObjects: content",
         std::strcmp(WriterCStr(&w),
                     "[{\"category\":\"CPU\",\"price\":65},"
                     "{\"category\":\"Cooling\",\"price\":45.5}]") == 0);
}

void TestEmptyObjectAndArray() {
  char buf[64];
  Writer w;
  InitWriter(&w, buf, sizeof(buf));

  BeginObject(&w);
  WriteKey(&w, "tags");
  BeginArray(&w);
  EndArray(&w);
  EndObject(&w);

  Assert("EmptyObjectAndArray: valid", WriterIsValid(&w));
  Assert("EmptyObjectAndArray: content",
         std::strcmp(WriterCStr(&w), "{\"tags\":[]}") == 0);
}

void TestStringEscaping() {
  char buf[128];
  Writer w;
  InitWriter(&w, buf, sizeof(buf));

  BeginObject(&w);
  WriteKey(&w, "note");
  WriteString(&w, "quote\" backslash\\ newline\n tab\t");
  EndObject(&w);

  Assert("StringEscaping: valid", WriterIsValid(&w));
  Assert("StringEscaping: content",
         std::strcmp(WriterCStr(&w),
                     "{\"note\":\"quote\\\" backslash\\\\ newline\\n "
                     "tab\\t\"}") == 0);
}

void TestDoubleTrimming() {
  char buf[64];

  {
    Writer w;
    InitWriter(&w, buf, sizeof(buf));
    WriteDouble(&w, 65.0);
    Assert("DoubleTrimming: whole number",
           std::strcmp(WriterCStr(&w), "65") == 0);
  }
  {
    Writer w;
    InitWriter(&w, buf, sizeof(buf));
    WriteDouble(&w, 65.5);
    Assert("DoubleTrimming: one decimal",
           std::strcmp(WriterCStr(&w), "65.5") == 0);
  }
  {
    Writer w;
    InitWriter(&w, buf, sizeof(buf));
    WriteDouble(&w, -12.340, 2);
    Assert("DoubleTrimming: negative, trailing zero",
           std::strcmp(WriterCStr(&w), "-12.34") == 0);
  }
  {
    Writer w;
    InitWriter(&w, buf, sizeof(buf));
    WriteDouble(&w, 0.0);
    Assert("DoubleTrimming: zero", std::strcmp(WriterCStr(&w), "0") == 0);
  }
}

void TestNullAndBool() {
  char buf[64];
  Writer w;
  InitWriter(&w, buf, sizeof(buf));

  BeginArray(&w);
  WriteNull(&w);
  WriteBool(&w, true);
  WriteBool(&w, false);
  WriteInt(&w, -7);
  EndArray(&w);

  Assert("NullAndBool: valid", WriterIsValid(&w));
  Assert("NullAndBool: content",
         std::strcmp(WriterCStr(&w), "[null,true,false,-7]") == 0);
}

void TestBufferOverflow() {
  // Buffer is deliberately too small to hold the object.
  char buf[8];
  Writer w;
  InitWriter(&w, buf, sizeof(buf));

  Assert("BufferOverflow: BeginObject succeeds", BeginObject(&w));
  Assert("BufferOverflow: WriteKey fails", !WriteKey(&w, "category"));
  Assert("BufferOverflow: writer invalid after overflow", !WriterIsValid(&w));
  Assert("BufferOverflow: WriterCStr returns nullptr",
         WriterCStr(&w) == nullptr);
}

void TestInvalidState() {
  {
    char buf[64];
    Writer w;
    InitWriter(&w, buf, sizeof(buf));
    Assert("InvalidState: EndObject without BeginObject fails", !EndObject(&w));
    Assert("InvalidState: writer invalid", !WriterIsValid(&w));
  }
  {
    char buf[64];
    Writer w;
    InitWriter(&w, buf, sizeof(buf));
    BeginObject(&w);
    Assert("InvalidState: EndArray closing an object fails", !EndArray(&w));
  }
  {
    char buf[64];
    Writer w;
    InitWriter(&w, buf, sizeof(buf));
    BeginArray(&w);
    Assert("InvalidState: WriteKey inside array fails", !WriteKey(&w, "oops"));
  }
  {
    char buf[64];
    Writer w;
    InitWriter(&w, buf, sizeof(buf));
    BeginObject(&w);
    WriteKey(&w, "a");
    WriteInt(&w, 1);
    Assert("InvalidState: unclosed object is not valid", !WriterIsValid(&w));
  }
}

void TestWriteWriterToFile() {
  char buf[128];
  Writer w;
  InitWriter(&w, buf, sizeof(buf));

  BeginObject(&w);
  WriteKey(&w, "ok");
  WriteBool(&w, true);
  EndObject(&w);

  Assert("WriteWriterToFile: valid before write", WriterIsValid(&w));

  const char* path = "test_output.json";
  Assert("WriteWriterToFile: write succeeds", WriteWriterToFile(&w, path));

  FILE* fp = OpenFile(path, "rb");
  Assert("WriteWriterToFile: file opens for read", fp != nullptr);
  if (fp != nullptr) {
    char read_buf[128] = {};
    size_t n = fread(read_buf, 1, sizeof(read_buf) - 1, fp);
    read_buf[n] = '\0';
    fclose(fp);
    Assert("WriteWriterToFile: round-tripped content matches",
           std::strcmp(read_buf, "{\"ok\":true}") == 0);
  }
  remove(path);
}

int main() {
  TestSimpleObject();
  TestArrayOfObjects();
  TestEmptyObjectAndArray();
  TestStringEscaping();
  TestDoubleTrimming();
  TestNullAndBool();
  TestBufferOverflow();
  TestInvalidState();
  TestWriteWriterToFile();

  Summary();
  return (g_fail_count == 0) ? 0 : 1;
}
