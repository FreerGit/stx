#include "stx.h"

#ifndef MEM_H
#define MEM_H

#include <assert.h>
#include <stddef.h>

// ------------------------------------------------------------- //
//                          Helpers
// ------------------------------------------------------------- //

inline bool is_power_of_two(uintptr_t x) {
  return (x & (x - 1)) == 0;
}

size_t align_forward_size(size_t ptr, size_t align) {
  size_t p, a, modulo;

  assert(is_power_of_two((uintptr_t)align));

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

uintptr_t align_forward_uintptr(uintptr_t ptr, uintptr_t align) {
  uintptr_t a, p, modulo;

  assert(is_power_of_two(align));

  a = align;
  p = ptr;
  modulo = p & (a - 1);
  if (modulo != 0) {
    p += a - modulo;
  }
  return p;
}

// ------------------------------------------------------------- //
//                    Allocator Interface
// ------------------------------------------------------------- //

// TODO(imp) Auto-alignment? Pass alignment? Seperate procedure?
typedef struct {
  void *(*alloc)(void *allocator, size_t bytes);
  void (*free)(void *allocator, void *ptr);
  void (*free_all)(void *allocator);
  void *allocator;
} Allocator;

#define allocator_alloc(T, n, a) ((T *)((a).alloc((a).allocator, sizeof(T) * n)))
#define allocator_free(p, a) ((a).free((a).allocator, p))
#define allocator_free_all(a) ((a).free((a).allocator))

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

#define arena_alloc_init(a) \
  (Allocator) { arena_alloc, arena_free, arena_free_all, a }

Arena arena_init(void *backing_buffer, size_t backing_buffer_length) {
  return (Arena){.buf = (unsigned char *)backing_buffer, .buf_len = backing_buffer_length};
}

void *arena_alloc_align(Arena *a, size_t size, size_t align) {
  // Align 'curr_offset' forward to the specified alignment
  uintptr_t curr_ptr = (uintptr_t)a->buf + (uintptr_t)a->curr_offset;
  uintptr_t offset = align_forward_uintptr(curr_ptr, align);
  offset -= (uintptr_t)a->buf; // Change to relative offset

  // Check to see if the backing memory has space left
  if (offset + size <= a->buf_len) {
    void *ptr = &a->buf[offset];
    a->prev_offset = offset;
    a->curr_offset = offset + size;

    return ptr;
  }
  // Escape early, will probably change this.
  assert(0 && "Memory is out of bounds of the buffer in this arena");
  return NULL;
}

// Because C doesn't have default parameters
void *arena_alloc(void *a, size_t size) {
  return arena_alloc_align((Arena *)a, size, DEFAULT_ALIGNMENT);
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

// Does nothing.
void arena_free(void *allocator, void *ptr) {
  (void)allocator;
  (void)ptr;
}

void arena_free_all(void *allocator) {
  Arena *a = allocator;
  a->curr_offset = 0;
  a->prev_offset = 0;
}

typedef struct {
  Arena *arena;
  size_t prev_offset;
  size_t curr_offset;
} Temp_Arena_Memory;

Temp_Arena_Memory temp_arena_memory_begin(Arena *a) {
  return (Temp_Arena_Memory){.prev_offset = a->prev_offset, .curr_offset = a->curr_offset};
}

void temp_arena_memory_end(Temp_Arena_Memory temp) {
  temp.arena->prev_offset = temp.prev_offset;
  temp.arena->curr_offset = temp.curr_offset;
}

// ------------------------------------------------------------- //
//                Pool Allocator (Block Allocator)
// ------------------------------------------------------------- //

typedef struct Pool_Free_Node Pool_Free_Node;
struct Pool_Free_Node {
  Pool_Free_Node *next;
};

typedef struct {
  unsigned char *buf;
  size_t buf_len;
  size_t chunk_size;

  Pool_Free_Node *head; // Free List Head
} Pool;

void pool_free_all(Pool *p);

void pool_init(Pool *p, void *backing_buffer, size_t backing_buffer_length,
               size_t chunk_size, size_t chunk_alignment) {
  // Align backing buffer to the specified chunk alignment
  uintptr_t initial_start = (uintptr_t)backing_buffer;
  uintptr_t start = align_forward_uintptr(initial_start, (uintptr_t)chunk_alignment);
  backing_buffer_length -= (size_t)(start - initial_start);

  // Align chunk size up to the required chunk_alignment
  chunk_size = align_forward_size(chunk_size, chunk_alignment);

  // Assert that the parameters passed are valid
  assert(chunk_size >= sizeof(Pool_Free_Node) &&
         "Chunk size is too small");
  assert(backing_buffer_length >= chunk_size &&
         "Backing buffer length is smaller than the chunk size");

  // Store the adjusted parameters
  p->buf = (unsigned char *)backing_buffer;
  p->buf_len = backing_buffer_length;
  p->chunk_size = chunk_size;
  p->head = NULL; // Free List Head

  // Set up the free list for free chunks
  pool_free_all(p);
}

void *pool_alloc(Pool *p) {
  // Get latest free node
  Pool_Free_Node *node = p->head;

  if (node == NULL) {
    assert(0 && "Pool allocator has no free memory");
    return NULL;
  }

  // Pop free node
  p->head = p->head->next;

  // Zero memory by default
  return memset(node, 0, p->chunk_size);
}

void pool_free(Pool *p, void *ptr) {
  Pool_Free_Node *node;

  void *start = p->buf;
  void *end = &p->buf[p->buf_len];

  if (ptr == NULL) {
    // Ignore NULL pointers
    return;
  }

  if (!(start <= ptr && ptr < end)) {
    assert(0 && "Memory is out of bounds of the buffer in this pool");
    return;
  }

  // Push free node
  node = (Pool_Free_Node *)ptr;
  node->next = p->head;
  p->head = node;
}

void pool_free_all(Pool *p) {
  size_t chunk_count = p->buf_len / p->chunk_size;
  size_t i;

  // Set all chunks to be free
  for (i = 0; i < chunk_count; i++) {
    void *ptr = &p->buf[i * p->chunk_size];
    Pool_Free_Node *node = (Pool_Free_Node *)ptr;
    // Push free node onto thte free list
    node->next = p->head;
    p->head = node;
  }
}

#endif