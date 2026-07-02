/*
 * Step 1 of Work 5: build two random samples of the data file.
 *
 * Each 300-byte record from "zipcodes.dat" is independently copied into
 * "sample1.dat" with 80% probability and into "sample2.dat" with 80%
 * probability. Because the two draws are independent, the samples overlap only
 * partially, which is exactly what makes the later "join" step interesting: some
 * records of sample2 exist in sample1 (and its index) and some do not.
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define RECORD_SIZE 300
#define SAMPLE_RATE 0.8

void generate_samples(void)
{
    FILE *input = fopen("zipcodes.dat", "rb");
    FILE *sample1 = fopen("sample1.dat", "wb");
    FILE *sample2 = fopen("sample2.dat", "wb");

    if (input == NULL || sample1 == NULL || sample2 == NULL) {
        printf("Error opening the sample files.\n");
        if (input) fclose(input);
        if (sample1) fclose(sample1);
        if (sample2) fclose(sample2);
        return;
    }

    srand((unsigned)time(NULL));

    char record[RECORD_SIZE];
    while (fread(record, RECORD_SIZE, 1, input) == 1) {
        if ((double)rand() / RAND_MAX <= SAMPLE_RATE) {
            fwrite(record, RECORD_SIZE, 1, sample1);
        }
        if ((double)rand() / RAND_MAX <= SAMPLE_RATE) {
            fwrite(record, RECORD_SIZE, 1, sample2);
        }
    }

    fclose(input);
    fclose(sample1);
    fclose(sample2);
}
