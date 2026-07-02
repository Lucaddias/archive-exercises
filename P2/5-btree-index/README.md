# Work 5 — ZIP Code Join with a B-Tree Index

Computes the **intersection** of two data samples using a disk-based B-Tree
index. The whole thing runs in three steps, driven by `main.c`:

1. **Sampling** (`sampling.c`) — copy each record of `zipcodes.dat` into `sample1.dat`
   with 80% probability and into `sample2.dat` with 80% probability (independent
   draws), producing two partially overlapping samples.
2. **Indexing** (`build_index.c`) — insert every ZIP code of `sample1.dat` into a
   B-Tree stored in `index.dat`, together with the record's byte offset.
3. **Join** (`join.c`) — for each record of `sample2.dat`, look its ZIP code up in
   the index; records found in both samples are written to `result.dat`.

## The B-Tree

`btree.c` / `btree.h` implement a B-Tree that lives entirely in one file:

- Byte 0 holds a header pointing at the current root page.
- Every page is a fixed-size block; its byte offset in the file acts as its
  pointer. Each element stores an 8-byte key (a ZIP code) and the byte offset of
  the matching record.
- Insertion splits a full page and promotes the median key to the parent,
  growing the tree by one level when the root itself splits.

## Files

| File | Role |
|------|------|
| `main.c` | Runs the three steps in order. |
| `sampling.c` | Step 1 — generate the two random samples. |
| `build_index.c` | Step 2 — build the B-Tree index. |
| `join.c` | Step 3 — write the intersection to `result.dat`. |
| `btree.c` / `btree.h` | Disk-based B-Tree implementation. |

## Build & run

```bash
gcc main.c sampling.c build_index.c join.c btree.c -o btree_join
./btree_join
```

Reads `zipcodes.dat` and produces `sample1.dat`, `sample2.dat`, `index.dat`, and
`result.dat`. See the [repository README](../../README.md) for the record
layout.
