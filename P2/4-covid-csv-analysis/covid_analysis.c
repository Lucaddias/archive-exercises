/*
 * Work 4 - COVID-19 Data Aggregation from a Large CSV
 *
 * Streams the "owid-covid-data.csv" file (Our World in Data) through a small
 * table-driven CSV parser and aggregates, per South American country, the total
 * number of confirmed cases and deaths. The file is read in fixed-size chunks,
 * so memory usage stays constant regardless of the file size.
 *
 * The "total_cases" / "total_deaths" columns are cumulative, so the final total
 * for a country is the maximum value ever seen for it.
 *
 * Build: gcc covid_analysis.c csv_parser.c -o covid_analysis -lm
 * Run:   ./covid_analysis
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "csv_parser.h"

#define READ_BUFFER_SIZE 8192
#define MAX_COUNTRIES 20
#define TARGET_CONTINENT "South America"

/* Column indices in the Our World in Data CSV layout. */
#define COL_CONTINENT   1
#define COL_LOCATION    2
#define COL_TOTAL_CASES 4
#define COL_TOTAL_DEATHS 7
#define MIN_COLUMNS     8

typedef struct {
    char location[64];
    double total_cases;
    double total_deaths;
} CountryData;

typedef struct {
    int row_count;                        /* data rows that matched the filter */
    int header_skipped;                   /* becomes 1 after the header row     */
    CountryData countries[MAX_COUNTRIES];
    int country_count;
} Accumulator;

/* Returns the index of a country, creating a fresh entry the first time. */
static int country_index(Accumulator *acc, const char *name)
{
    for (int i = 0; i < acc->country_count; i++) {
        if (strcmp(acc->countries[i].location, name) == 0) {
            return i;
        }
    }
    int i = acc->country_count++;
    strncpy(acc->countries[i].location, name, sizeof(acc->countries[i].location) - 1);
    acc->countries[i].location[sizeof(acc->countries[i].location) - 1] = '\0';
    acc->countries[i].total_cases = 0;
    acc->countries[i].total_deaths = 0;
    return i;
}

/* Called by the CSV parser once per assembled row. */
static void on_row(char **cols, int ncols, void *user_data)
{
    Accumulator *acc = (Accumulator *)user_data;

    /* The very first row is the header: skip it. */
    if (!acc->header_skipped) {
        acc->header_skipped = 1;
        return;
    }

    if (ncols < MIN_COLUMNS) {
        return;
    }
    if (strcmp(cols[COL_CONTINENT], TARGET_CONTINENT) != 0) {
        return;
    }
    if (acc->country_count >= MAX_COUNTRIES) {
        return;
    }

    double cases = (cols[COL_TOTAL_CASES][0] != '\0') ? atof(cols[COL_TOTAL_CASES]) : 0;
    double deaths = (cols[COL_TOTAL_DEATHS][0] != '\0') ? atof(cols[COL_TOTAL_DEATHS]) : 0;

    int idx = country_index(acc, cols[COL_LOCATION]);
    if (cases > acc->countries[idx].total_cases) {
        acc->countries[idx].total_cases = cases;
    }
    if (deaths > acc->countries[idx].total_deaths) {
        acc->countries[idx].total_deaths = deaths;
    }

    acc->row_count++;
}

int main(void)
{
    Accumulator acc;
    acc.row_count = 0;
    acc.header_skipped = 0;
    acc.country_count = 0;

    char *buffer = malloc(READ_BUFFER_SIZE);
    if (buffer == NULL) {
        fprintf(stderr, "Error: not enough memory for the read buffer.\n");
        return 1;
    }

    CSVParser parser;
    CSVParser_init(&parser);

    FILE *file = fopen("owid-covid-data.csv", "rb");
    if (file == NULL) {
        fprintf(stderr, "Error: could not open 'owid-covid-data.csv'.\n");
        free(buffer);
        return 1;
    }

    int read = fread(buffer, 1, READ_BUFFER_SIZE, file);
    while (read > 0) {
        CSVParser_processLines(&parser, buffer, read, on_row, &acc);
        read = fread(buffer, 1, READ_BUFFER_SIZE, file);
    }
    fclose(file);

    /* Flush any final row not terminated by a newline. */
    CSVParser_processLines(&parser, "\n", 1, on_row, &acc);

    free(buffer);

    printf("============================================================\n");
    printf("  TOTAL CASES AND DEATHS - SOUTH AMERICA (COVID-19)\n");
    printf("============================================================\n");
    printf("%-30s %15s %15s\n", "Country", "Total cases", "Total deaths");
    printf("------------------------------------------------------------\n");

    double sum_cases = 0;
    double sum_deaths = 0;

    for (int i = 0; i < acc.country_count; i++) {
        printf("%-30s %15.0f %15.0f\n",
               acc.countries[i].location,
               acc.countries[i].total_cases,
               acc.countries[i].total_deaths);
        sum_cases += acc.countries[i].total_cases;
        sum_deaths += acc.countries[i].total_deaths;
    }

    printf("============================================================\n");
    printf("%-30s %15.0f %15.0f\n", "GRAND TOTAL", sum_cases, sum_deaths);
    printf("============================================================\n");
    printf("Data rows processed: %d\n", acc.row_count);
    printf("Countries found: %d\n", acc.country_count);

    return 0;
}
