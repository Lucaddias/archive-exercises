/*
 * Work 3 - External Merge Sort with K Blocks
 *
 * Sorts "zipcodes.dat" by ZIP code when the file is too large to hold in memory all
 * at once. The work is split into three phases:
 *
 *   Phase 1 - Split : divide the file into K blocks of roughly equal size,
 *                     sort each block in memory (qsort) and write it to disk.
 *   Phase 2 - Merge : repeatedly merge the blocks two at a time, round after
 *                     round, until a single sorted file remains.
 *   Phase 3 - Result: rename the final file to "zipcodes_sorted.dat".
 *
 * K must be a power of two (default: 4). Temporary block files are created and
 * removed automatically during the merge rounds.
 *
 * Usage: ./external_merge_sort [K]
 *        ./external_merge_sort        # K = 4
 *        ./external_merge_sort 8      # K = 8
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INPUT_FILE  "zipcodes.dat"
#define OUTPUT_FILE "zipcodes_sorted.dat"
#define DEFAULT_K   4
#define NAME_MAX_LEN 64
#define ZIP_LENGTH  8

/* Fixed-length record: exactly 300 bytes on disk. */
typedef struct {
    char street[72];
    char neighborhood[72];
    char city[72];
    char state[72];
    char state_code[2];
    char zip_code[8];
    char padding[2];
} ZipRecord;

static int compare_records(const void *a, const void *b)
{
    return strncmp(((const ZipRecord *)a)->zip_code,
                   ((const ZipRecord *)b)->zip_code, ZIP_LENGTH);
}

/* Temporary file names. */
static void block_name(char *buffer, int block)
{
    sprintf(buffer, "block_%02d.tmp", block);
}

static void merge_name(char *buffer, int round, int pair)
{
    sprintf(buffer, "merge_%02d_%02d.tmp", round, pair);
}

/*
 * Phase 1: split the input into K blocks, sort each in memory, save to disk.
 * The first (record_count % k) blocks receive one extra record so that every
 * record is placed in exactly one block.
 */
static int split_phase(FILE *input, long record_count, int k)
{
    long base = record_count / k;
    long remainder = record_count % k;

    for (int b = 0; b < k; b++) {
        long block_size = base + (b < remainder ? 1 : 0);

        ZipRecord *buffer = malloc(block_size * sizeof(ZipRecord));
        if (buffer == NULL) {
            fprintf(stderr, "Error: not enough memory for block %d.\n", b);
            return 0;
        }

        long read = (long)fread(buffer, sizeof(ZipRecord), block_size, input);
        if (read == 0) {
            free(buffer);
            break;
        }

        qsort(buffer, read, sizeof(ZipRecord), compare_records);

        char name[NAME_MAX_LEN];
        block_name(name, b);
        FILE *file = fopen(name, "wb");
        if (file == NULL) {
            fprintf(stderr, "Error: could not create '%s'.\n", name);
            free(buffer);
            return 0;
        }
        fwrite(buffer, sizeof(ZipRecord), read, file);
        fclose(file);
        free(buffer);

        printf("  Block %d created: '%s' (%ld records).\n", b, name, read);
    }
    return 1;
}

/* Merges two sorted files into a single sorted output file. */
static void merge_files(FILE *a, FILE *b, FILE *output)
{
    ZipRecord ra, rb;
    int has_a = (fread(&ra, sizeof(ZipRecord), 1, a) == 1);
    int has_b = (fread(&rb, sizeof(ZipRecord), 1, b) == 1);

    while (has_a || has_b) {
        ZipRecord *chosen;

        if (has_a && has_b) {
            chosen = (strncmp(ra.zip_code, rb.zip_code, ZIP_LENGTH) <= 0) ? &ra : &rb;
        } else {
            chosen = has_a ? &ra : &rb;
        }

        fwrite(chosen, sizeof(ZipRecord), 1, output);

        /* Advance only the stream whose record was just written. */
        if (chosen == &ra) {
            has_a = (fread(&ra, sizeof(ZipRecord), 1, a) == 1);
        } else {
            has_b = (fread(&rb, sizeof(ZipRecord), 1, b) == 1);
        }
    }
}

