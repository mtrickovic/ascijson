#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>

#include "../include/json.hpp"

using namespace ascijson;

// ---------------------------------------------------------------------------
// Config
// ---------------------------------------------------------------------------
static const unsigned int kIterations = 100000;
static const size_t kMaxInputSize = 512;
static const unsigned int kSeed = 0;  // 0 = use time

// ---------------------------------------------------------------------------
// Minimal PRNG (xorshift32 - no <random> needed, works on MSVC + g++)
// ---------------------------------------------------------------------------
static unsigned int g_rng_state;

static unsigned int Rand() {
  g_rng_state ^= g_rng_state << 13;
  g_rng_state ^= g_rng_state >> 17;
  g_rng_state ^= g_rng_state << 5;
  return g_rng_state;
}

static unsigned int RandRange(unsigned int lo, unsigned int hi) {
  return lo + (Rand() % (hi - lo + 1));
}

// ---------------------------------------------------------------------------
// Input generation
// ---------------------------------------------------------------------------

// Seeds for mutation-based fuzzing: valid inputs the parser should handle
static const char* kSeeds[] = {
    "{}",
    "[]",
    "{\"a\":1}",
    "{\"a\":true,\"b\":false,\"c\":null}",
    "{\"x\":[1,2,3]}",
    "[{\"ticker\":\"NVDA\",\"shares\":150,\"price\":924.50}]",
    "{\"project\":\"ascijson\",\"version\":\"0.1.0\",\"deps\":0}",
    "{\"a\":{\"b\":{\"c\":1}}}",
    // Tricky edge cases
    "{\"k\":\"\"}",
    "{\"k\":\"val with \\\"quotes\\\"\"}",
    "{\"a\":1,\"a\":2}",  // duplicate keys
    "{ }",
    "[ ]",
    "{\"n\":-1}",
    "{\"f\":-1.25}",
};
static const unsigned int kSeedCount =
    (unsigned int)(sizeof(kSeeds) / sizeof(kSeeds[0]));

static void GenerateRandom(char* buf, size_t size) {
  static const char kCharset[] =
      "{}[]\":,.-_0123456789abcdefghijklmnopqrstuvwxyz"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ \n\t\r\\";
  size_t len = RandRange(0, (unsigned int)(size - 1));
  for (size_t i = 0; i < len; ++i)
    buf[i] = kCharset[Rand() % (sizeof(kCharset) - 1)];
  buf[len] = '\0';
}

static void MutateSeed(const char* seed, char* buf, size_t buf_size) {
  size_t seed_len = strlen(seed);
  size_t copy_len = seed_len < buf_size - 1 ? seed_len : buf_size - 1;
  memcpy(buf, seed, copy_len);
  buf[copy_len] = '\0';

  unsigned int mutations = RandRange(1, 8);
  for (unsigned int m = 0; m < mutations; ++m) {
    size_t cur_len = strlen(buf);
    if (cur_len == 0) break;

    unsigned int op = Rand() % 4;
    size_t pos = Rand() % cur_len;

    switch (op) {
      case 0:  // flip a byte
        buf[pos] = (char)(Rand() % 256);
        break;
      case 1:  // truncate
        buf[pos] = '\0';
        break;
      case 2:  // insert a char (shift right by 1 if room)
        if (cur_len + 1 < buf_size) {
          memmove(buf + pos + 1, buf + pos, cur_len - pos + 1);
          buf[pos] = (char)(Rand() % 256);
        }
        break;
      case 3:  // delete a char
        memmove(buf + pos, buf + pos + 1, cur_len - pos);
        break;
    }
  }
}

// ----------------------------------------------------------------------------
// Invariant checks
// ----------------------------------------------------------------------------

// Rule: IsTrue, IsFalse, IsNull must be mutually exclusive for any one field.
static void CheckBooleanInvariants(const char* json, const char* field) {
  Error e1, e2, e3;
  bool t = IsTrue(json, field, &e1);
  bool f = IsFalse(json, field, &e2);
  bool n = IsNull(json, field, &e3);

  // At most one may be true
  int count = (t ? 1 : 0) + (f ? 1 : 0) + (n ? 1 : 0);
  assert(count <= 1 && "IsTrue/IsFalse/IsNull returned true simultaneously");
}

