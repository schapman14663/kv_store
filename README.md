# kv_store
A Key-Value Store written in C

Expected features:

- [ x ] Store string Key-Value pairs in a hash table. 
- [ x ] Insert and update entries with `kv_put`.
- [ x ]Look up values by key with `kv_get`.
- [ x ] Delete entries by key with `kv_delete`.
- [ x ] Manage all memory correctly with no leaks.
- [ x ] Handle hash collisions with linear probing.
- [ x ] Handle deletions correctly using tombstones.

Additionally:

- [ x ] Automatically reallocates it size when near full
capacity.

