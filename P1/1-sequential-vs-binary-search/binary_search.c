/*
 * Work 1 - Binary Search on a Sorted Fixed-Length Record File
 *
 * Reads a binary file of address records already sorted by ZIP code
 * ("zipcodes_sorted.dat") and locates a record with binary search. Instead of
 * scanning every record, it uses fseek/fread to jump straight to the middle
 * of the current search window and halves that window on each step. On a file
 * with hundreds of thousands of records this finds (or rules out) any key in
 * about log2(N) reads, versus up to N reads for the sequential version.
 *
 * Usage: ./binary_search <ZIP_CODE>
 */

#include <stdio.h>
#include <string.h>

#define DATA_FILE "zipcodes_sorted.dat"
#define ZIP_LENGTH 8

/* Fixed-length record: exactly 300 bytes on disk. */
typedef struct {
    char street[72];        /* street address        */
    char neighborhood[72];  /* district              */
    char city[72];          /* city                  */
    char state[72];         /* full state name       */
    char state_code[2];     /* 2-letter state code    */
    char zip_code[8];       /* ZIP code (search key) */
    char padding[2];        /* keeps the record 300B */
} ZipRecord;

/* Prints a fixed-length field, trimming trailing blanks and line breaks. */
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
 * Binary search over the sorted file.
 * Returns 1 and fills *result when the key is found, 0 otherwise.
 * Stores the number of reads performed in *reads.
 */
static int binary_search(FILE *file, long record_count,
                         const char *target_zip,
                         ZipRecord *result, int *reads)
{
    long low = 0;
    long high = record_count - 1;
    *reads = 0;

    while (low <= high) {
        long mid = low + (high - low) / 2;   /* avoids long overflow */
        (*reads)++;

        fseek(file, mid * (long)sizeof(ZipRecord), SEEK_SET);
        if (fread(result, sizeof(ZipRecord), 1, file) != 1) {
            break;
        }

        int comparison = strncmp(target_zip, result->zip_code, ZIP_LENGTH);
        if (comparison == 0) {
            return 1;               /* found */
        } else if (comparison < 0) {
            high = mid - 1;         /* search the lower half */
        } else {
            low = mid + 1;          /* search the upper half */
        }
    }
    return 0;
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

    /* Number of records = file size / record size. */
    fseek(data_file, 0, SEEK_END);
    long record_count = ftell(data_file) / (long)sizeof(ZipRecord);
    rewind(data_file);

    printf("Record size: %zu bytes\n", sizeof(ZipRecord));
    printf("Total records: %ld\n\n", record_count);

    ZipRecord result;
    int reads = 0;

    if (binary_search(data_file, record_count, argv[1], &result, &reads)) {
        printf("--- RECORD FOUND ---\n");
        print_record(&result);
        printf("--------------------\n");
    } else {
        printf("ZIP code '%s' not found.\n", argv[1]);
    }

    printf("\nReads performed: %d\n", reads);

    fclose(data_file);
    return 0;
}
