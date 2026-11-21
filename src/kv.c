#include <kv.h>
#include <stdlib.h>
#include <string.h>

static const char TOMBSTONE;
// fn hash
// params:
//   - val: character pointer for a value we
//   want to hash
//   - capacity: the size of the database
// returns: a number that is smaller than the
// size of the database
// TODO: Change how hashing function works
size_t hash(const char *val, size_t capacity) {
  size_t hash = 0x01928465deadbeef;

  while (*val) {

    hash ^= *val;
    hash = hash << 8;
    hash += *val;

    val++;
  }
  return hash % capacity;
}

// fn kv_resize
//  params:
//  - database: the database we need to resize
// returns: a database
static kv_t *kv_resize(kv_t *db) {

  kv_entry_t *old_table = db->entries;
  size_t old_cap = db->capacity;

  // Probably room to fine tune this, but generally want
  // to not double the store unneccesasrily.
  // TODO: fine tune capacity fluctuation, store should
  // have room to shrink if needed.
  db->capacity =
      (db->count > db->tombstone_count) ? (old_cap * 2) : (old_cap + 10);
  db->entries = calloc(db->capacity, sizeof(kv_entry_t));
  if (db->entries == NULL) {
    db->entries = old_table;
    db->capacity = old_cap;
    return NULL;
  }
  db->count = 0;
  db->tombstone_count = 0;

  for (size_t i = 0; i < old_cap; i++) {

    kv_entry_t *entry = &old_table[i];

    // make sure we are only rehashing actual entries
    if (entry->key && entry->key != &TOMBSTONE) {
      size_t new_idx = hash(entry->key, db->capacity);
      for (size_t j = 0; j < db->capacity; j++) {
        size_t realloc_idx = (new_idx + j) % db->capacity;

        kv_entry_t *new_entry = &db->entries[realloc_idx];

        if (new_entry->key == NULL) {
          new_entry->key = entry->key;
          new_entry->value = entry->value;
          db->count++;
          break;
        }
      }
    }
  }
  free(old_table);
  return db;
}

// fn kv_put
// params:
//   - db: a pointer to the database
//   - key: a pointer to the key value
//   - value: a pointer to the value itself
// returns: 0 on success
// returns -1, returns -2 on full database.
int kv_put(kv_t *db, const char *key, const char *value) {
  if (!db || !key || !value)
    return -1;

  // resize database when approaching max capacity
  if ((db->count + db->tombstone_count) >= 0.9 * db->capacity) {
    if (!kv_resize(db)) {
      return -1;
    }
  }

  size_t idx = hash(key, db->capacity);

  kv_entry_t *target = NULL;

  for (size_t i = 0; i < db->capacity; i++) {
    size_t real_idx = (idx + i) % db->capacity;

    kv_entry_t *entry = &db->entries[real_idx];

    // if a tombstone is found before a key, take a note of it and
    // keep probing.
    if (entry->key == &TOMBSTONE && !target) {
      target = entry;
    }

    // if key already set, update value
    if (entry->key && entry->key != &TOMBSTONE && !strcmp(entry->key, key)) {
      char *new_value = strdup(value);
      if (!new_value)
        return -1;
      free(entry->value);
      entry->value = new_value;
      return 0;
    }

    // arrive in slot that is empty
    if (!entry->key) {
      if (!target) {
        target = entry;
      }
      break;
    }
  }

  if (target) {
    char *new_key = strdup(key);
    char *new_value = strdup(value);
    if (!new_key || !new_value) {
      free(new_key);
      free(new_value);
      return -1;
    }
    if (target->key == (char *)&TOMBSTONE) {
      db->tombstone_count--;
    }
    target->key = new_key;
    target->value = new_value;
    db->count++;
    return 0;
  }
  // database is full
  return -2;
}

// fn kv_get
// params:
//  - database: the database we need the pair from.
//  - key: the key for the kv pair we're looking for.
// returns: the value associated with the key.
char *kv_get(kv_t *db, const char *key) {
  if (!db || !key)
    return NULL;

  size_t start_idx = hash(key, db->capacity);

  for (size_t i = 0; i < db->capacity; i++) {

    size_t real_idx = (start_idx + i) % db->capacity;

    kv_entry_t *entry = &db->entries[real_idx];

    if (entry->key && entry->key != &TOMBSTONE && !strcmp(entry->key, key)) {
      return entry->value;
    }

    if (entry->key == NULL) {
      return NULL;
    }
  }
  return NULL;
}

// fn kv_delete
// params:
//  - database: the database we are interacting with
//  - key: the key to delete
// returns: confirmation that the entry has been deleted
// and a tombstone has been created in its place.
int kv_delete(kv_t *db, const char *key) {
  if (!db || !key)
    return -1;

  size_t start_idx = hash(key, db->capacity);

  for (size_t i = 0; i < db->capacity; i++) {

    size_t real_idx = (start_idx + i) % db->capacity;
    kv_entry_t *entry = &db->entries[real_idx];

    if (entry->key && entry->key != &TOMBSTONE && !strcmp(entry->key, key)) {
      free(entry->key);
      free(entry->value);
      db->count--;
      db->tombstone_count++;
      entry->key = (char *)&TOMBSTONE;
      entry->value = NULL;
      return 0;
    }

    if (!entry->key) {
      return -1;
    }
  }
  return -1;
}

// fn kv_free
// params:
//   - database: the database being deleted
// returns: confirmation of deletion of table
int kv_free(kv_t *db) {
  if (!db)
    return -1;

  for (size_t i = 0; i < db->capacity; i++) {

    kv_entry_t *entry = &db->entries[i];

    if (entry->key && entry->key != &TOMBSTONE) {
      free(entry->value);
      free(entry->key);
      entry->value = NULL;
      entry->key = NULL;
      db->count--;
    }
  }
  free(db->entries);
  free(db);
  return 0;
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
  table->tombstone_count = 0;

  table->entries = calloc(capacity, sizeof(kv_entry_t));
  if (table->entries == NULL) {
    free(table);
    return NULL;
  }
  return table;
}
