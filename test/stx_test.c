
#include "stx.h"

// int main(int argc, char **argv) {
// }

#include <stddef.h>
#include <stdint.h>

// #include <assert.h>
#include <stdio.h>
// #include <string.h>

int main() {
  // CHECK_TIME(

  //     {
  int i;

  // unsigned char backing_buffer[256];
  // Arena a = {};
  // arena_init(&a, backing_buffer, len(backing_buffer));

  bool a;
  bool b;
  // for (i = 0; i < 1000000; i++) {
  CHECK_TIME({
    for (i = 0; i < 1000000; i++) {

      a = is_power_of_two(64);
    }
  });

  CHECK_TIME({
    for (i = 0; i < 1000000; i++) {

      b = is_power_of_t(64);
    }
  });

  // int *x;
  // float *f;
  // str_t str = str_new_static("Hellope");

  // Reset all arena offsets for each loop
  // arena_free_all(&a);

  // x = (int *)arena_alloc(&a, sizeof(int));
  // f = (float *)arena_alloc(&a, sizeof(float));
  // str = arena_alloc(&a, 10);

  // *x = 123;
  // *f = 987;
  // str_t nstr = {"            ", len("            ")};
  // str_t nstr = str_t("AAAA");
  // str_t xx = str_t("body");
  // str_t abc = {};
  // str_t y = str_copy(nstr, xx);
  // // str_t a = str_t("hej");
  // printf("%td: %s\n", nstr.len, nstr.buf);

  // printf("%d\n", str_equal(str_t("body"), str_t("body")));
  // memcpy(str, "Hellope", 7);

  // printf("%p: %d\n", x, *x);
  // printf("%p: %f\n", f, *f);

  // str = arena_resize(&a, str, 10, 16);
  // memcpy(str + 7, " world!", 7);
  // printf("%p: %s\n", str, str);
  // }

  // arena_free_all(&a);
  // }

  //   )

  return 0;
}