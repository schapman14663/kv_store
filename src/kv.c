#include <kv.h>
#include <stdlib.h>
#include <string.h>

#define TOMBSTONE ((char *)0x1)
/// fn hash
// params:
//   - val: character pointer for a value we
//   want to hash
//   - capacity: the size of the database
// returns: a number that is smaller than the
// size of the database
size_t hash(char *val, int capacity) {
  size_t hash = 0x13371337deadbeef;

  while (*val) {

    hash ^= *val;
    hash = hash << 8;
    hash += *val;

    val++;
  }
  return hash % capacity;
}

// fn kv_put
// params:
//   - db: a pointer to the database
//   - key: a pointer to the key value
//   - value: a pointer to the value itself
// returns: the index of the key or on error,
// returns -1, returns -2 on full database.
int kv_put(kv_t *db, char *key, char *value) {
  if (!db || !key || !value)
    return -1;

  size_t idx = hash(key, db->capacity);

  for (int i = 0; i < db->capacity; i++) {
    size_t real_idx = (idx + i) % db->capacity;

    kv_entry_t *entry = &db->entries[real_idx];

    // if key already set, update value
    if (entry->key && entry->key != TOMBSTONE && !strcmp(entry->key, key)) {
      char *new_value = strdup(value);
      if (!new_value)
        return -1;
      free(entry->value);
      entry->value = new_value;
      return 0;
    }

    // arrive in slot that is empty
    if (!entry->key || entry->key == TOMBSTONE) {
      char *new_key = strdup(key);
      char *new_value = strdup(value);
      if (!new_key || !new_value) {
        free(new_key);
        free(new_value);
        return -1;
      }
      entry->key = new_key;
      entry->value = new_value;
      db->count++;
      return 0;
    }
  }
  // database is full  TODO: have database reallocate its own side
  return -2;
}

// fn kv_get
// params:
//  - database: the database we need the pair from.
//  - key: the key for the kv pair we're looking for.
// returns: the value associated with the key.
char *kv_get(kv_t *db, char *key) {
  size_t start_idx = hash(key, db->capacity);

  for (int i = start_idx; i < db->capacity; i++) {

    kv_entry_t *entry = &db->entries[i];

    if (entry->key && entry->key != TOMBSTONE && !strcmp(entry->key, key)) {
      return entry->value;
    }

    if (!entry->key || entry->key == TOMBSTONE) {
      if (i != db->capacity) {
        continue;
      }
    }
  }
  return NULL;
}

// fn kv_init
// params:
//   - capacity: the maximum (unused) size
//   of the database.
// returns: a database
kv_t *kv_init(size_t capacity) {
  if (capacity == 0)
    return NULL;

  kv_t *table = malloc(sizeof(kv_t));
  if (table == NULL) {
    return NULL;
  }

  table->capacity = capacity;
  table->count = 0;

  table->entries = calloc(sizeof(kv_entry_t), capacity);
  if (table->entries == NULL) {
    free(table);
    return NULL;
  }

  return table;
}