// Rule: if a function returns a non-null/non-false result, error must be kNone.
// Rule: if error is kNone but result is null/false, that is also acceptable
//       (field exists, type just didn't match the call).
static void CheckFindValue(const char* json, const char* key) {
  Error err = Error::kNone;
  const char* val = FindValue(json, key, &err);
  if (val != nullptr)
    assert(err == Error::kNone && "FindValue returned non-null but set error");
}

static void CheckCountFields(const char* json, const char* field) {
  Error err;
  unsigned int count = CountFields(json, field, &err);
  if (err != Error::kNone)
    assert(count == 0 && "CountFields returned non-zero count on error");
}

static void CheckCountArray(const char* json) {
  Error err;
  unsigned int count = CountArrayElements(json, &err);
  if (err != Error::kNone)
    assert(count == 0 && "CountArrayElements returned non-zero count on error");
}

static void CheckGetNthInt(const char* json, const char* field) {
  Error err;
  int val = 0;
  bool ok = GetNthInt(json, field, 0, &val, &err);
  if (ok)
    assert(err == Error::kNone && "GetNthInt returned true but set error");
  if (!ok)
    assert(err != Error::kNone &&
           "GetNthInt returned false but error is kNone");
}

static void CheckGetNthDouble(const char* json, const char* field) {
  Error err;
  double val = 0.0;
  bool ok = GetNthDouble(json, field, 0, &val, &err);
  if (ok)
    assert(err == Error::kNone && "GetNthDouble returned true but set error");
  if (!ok)
    assert(err != Error::kNone &&
           "GetNthDouble returned false but error is kNone");
}

static void CheckGetNthString(const char* json, const char* field) {
  Error err;
  char buf[128];
  bool ok = GetNthString(json, field, 0, buf, sizeof(buf), &err);
  if (ok)
    assert(err == Error::kNone && "GetNthString returned true but set error");
}

// ----------------------------------------------------------------------------
// Run all checks on one input
// ----------------------------------------------------------------------------
static const char* kTestFields[] = {
    "a",
    "b",
    "x",
    "ticker",
    "shares",
    "price",
    "project",
    "version",
    "text",
    "author",
    "dark_mode_enabled",
    "nonexistent_key",
};
static const unsigned int kFieldCount =
    (unsigned int)(sizeof(kTestFields) / sizeof(kTestFields[0]));

static void RunAllChecks(const char* input) {
  // These must never crash regardless of input
  CheckCountArray(input);

  const char* field = kTestFields[Rand() % kFieldCount];
  CheckFindValue(input, field);
  CheckCountFields(input, field);
  CheckGetNthInt(input, field);
  CheckGetNthDouble(input, field);
  CheckGetNthString(input, field);
  CheckBooleanInvariants(input, field);

  // Also probe null/empty field names
  CheckFindValue(input, "");
  CheckFindValue(input, nullptr);
}

// ----------------------------------------------------------------------------
// Main
// ----------------------------------------------------------------------------
int main(int argc, char* argv[]) {
  unsigned int seed = (kSeed != 0) ? kSeed : (unsigned int)time(nullptr);
  if (argc == 2) seed = (unsigned int)atoi(argv[1]);

  g_rng_state = seed;
  printf("ascijson fuzzer — seed: %u  iterations: %u\n", seed, kIterations);

  char buf[kMaxInputSize];
  unsigned int crashes = 0;

  for (unsigned int i = 0; i < kIterations; ++i) {
    // Alternate between fully random and seed-mutated inputs
    if (Rand() % 2 == 0) {
      GenerateRandom(buf, sizeof(buf));
    } else {
      const char* s = kSeeds[Rand() % kSeedCount];
      MutateSeed(s, buf, sizeof(buf));
    }

    RunAllChecks(buf);

    if ((i + 1) % 10000 == 0) printf("  ... %u / %u\n", i + 1, kIterations);
  }

  printf("Done. %u iterations, %u crashes.\n", kIterations, crashes);
  return EXIT_SUCCESS;
}
