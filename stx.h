
#include <assert.h>
#include <stddef.h>
#include <stdint.h>

/*
//  I will keep this here as a lookup for my own sake, C primitives are hard :(
typedef uint8_t u8;
typedef int32_t b32;
typedef int32_t i32;
typedef uint32_t u32;
typedef uint64_t u64;
typedef float f32;
typedef double f64;
typedef uintptr_t uptr;
typedef char byte;
typedef ptrdiff_t size;
typedef size_t usize;
*/

// This is interesting, can be used in many ways for optimizations.
// Leaving this here because I don't _fully_ understand the implications.
// #define assert(c) while (!(c)) __builtin_unreachable()

#define count_of(...) (ptrdiff_t)(sizeof(__VA_ARGS__) / sizeof(*__VA_ARGS__))
#define len(s) (count_of(s) - 1)
#define new(a, t, n) (t *)alloc(a, sizeof(t), n)
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

#define memset(d, c, n) __builtin_memset(d, c, n)
#define memcpy(d, s, n) __builtin_memcpy(d, s, n)
#define memcmp(a, b, n) __builtin_memcmp(a, b, n)

// Arena stuff
// typedef struct Arena {
//   char *buf;
//   size_t len;
//   size_t off;
// } Arena;

// void *alloc(Arena *arena, size_t len) {
//   len = (len + 7) & ~7;
//   if (arena->len - arena->off < len) {
//     return 0;
//   }
//   void *p = arena->buf + arena->off;
//   arena->off += len;
//   memset(p, 0, len);
//   return p;
// }

// Reject null-terminated strings, embrace modernity.
// UTF-8 assumed string.
#define str_t(s) (str_t)(uint8_t *) s, LEN(s)

typedef struct {
  uint8_t *buf;
  ptrdiff_t len;
} str_t;

// static str_t str_clone(Arena *a, str_t s) {
//   str_t c = {0};
//   c.buf = new (a, uint8_t, s.len);
//   c.len = s.len;
//   if (s.len) {
//     memcpy(c.buf, s.buf, s.len);
//   }
//   return c;
// }

static str_t str_copy(str_t dst, str_t src) {
  assert(dst.len >= src.len);
  for (; src.len; src.len--) {
    *dst.buf++ = *src.buf++;
  }
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

// TODO https://nullprogram.com/blog/2023/10/05/