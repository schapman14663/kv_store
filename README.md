# kv_store
A Key-Value Store written in C

Expected features:

- Store string Key-Value pairs in a hash table. 
- Insert and update entries with `kv_put`.
- Look up values by key with `kv_get`.
- Delete entries by key with `kv_delete`.
- Manage all memory correctly with no leaks.
- Handle hash collisions with linear probing.
- Handle deletions correctly using tombstones.