/*
 * Phase 2: merge blocks two at a time across successive rounds.
 * On return, final_name holds the name of the single remaining sorted file.
 * final_name must have room for at least NAME_MAX_LEN bytes.
 */
static int merge_phase(int k, char *final_name)
{
    int active = k;   /* files still in play this round */
    int round = 0;

    while (active > 1) {
        int produced = 0;   /* files carried into the next round */

        for (int pair = 0; pair < active; pair += 2) {
            char name_a[NAME_MAX_LEN], name_b[NAME_MAX_LEN], name_out[NAME_MAX_LEN];

            /* Input file names depend on whether this is the first round. */
            if (round == 0) {
                block_name(name_a, pair);
                block_name(name_b, pair + 1);
            } else {
                merge_name(name_a, round - 1, pair);
                merge_name(name_b, round - 1, pair + 1);
            }
            merge_name(name_out, round, produced);

            /* Lone file without a partner: promote it to the next round. */
            if (pair + 1 >= active) {
                rename(name_a, name_out);
                produced++;
                continue;
            }

            FILE *a = fopen(name_a, "rb");
            FILE *b = fopen(name_b, "rb");
            FILE *out = fopen(name_out, "wb");

            if (a == NULL || b == NULL || out == NULL) {
                fprintf(stderr, "Error opening files in round %d pair %d.\n",
                        round, pair);
                if (a) fclose(a);
                if (b) fclose(b);
                if (out) fclose(out);
                return 0;
            }

            merge_files(a, b, out);

            fclose(a);
            fclose(b);
            fclose(out);

            /* Remove the two inputs now that they are merged. */
            remove(name_a);
            remove(name_b);

            printf("  Round %d: '%s' + '%s' -> '%s'\n",
                   round, name_a, name_b, name_out);
            produced++;
        }

        active = produced;
        round++;
    }

    /* The single remaining file is the sorted result. */
    if (round == 0) {
        block_name(final_name, 0);           /* K == 1, no merge happened */
    } else {
        merge_name(final_name, round - 1, 0);
    }
    return 1;
}

int main(int argc, char **argv)
{
    int k = DEFAULT_K;
    if (argc == 2) {
        k = atoi(argv[1]);
        if (k < 2 || (k & (k - 1)) != 0) {
            fprintf(stderr, "K must be a power of two greater than 1.\n");
            return 1;
        }
    }

    printf("=== External Merge Sort (K = %d) ===\n\n", k);

    FILE *input = fopen(INPUT_FILE, "rb");
    if (input == NULL) {
        fprintf(stderr, "Error: could not open '%s'.\n", INPUT_FILE);
        return 1;
    }

    fseek(input, 0, SEEK_END);
    long record_count = ftell(input) / (long)sizeof(ZipRecord);
    rewind(input);

    printf("File        : %s\n", INPUT_FILE);
    printf("Records     : %ld\n", record_count);
    printf("Record size : %zu bytes\n\n", sizeof(ZipRecord));

    /* Phase 1 */
    printf("-- Phase 1: split and in-memory sort --\n");
    if (!split_phase(input, record_count, k)) {
        fclose(input);
        return 1;
    }
    fclose(input);

    /* Phase 2 */
    printf("\n-- Phase 2: merge --\n");
    char final_name[NAME_MAX_LEN];
    if (!merge_phase(k, final_name)) {
        return 1;
    }

    /* Phase 3: publish the result. */
    remove(OUTPUT_FILE);
    if (rename(final_name, OUTPUT_FILE) != 0) {
        fprintf(stderr, "Warning: could not rename to '%s'.\n", OUTPUT_FILE);
        printf("Final file: '%s'\n", final_name);
    } else {
        printf("\nSorted file saved as '%s'.\n", OUTPUT_FILE);
    }

    printf("Sorting finished successfully.\n");
    return 0;
}
