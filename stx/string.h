#ifndef STRING_H
#define STRING_H

#include "macros.h"

// ------------------------------------------------------------- //
//    Reject null-terminated strings, embrace modernity.
//                  UTF-8 assumed string.
// ------------------------------------------------------------- //

#define str_t(s) \
  (str_t) { (uint8_t *)s, count_of(s) - 1 }

typedef struct {
  uint8_t *buf;
  ptrdiff_t len;
} str_t;

// static str_t str_new(char *buf) {
//   str_t new_str = {.buf = buf, .len = len(buf)};
//   printf("in new %s, %td\n", new_str.buf, len(buf));
//   return new_str;
// }

// static str_t str_clone(Arena *a, str_t s) {
//   str_t c = {0};
//   c.buf = new (a, uint8_t, s.len);
//   c.len = s.len;
//   if (s.len) {
//     memcpy(c.buf, s.buf, s.len);
//   }
//   return c;
// }

static void u8copy(uint8_t *dst, uint8_t *src, ptrdiff_t n) {
  assert(n >= 0);
  for (; n; n--) {
    printf(".. %s\n", dst);
    // *dst++ = *src;
  }
}

static str_t str_copy(str_t dst, str_t src) {
  assert(dst.len >= src.len);
  // for (; src.len; src.len--) {
  u8copy(dst.buf, src.buf, src.len);
  printf("%s, %td\n", dst.buf, dst.len);
  printf("%s, %td\n", src.buf, src.len);
  // }
  dst.buf += src.len;
  dst.len -= src.len;
  return dst;
}

static str_t str_span(uint8_t *beg, uint8_t *end) {
  str_t s = {0};
  s.buf = beg;
  s.len = end - beg;
  return s;
}

static uint64_t str_hash(str_t s) {
  uint64_t h = 0;
  memcpy(&h, s.buf, min(8, s.len));
  return h * 1111111111111111111u;
}

static bool str_equal(str_t a, str_t b) {
  return a.len == b.len && !memcmp(a.buf, b.buf, a.len);
}

static ptrdiff_t str_compare(str_t a, str_t b) {
  ptrdiff_t len = a.len < b.len ? a.len : b.len;
  ptrdiff_t r = len ? memcmp(a.buf, b.buf, len) : b.len - a.len;
  return r ? r : a.len - b.len;
}

#endif