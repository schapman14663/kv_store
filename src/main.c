#include <kv.h>
#include <stdio.h>

int main() {
  kv_t *table = kv_init(10);
  printf("%p\n", table);
  printf("%ld\n", table->capacity);

  kv_put(table, "a", "b");
  kv_put(table, "hehe", "hoho");
  kv_put(table, "haha", "lmao");

  // kv_delete(table, "a");

  for (int i = 0; i < table->capacity; i++) {
    printf("%s : %s\n", table->entries[i].key, table->entries[i].value);
  }

  char *get = kv_get(table, "hehe");
  printf("%s\n", get);

  kv_free(table);
}
