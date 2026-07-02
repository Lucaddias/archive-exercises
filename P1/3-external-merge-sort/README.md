# Work 3 — External Merge Sort with K Blocks

Sorts `zipcodes.dat` by ZIP code when the file is too large to fit in memory all at
once. Only one block is ever held in RAM.

| Phase | What happens |
|-------|--------------|
| **1 — Split** | Divide the file into *K* blocks of roughly equal size, sort each block in memory (`qsort`), and write it to a temporary file. |
| **2 — Merge** | Merge the blocks two at a time, round after round, until a single sorted file remains. |
| **3 — Result** | Rename the final file to `zipcodes_sorted.dat`. |

*K* must be a power of two (default: 4). Temporary `block_*.tmp` and `merge_*.tmp`
files are created and cleaned up automatically during the merge rounds.

## Build & run

```bash
gcc external_merge_sort.c -o external_merge_sort
./external_merge_sort        # K = 4
./external_merge_sort 8      # K = 8
```

Reads `zipcodes.dat` and produces `zipcodes_sorted.dat`, which
[Work 1's binary search](../1-sequential-vs-binary-search/) can then use.
See the [repository README](../../README.md) for the record layout.
