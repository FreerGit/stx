
#include "../stx/mem.h"
#include "../stx/stx.h"

// int main(int argc, char **argv) {
// }

#include <stddef.h>
#include <stdint.h>

// #include <assert.h>
#include <stdio.h>
// #include <string.h>

// int main() {
//   // CHECK_TIME(

//   //     {
//   int i;

//   // unsigned char backing_buffer[256];
//   // Arena a = {};
//   // arena_init(&a, backing_buffer, len(backing_buffer));

//   bool a;
//   bool b;
//   // for (i = 0; i < 1000000; i++) {
//   CHECK_TIME({
//     for (i = 0; i < 1000000; i++) {

//       a = is_power_of_two(64);
//     }
//   });

//   CHECK_TIME({
//     for (i = 0; i < 1000000; i++) {

//       b = is_power_of_t(64);
//     }
//   });

//   // int *x;
//   // float *f;
//   // str_t str = str_new_static("Hellope");

//   // Reset all arena offsets for each loop
//   // arena_free_all(&a);

//   // x = (int *)arena_alloc(&a, sizeof(int));
//   // f = (float *)arena_alloc(&a, sizeof(float));
//   // str = arena_alloc(&a, 10);

//   // *x = 123;
//   // *f = 987;
//   // str_t nstr = {"            ", len("            ")};
//   // str_t nstr = str_t("AAAA");
//   // str_t xx = str_t("body");
//   // str_t abc = {};
//   // str_t y = str_copy(nstr, xx);
//   // // str_t a = str_t("hej");
//   // printf("%td: %s\n", nstr.len, nstr.buf);

//   // printf("%d\n", str_equal(str_t("body"), str_t("body")));
//   // memcpy(str, "Hellope", 7);

//   // printf("%p: %d\n", x, *x);
//   // printf("%p: %f\n", f, *f);

//   // str = arena_resize(&a, str, 10, 16);
//   // memcpy(str + 7, " world!", 7);
//   // printf("%p: %s\n", str, str);
//   // }

//   // arena_free_all(&a);
//   // }

//   //   )

//   return 0;
// }

static int global_total_tests;
static int global_failed_tests;

#define ASSERT_TRUE(expression)                                      \
  ++global_total_tests;                                              \
  if (!(expression)) {                                               \
    ++global_failed_tests;                                           \
    printf("%s(%d): expression assert fail.\n", __FILE__, __LINE__); \
  }

void pool_test(void) {
  unsigned char backing_buffer[1024];
  Pool p;
  uint64_t *a, *b, *c, *d, *e, *f;
  pool_init(&p, backing_buffer, 1024, 64, DEFAULT_ALIGNMENT);

  a = pool_alloc(&p);
  *a = 5;
  b = pool_alloc(&p);
  c = pool_alloc(&p);
  d = pool_alloc(&p);
  e = pool_alloc(&p);
  f = pool_alloc(&p);

  pool_free(&p, f);
  pool_free(&p, c);
  pool_free(&p, b);
  pool_free(&p, d);

  d = pool_alloc(&p);

  ASSERT_TRUE(*a == 5);
  pool_free(&p, a);

  a = pool_alloc(&p);

  pool_free(&p, e);
  pool_free(&p, a);
  pool_free(&p, d);
}

void arena_test(void) {
  int i;

  unsigned char backing_buffer[256];
  Arena a = {0};
  arena_init(&a, backing_buffer, 256);

  for (i = 0; i < 10; i++) {
    int *x;
    uint64_t *f;
    char *str;

    // Reset all arena offsets for each loop
    arena_free_all(&a);

    x = (int *)arena_alloc(&a, sizeof(int));
    f = (uint64_t *)arena_alloc(&a, sizeof(float));
    str = arena_alloc(&a, 10);

    *x = 123;
    *f = 987;
    memmove(str, "Hellope", 7);
    ASSERT_TRUE(*x == 123)
    ASSERT_TRUE(*f == 987)

    str = arena_resize(&a, str, 10, 16);
    memmove(str + 7, " world!", 7);
    ASSERT_TRUE(memeql(str, "Hellope world!", 15));
  }

  arena_free_all(&a);
}

int main() {
  int result = (global_failed_tests != 0);

  pool_test();
  arena_test();

  printf("Unit Tests %s: %d/%d passed.\n",
         result ? "Failed" : "Successful",
         global_total_tests - global_failed_tests,
         global_total_tests);
}
