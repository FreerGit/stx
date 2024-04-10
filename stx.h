
#include <assert.h>
#include <stddef.h>
#include <stdint.h>
#include <time.h>

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
typedef unsigned char byte;
typedef ptrdiff_t size;
typedef size_t usize;
*/

// This is interesting, can be used in many ways for optimizations.
// Leaving this here because I don't _fully_ understand the implications.
// #define assert(c) while (!(c)) __builtin_unreachable()

// Useful macros
#define count_of(a) (ptrdiff_t)(sizeof(a) / sizeof(*(a)))
// #define new(a, t, n) (t *)alloc(a, sizeof(t), n)
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

#define memset(d, c, n) __builtin_memset(d, c, n)
#define memcpy(d, s, n) __builtin_memcpy(d, s, n)
#define memccpy(a, b, n) __builtin_memccpy(a, b, n)
#define memmove(a, b, n) __builtin_memmove(a, b, n)
#define memcmp(a, b, n) __builtin_memcmp(a, b, n)

// Nice utility to check the timing of a function
#define CHECK_TIME(x)                                             \
  {                                                               \
    struct timespec start, end;                                   \
    timespec_get(&start, TIME_UTC);                               \
    x;                                                            \
    timespec_get(&end, TIME_UTC);                                 \
    long double f = ((double)end.tv_sec * 1e9 + end.tv_nsec) -    \
                    ((double)start.tv_sec * 1e9 + start.tv_nsec); \
    printf("time %Lf ns\n", f);                                   \
  }

// ------------------------------------------------------------- //
//            Arena Allocator (Linear Allactor)
// ------------------------------------------------------------- //

#define DEFAULT_ALIGNMENT (2 * sizeof(void *))

typedef struct {
  unsigned char *buf;
  size_t buf_len;
  size_t prev_offset;
  size_t curr_offset;
} Arena;

inline bool is_power_of_two(uintptr_t x) {
  return (x & (x - 1)) == 0;
}

uintptr_t align_forward(uintptr_t ptr, size_t align) {
  uintptr_t p, a, modulo;

  assert(is_power_of_two(align));

  p = ptr;
  a = (uintptr_t)align;
  // Same as (p % a) but faster as 'a' is a power of two
  modulo = p & (a - 1);

  if (modulo != 0) {
    // If 'p' address is not aligned, push the address to the
    // next value which is aligned
    p += a - modulo;
  }
  return p;
}

void arena_init(Arena *a, void *backing_buffer, size_t backing_buffer_length) {
  a->buf = (unsigned char *)backing_buffer;
  a->buf_len = backing_buffer_length;
  a->curr_offset = 0;
  a->prev_offset = 0;
}

void *arena_alloc_align(Arena *a, size_t size, size_t align) {
  // Align 'curr_offset' forward to the specified alignment
  uintptr_t curr_ptr = (uintptr_t)a->buf + (uintptr_t)a->curr_offset;
  uintptr_t offset = align_forward(curr_ptr, align);
  offset -= (uintptr_t)a->buf; // Change to relative offset

  // Check to see if the backing memory has space left
  if (offset + size <= a->buf_len) {
    void *ptr = &a->buf[offset];
    a->prev_offset = offset;
    a->curr_offset = offset + size;

    // Zero new memory by default
    memset(ptr, 0, size);
    return ptr;
  }
  // Escape early, will probably change this.
  assert(0 && "Memory is out of bounds of the buffer in this arena");
  return NULL;
}

// Because C doesn't have default parameters
void *arena_alloc(Arena *a, size_t size) {
  return arena_alloc_align(a, size, DEFAULT_ALIGNMENT);
}

void *arena_resize_align(Arena *a, void *old_memory, size_t old_size, size_t new_size, size_t align) {
  unsigned char *old_mem = (unsigned char *)old_memory;

  assert(is_power_of_two(align));

  if (old_mem == NULL || old_size == 0) {
    return arena_alloc_align(a, new_size, align);
  } else if (a->buf <= old_mem && old_mem < a->buf + a->buf_len) {
    if (a->buf + a->prev_offset == old_mem) {
      a->curr_offset = a->prev_offset + new_size;
      if (new_size > old_size) {
        // ZII
        memset(&a->buf[a->curr_offset], 0, new_size - old_size);
      }
      return old_memory;
    } else {
      void *new_memory = arena_alloc_align(a, new_size, align);
      size_t copy_size = old_size < new_size ? old_size : new_size;
      // Copy across old memory to the new memory
      memcpy(new_memory, old_memory, copy_size);
      return new_memory;
    }

  } else {
    assert(0 && "Memory is out of bounds of the buffer in this arena");
    return NULL;
  }
}

// Because C doesn't have default parameters
void *arena_resize(Arena *a, void *old_memory, size_t old_size, size_t new_size) {
  return arena_resize_align(a, old_memory, old_size, new_size, DEFAULT_ALIGNMENT);
}

void arena_free_all(Arena *a) {
  a->curr_offset = 0;
  a->prev_offset = 0;
}

typedef;
typedef struct {
  Arena *arena;
  size_t prev_offset;
  size_t curr_offset;
} Temp_Arena_Memory;

Temp_Arena_Memory temp_arena_memory_begin(Arena *a) {
  return (Temp_Arena_Memory){a->prev_offset, a->curr_offset};
}

void temp_arena_memory_end(Temp_Arena_Memory temp) {
  temp.arena->prev_offset = temp.prev_offset;
  temp.arena->curr_offset = temp.curr_offset;
}

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

#include <stdio.h>

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

// TODO https://nullprogram.com/blog/2023/10/05/