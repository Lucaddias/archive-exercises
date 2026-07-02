# File Structures & Organization — Coursework in C

A collection of low-level **C** programs that manipulate large, fixed-length
binary files on disk and implement classic external-memory algorithms:
sequential and binary search, index files, external merge sort, streaming CSV
parsing, and a disk-based B-Tree.

The recurring dataset is a database of Brazilian address records (street,
neighborhood, city, state, and ZIP code — *CEP* in Portuguese), stored as
fixed-length **300-byte** records. The exercises focus on what changes when the
data is far bigger than memory and every disk access counts.

## Repository structure

The five assignments are grouped by exam term — **P1** (assignments 1–3) and
**P2** (assignments 4–5):

```
.
├── P1/
│   ├── 1-sequential-vs-binary-search/   # sequential vs. binary search on a record file
│   ├── 2-indexed-search/                # build a sorted index file and search it
│   └── 3-external-merge-sort/           # K-block external merge sort
└── P2/
    ├── 4-covid-csv-analysis/            # streaming CSV parser + data aggregation
    └── 5-btree-index/                   # disk-based B-Tree index and join
```

## Assignments

| # | Term | Project | Key idea |
|---|------|---------|----------|
| 1 | P1 | [Sequential vs. Binary Search](P1/1-sequential-vs-binary-search/) | Compare *O(N)* scanning against *O(log N)* `fseek`-based binary search on a sorted file. |
| 2 | P1 | [Indexed Search](P1/2-indexed-search/) | Sort a small `{ zip, offset }` index instead of the large data file, then search the index. |
| 3 | P1 | [External Merge Sort](P1/3-external-merge-sort/) | Sort a file larger than memory in three phases: split → merge → result. |
| 4 | P2 | [COVID-19 CSV Analysis](P2/4-covid-csv-analysis/) | Parse a huge CSV with a constant-memory, table-driven state machine and aggregate per country. |
| 5 | P2 | [B-Tree Index & Join](P2/5-btree-index/) | Build a disk-based B-Tree index and use it to compute the intersection of two samples. |

Each folder has its own README with details and exact build/run commands.

## The record format

Assignments 1–3 and 5 operate on fixed-length 300-byte address records:

```c
typedef struct {
    char street[72];        /* street address                     */
    char neighborhood[72];  /* district                           */
    char city[72];          /* city                               */
    char state[72];         /* full state name                    */
    char state_code[2];     /* 2-letter state code (UF)           */
    char zip_code[8];       /* ZIP code (CEP) — the search key     */
    char padding[2];        /* keeps every record exactly 300 B    */
} ZipRecord;                /* total: 300 bytes                    */
```

The 8-byte ZIP code (`zip_code`) sits at byte offset 290 within each record and
is the key used by every search, sort, and index in this repository.

## Building

Everything is plain C (C11) with no external dependencies. Any modern compiler
works — `gcc` or `clang`:

```bash
# single-file programs
gcc <program>.c -o <program>

# multi-file programs (see each folder's README)
gcc covid_analysis.c csv_parser.c -o covid_analysis -lm
gcc main.c sampling.c build_index.c join.c btree.c -o btree_join
```

## Data files

The `.dat` binary databases and the COVID-19 CSV are **not** committed to the
repository (they are large and are treated as build/run inputs). Each program
expects its input file in the current working directory; see the per-assignment
READMEs for the exact file names.

---

*Coursework for the File Organization course, CEFET-MG. Rewritten and documented
in English.*
