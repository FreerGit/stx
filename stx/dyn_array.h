#include "mem.h"
#include "stx.h"
#include <stddef.h>

#ifndef DYN_ARRAY_H
#define DYN_ARRAY_H

// onst size_t new_size = sizeof(ListPrelude) + new_capacity * item_size;

// prelude = prelude->allocator->realloc(prelude, new_size);

typedef struct {
  size_t len;
  size_t capacity;
  size_t __ignore; // For alignement
  Allocator *a;
} Array_Header;

// We align to move from 24 to 32 bytes, keeping power-of-two to make CPU happy.
static_assert(sizeof(Array_Header) == 32);

#define ARRAY_INITIAL_CAPACITY 16

#define array(T, a) array_init(sizeof(T), ARRAY_INITIAL_CAPACITY, a)

#define array_header(a) ((Array_Header *)(a)-1)
#define array_length(a) (array_header(a)->len)
#define array_capacity(a) (array_header(a)->capacity)

#define array_append(a, v) (                      \
    (a) = array_ensure_capacity(a, 1, sizeof(v)), \
    (a)[array_header(a)->len] = (v),              \
    &(a)[array_header(a)->len++])

#define array_remove(a, i)             \
  do {                                 \
    Array_Header *h = array_header(a); \
    if (i == h->len - 1) {             \
      h->len -= 1;                     \
    } else if (h->len > 1) {           \
      void *ptr = &a[i];               \
      void *last = &a[h->len - 1];     \
      h->len -= 1;                     \
      memcpy(ptr, last, sizeof(*a));   \
    }                                  \
  } while (0);

#define array_pop_back(a) (array_header(a)->len -= 1)

void *array_init(size_t item_size, size_t capacity, Allocator *a) {
  void *ptr = 0;
  size_t size = item_size * capacity + sizeof(Array_Header);

  // TODO(imp) Alingment?
  Array_Header *h = a->alloc(size, a->allocator);

  if (h) {
    h->len = 0;
    h->capacity = capacity;
    h->a = a;
    ptr = h + 1;
  }

  return ptr;
}

void *array_ensure_capacity(void *a, size_t item_count, size_t item_size) {
  Array_Header *h = array_header(a);
  size_t desired_capacity = h->len + item_count;

  if (h->capacity < desired_capacity) {
    size_t new_capacity = h->capacity * 2;
    while (new_capacity < desired_capacity) {
      new_capacity *= 2;
    }

    size_t new_size = sizeof(Array_Header) + new_capacity * item_size;
    Array_Header *new_h = h->a->alloc(new_size, h->a->allocator);

    if (new_h) {
      size_t old_size = sizeof(*h) + h->len * item_size;
      memcpy(new_h, h, old_size);

      if (h->a->free) {
        h->a->free(old_size, h, h->a->allocator);
      }

      new_h->capacity = new_capacity;
      h = new_h + 1;
    } else {
      h = 0;
    }
  } else {
    h += 1;
  }

  return h;
}

#endif