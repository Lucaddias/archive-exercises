#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CHUNKS 2

// Estrutura traduzida mantendo o tamanho original
typedef struct LocationData {
  char street_name[72];
  char neighborhood[72];
  char city_name[72];
  char state_full[72];
  char state_abbr[2];
  char zip_code[8];
  char padding[2];
} ZipRecord;

// Função de comparação para o qsort
int compare_zip_codes(const void *record_a, const void *record_b) {
  return strncmp(((ZipRecord *)record_a)->zip_code,
                 ((ZipRecord *)record_b)->zip_code, 8);
}

int main(int argc, char **argv) {
  FILE *source_file;
  long items_read;

  source_file = fopen("cep.dat", "rb");
  if (!source_file)
    return 1;

  fseek(source_file, 0, SEEK_END);
  long file_bytes = ftell(source_file);
  long total_records = file_bytes / sizeof(ZipRecord);
  rewind(source_file);

  long base_chunk_size = total_records / CHUNKS;
  long max_chunk_size = base_chunk_size + (total_records % CHUNKS);

  ZipRecord *record_buffer = malloc(max_chunk_size * sizeof(ZipRecord));

  // Passo 1: Criação e ordenação dos blocos
  for (int i = 0; i < CHUNKS; i++) {
    long current_chunk_size =
        (i == CHUNKS - 1) ? (total_records - base_chunk_size * (CHUNKS - 1))
                          : base_chunk_size;
    items_read = fread(record_buffer, sizeof(ZipRecord), current_chunk_size,
                       source_file);

    qsort(record_buffer, items_read, sizeof(ZipRecord), compare_zip_codes);

    char chunk_name[32];
    sprintf(chunk_name, "chunk%d.dat", i);
    FILE *chunk_output = fopen(chunk_name, "wb");
    fwrite(record_buffer, sizeof(ZipRecord), items_read, chunk_output);
    fclose(chunk_output);
  }

  free(record_buffer);

  // Passo 2: Mesclagem (Intercalação) dos blocos ordenados
  int remaining_chunks = CHUNKS;
  int current_round = 0;

  while (remaining_chunks > 1) {
    for (int b = 0; b < remaining_chunks; b += 2) {
      char file_a_name[64], file_b_name[64], output_name[64];

      if (current_round == 0) {
        sprintf(file_a_name, "chunk%d.dat", b);
        sprintf(file_b_name, "chunk%d.dat", b + 1);
      } else {
        sprintf(file_a_name, "temp_%d_%d.dat", current_round - 1, b / 2);
        sprintf(file_b_name, "temp_%d_%d.dat", current_round - 1, b / 2 + 1);
      }
      sprintf(output_name, "temp_%d_%d.dat", current_round, b / 2);

      FILE *file_a = fopen(file_a_name, "rb");
      FILE *file_b = fopen(file_b_name, "rb");
      FILE *merged_out = fopen(output_name, "wb");

      ZipRecord rec_a, rec_b, winning_record;
      long read_a = fread(&rec_a, sizeof(ZipRecord), 1, file_a);
      long read_b = fread(&rec_b, sizeof(ZipRecord), 1, file_b);

      while (read_a > 0 || read_b > 0) {
        if (read_a > 0 && read_b > 0 &&
            strncmp(rec_a.zip_code, rec_b.zip_code, 8) == 0) {
          winning_record = rec_a;
          read_a = fread(&rec_a, sizeof(ZipRecord), 1, file_a);
          read_b = fread(&rec_b, sizeof(ZipRecord), 1, file_b);
        } else if (read_a > 0 &&
                   (read_b == 0 ||
                    strncmp(rec_a.zip_code, rec_b.zip_code, 8) < 0)) {
          winning_record = rec_a;
          read_a = fread(&rec_a, sizeof(ZipRecord), 1, file_a);
        } else {
          winning_record = rec_b;
          read_b = fread(&rec_b, sizeof(ZipRecord), 1, file_b);
        }
        fwrite(&winning_record, sizeof(ZipRecord), 1, merged_out);
      }
      fclose(file_a);
      fclose(file_b);
      fclose(merged_out);
    }
    remaining_chunks /= 2;
    current_round++;
  }

  fclose(source_file);
  return 0;
}