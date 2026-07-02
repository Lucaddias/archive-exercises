# Work 4 — COVID-19 Data Aggregation from a Large CSV

Streams the [Our World in Data](https://github.com/owid/covid-19-data) COVID-19
dataset (`owid-covid-data.csv`) and aggregates, **per South American country**,
the total number of confirmed cases and deaths.

The file is read in fixed-size 8 KB chunks and fed to a small **table-driven CSV
parser** (a finite-state machine), so memory usage stays constant no matter how
large the file is. Because the `total_cases` / `total_deaths` columns are
cumulative, the final figure for each country is the maximum value ever seen for
it.

## Files

| File | Role |
|------|------|
| `covid_analysis.c` | Filters rows, aggregates per country, prints the report. |
| `csv_parser.c` / `csv_parser.h` | Reusable streaming CSV parser (state machine). |

## Build & run

```bash
gcc covid_analysis.c csv_parser.c -o covid_analysis -lm
./covid_analysis
```

The program expects `owid-covid-data.csv` in the working directory and prints a
per-country table plus a grand total.
