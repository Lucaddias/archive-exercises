/*
 * Work 1 - Sequential Search on a Fixed-Length Record File
 *
 * Reads a binary file of address records ("zipcodes.dat") sequentially, from the
 * first record to the last, looking for the ZIP code passed on the command
 * line. Every record read counts as one disk access, so this program also
 * reports how many reads were needed. It is meant to be compared against the
 * binary search version to show the performance gap on large sorted files.
 *
 * Usage: ./sequential_search <ZIP_CODE>
 */

#include <stdio.h>
#include <string.h>

#define DATA_FILE "zipcodes.dat"
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

    printf("Record size: %zu bytes\n\n", sizeof(ZipRecord));

    ZipRecord record;
    int reads = 0;
    int found = 0;

    /* Scan the whole file from the beginning until the key is found. */
    while (fread(&record, sizeof(ZipRecord), 1, data_file) == 1) {
        reads++;
        if (strncmp(argv[1], record.zip_code, ZIP_LENGTH) == 0) {
            found = 1;
            printf("--- RECORD FOUND ---\n");
            print_record(&record);
            printf("--------------------\n");
            break;
        }
    }

    if (!found) {
        printf("ZIP code '%s' not found.\n", argv[1]);
    }

    printf("\nReads performed: %d\n", reads);

    fclose(data_file);
    return 0;
}
