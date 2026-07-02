# Work 2 — Building and Using an Index File

Binary search needs sorted data, but physically sorting a file of large 300-byte
records is expensive. This program sorts a **small index** instead:

1. Read every record of `zipcodes.dat` and build an in-memory array of index entries
   `{ zip_code, byte_offset }`.
2. Sort the index by ZIP code with `qsort`.
3. Write the sorted index to `index.dat`.
4. Binary search the index file for the requested ZIP code.
5. Use the stored byte offset to `fseek` straight to the full record in `zipcodes.dat`.

Each index entry is a few bytes versus 300 for a full record, so sorting and
searching the index is far cheaper than sorting the data file itself — and the
original data file never has to move.

## Build & run

```bash
gcc indexed_search.c -o indexed_search
./indexed_search 01310100
```

Reads `zipcodes.dat`, creates/overwrites `index.dat`, then prints the matching
record. See the [repository README](../../README.md) for the record layout.
