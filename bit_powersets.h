#ifndef BIT_POWERSETS
#define BIT_POWERSETS

#include <assert.h>
#include <set>
#include <stdint.h>

using namespace std;

typedef uint64_t BitPowerset;

const BitPowerset ZERO = 0;
const BitPowerset ONE = 1;


set<int> unpack(BitPowerset b) {
  set<int> result;
  for (int i = 0; i < 64; i++) {
    if (b & (ONE << i))
      result.insert(i);
  }
  return result;
}

BitPowerset pack(set<int> xs) {
  BitPowerset result = 0;
  for (int x : xs)
    result |= ONE << x;
  return result;
}


BitPowerset naive_set_bit(BitPowerset b, int bit) {
  auto xs = unpack(b);
  set<int> ys;
  for (int x : xs) {
    if (x & ONE<<bit) continue;
    ys.insert(x | ONE<<bit);
  }
  return pack(ys);
}

BitPowerset naive_clear_bit(BitPowerset b, int bit) {
  auto xs = unpack(b);
  set<int> ys;
  for (int x : xs) {
    if (x & ONE<<bit)
      ys.insert(x ^ ONE<<bit);
  }
  return pack(ys);
}

BitPowerset naive_flip_bit(BitPowerset b, int bit) {
  auto xs = unpack(b);
  set<int> ys;
  for (int x : xs) {
    ys.insert(x ^ ONE<<bit);
  }
  return pack(ys);
}


// bool can_have(BitPowerset b, int index, int value) {
//   assert(value == 0 || value == 1);

// }


void test_bitpowersets() {
  assert(pack(unpack(42)) == 42);

  assert(unpack(42) == set<int>({1, 3, 5}));

  assert(unpack(naive_set_bit(42, 0)) == set<int>({}));
  assert(unpack(naive_set_bit(42, 1)) == set<int>({7, 3}));

  assert(unpack(naive_clear_bit(42, 1)) == set<int>({1}));
  assert(unpack(naive_clear_bit(42, 3)) == set<int>({}));

  assert(unpack(naive_flip_bit(42, 0)) == set<int>({0, 2, 4}));
  assert(unpack(naive_flip_bit(42, 1)) == set<int>({1, 3, 7}));
}

#endif
