# Work 1 — Sequential vs. Binary Search

Two programs that locate an address record by ZIP code in a large binary file of
fixed-length (300-byte) records, and report how many disk reads each approach
needs. The point is to make the performance gap tangible.

| Program | Input file | Requires sorted data | Reads to find a key |
|---------|------------|----------------------|---------------------|
| `sequential_search.c` | `zipcodes.dat` | No | up to *N* |
| `binary_search.c` | `zipcodes_sorted.dat` | Yes (sorted by ZIP) | about log₂(*N*) |

On a file with ~700,000 records, sequential search may perform hundreds of
thousands of reads, while binary search finds (or rules out) any key in ~20.

## Build & run

```bash
gcc sequential_search.c -o sequential_search
./sequential_search 01310100

gcc binary_search.c -o binary_search
./binary_search 01310100
```

`binary_search` expects a file already sorted by ZIP code — produce one with
[Work 3 (External Merge Sort)](../3-external-merge-sort/). See the
[repository README](../../README.md) for the record layout.
