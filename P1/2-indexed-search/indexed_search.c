/*
 * Work 2 - Building and Using an Index File
 *
 * The data file "zipcodes.dat" is not sorted, so binary search cannot be applied
 * to it directly. Instead of physically sorting the (large) 300-byte records,
 * this program builds a small index and searches that:
 *
 *   1. Read every record from "zipcodes.dat" and build an in-memory array of
 *      index entries, each holding a ZIP code and the byte offset of its record.
 *   2. Sort the index array by ZIP code with qsort.
 *   3. Write the sorted index to "index.dat".
 *   4. Binary search the index file for the requested ZIP code.
 *   5. Use the stored offset to seek straight to the full record in "zipcodes.dat".
 *
 * Each index entry is tiny compared to a full record, so sorting and searching
 * the index is far cheaper than sorting the data file itself.
 *
 * Usage: ./indexed_search <ZIP_CODE>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DATA_FILE  "zipcodes.dat"
#define INDEX_FILE "index.dat"
#define ZIP_LENGTH 8

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

/* Lightweight index entry: ZIP code plus the record's byte offset. */
typedef struct {
    char zip_code[8];
    long offset;            /* byte offset of the record in the data file */
} IndexEntry;

/* Comparison used by qsort and, conceptually, by the binary search. */
static int compare_entries(const void *a, const void *b)
{
    return strncmp(((const IndexEntry *)a)->zip_code,
                   ((const IndexEntry *)b)->zip_code, ZIP_LENGTH);
}

static void print_field(const char *label, const char *text, int max_length)
{
    int length = max_length;
    while (length > 0 && (text[length - 1] == ' ' ||
                          text[length - 1] == '\n' ||
                          text[length - 1] == '\r')) {
        length--;
    }
    printf("%-14s: %.*s\n", label, length, text);
}

static void print_record(const ZipRecord *record)
{
    print_field("Street", record->street, 72);
    print_field("Neighborhood", record->neighborhood, 72);
    print_field("City", record->city, 72);
    print_field("State", record->state, 72);
    print_field("State code", record->state_code, 2);
    print_field("ZIP code", record->zip_code, ZIP_LENGTH);
}

/*
 * Step 1-3: build the index from the data file, sort it, and save it to disk.
 * Returns the in-memory index array (caller frees it) or NULL on error.
 */
static IndexEntry *build_index(FILE *data_file, long record_count,
                               const char *index_path)
{
    IndexEntry *index = malloc(record_count * sizeof(IndexEntry));
    if (index == NULL) {
        fprintf(stderr, "Error: not enough memory for the index.\n");
        return NULL;
    }

    rewind(data_file);
    ZipRecord record;
    long offset = 0;

    for (long i = 0; fread(&record, sizeof(ZipRecord), 1, data_file) == 1; i++) {
        strncpy(index[i].zip_code, record.zip_code, ZIP_LENGTH);
        index[i].offset = offset;
        offset += (long)sizeof(ZipRecord);
    }

    qsort(index, record_count, sizeof(IndexEntry), compare_entries);

    FILE *index_file = fopen(index_path, "wb");
    if (index_file == NULL) {
        fprintf(stderr, "Error: could not create '%s'.\n", index_path);
        free(index);
        return NULL;
    }
    fwrite(index, sizeof(IndexEntry), record_count, index_file);
    fclose(index_file);

    printf("Index created: '%s' (%ld entries).\n\n", index_path, record_count);
    return index;
}

/*
 * Step 4: binary search the index file for target_zip.
 * Returns the record's byte offset in the data file, or -1 if not found.
 */
static long search_index(FILE *index_file, long entry_count,
                         const char *target_zip, int *reads)
{
    long low = 0;
    long high = entry_count - 1;
    *reads = 0;
    IndexEntry entry;

    while (low <= high) {
        long mid = low + (high - low) / 2;
        (*reads)++;

        fseek(index_file, mid * (long)sizeof(IndexEntry), SEEK_SET);
        if (fread(&entry, sizeof(IndexEntry), 1, index_file) != 1) {
            break;
        }

        int comparison = strncmp(target_zip, entry.zip_code, ZIP_LENGTH);
        if (comparison == 0) {
            return entry.offset;
        } else if (comparison < 0) {
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }
    return -1L;
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <ZIP_CODE>\n", argv[0]);
        fprintf(stderr, "Example: %s 01310100\n", argv[0]);
        return 1;
    }

    FILE *data_file = fopen(DATA_FILE, "rb");
    if (data_file == NULL) {
        fprintf(stderr, "Error: could not open '%s'.\n", DATA_FILE);
        return 1;
    }

    fseek(data_file, 0, SEEK_END);
    long record_count = ftell(data_file) / (long)sizeof(ZipRecord);
    rewind(data_file);

    printf("Record size: %zu bytes\n", sizeof(ZipRecord));
    printf("Total records: %ld\n\n", record_count);

    /* Build and persist the sorted index. */
    IndexEntry *index = build_index(data_file, record_count, INDEX_FILE);
    if (index == NULL) {
        fclose(data_file);
        return 1;
    }

    /* Reopen the index file to binary search it. */
    FILE *index_file = fopen(INDEX_FILE, "rb");
    if (index_file == NULL) {
        fprintf(stderr, "Error: could not open '%s'.\n", INDEX_FILE);
        free(index);
        fclose(data_file);
        return 1;
    }

    int reads = 0;
    long offset = search_index(index_file, record_count, argv[1], &reads);

    if (offset < 0) {
        printf("ZIP code '%s' not found in the index.\n", argv[1]);
    } else {
        /* Step 5: seek straight to the record using the stored offset. */
        fseek(data_file, offset, SEEK_SET);
        ZipRecord record;
        if (fread(&record, sizeof(ZipRecord), 1, data_file) == 1) {
            printf("--- RECORD FOUND (VIA INDEX) ---\n");
            print_record(&record);
            printf("--------------------------------\n");
        } else {
            printf("Error: could not read the record from the data file.\n");
        }
    }

    printf("\nIndex reads performed: %d\n", reads);

    fclose(index_file);
    fclose(data_file);
    free(index);
    return 0;
}
