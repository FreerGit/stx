#include "mem.h"
#include "stx.h"

#ifndef DYN_ARRAY_H
#define DYN_ARRAY_H

// onst size_t new_size = sizeof(ListPrelude) + new_capacity * item_size;

// prelude = prelude->allocator->realloc(prelude, new_size);

typedef struct {
  size_t length;
  size_t capacity;
  Allocator *a;
} __attribute__((aligned(32))) Array_Header;

// We align to move from 24 to 32 bytes, keeping power-of-two to make CPU happy.
static_assert(sizeof(Array_Header) == 32);

#define ARRAY_INITIAL_CAPACITY 16

#define array(T, a) array_init(sizeof(T), ARRAY_INITIAL_CAPACITY, a)

void *array_init(size_t item_size, size_t capacity, Allocator *a) {
  void *ptr = 0;
  size_t size = item_size * capacity + sizeof(Array_Header);

  // TODO(imp) Alingment?
  Array_Header *h = a->alloc(a, size);

  if (h) {
    h->length = 0;
    h->capacity = capacity;
    h->a = a;
    ptr = h + 1;
  }

  return ptr;
}

#define array_append(a, v) (                      \
    (a) = array_ensure_capacity(a, 1, sizeof(v)), \
    (a)[array_header(a)->length] = (v),           \
    &(a)[array_header(a)->length++])

#endif