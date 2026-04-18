#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct LocationData {
  char street_name[72];
  char neighborhood[72];
  char city_name[72];
  char state_full[72];
  char state_abbr[2];
  char zip_code[8];
  char padding[2];
} ZipRecord;

// Nossa estrutura leve de Índice
typedef struct IndexTable {
  char zip_key[8];
  long byte_offset;
} IndexEntry;

// Limpa os espaços na hora de imprimir
void print_clean(const char *label, const char *text, int max_length) {
  int len = max_length;
  while (len > 0 && (text[len - 1] == ' ' || text[len - 1] == '\n' ||
                     text[len - 1] == '\r')) {
    len--;
  }
  printf("%-14s: %.*s\n", label, len, text);
}

// Compara o CEP dentro do índice
int compare_indices(const void *item1, const void *item2) {
  return strncmp(((IndexEntry *)item1)->zip_key, ((IndexEntry *)item2)->zip_key,
                 8);
}

int main(int argc, char **argv) {
  if (argc < 2) {
    printf("Usage: %s <ZIP_CODE>\n", argv[0]);
    return 1;
  }

  FILE *main_database, *index_file;
  ZipRecord current_record;
  long items_read, total_bytes, total_records;

  main_database = fopen("cep.dat", "rb");
  if (!main_database)
    return 1;

  fseek(main_database, 0, SEEK_END);
  total_bytes = ftell(main_database);
  total_records = total_bytes / sizeof(ZipRecord);

  // Alocação do array de índice na memória
  IndexEntry *index_array =
      (IndexEntry *)malloc(total_records * sizeof(IndexEntry));
  rewind(main_database);

  items_read = fread(&current_record, sizeof(ZipRecord), 1, main_database);
  int array_pos = 0;
  long current_byte_pos = 0;

  // 1. Extração do Índice
  while (items_read > 0) {
    strncpy(index_array[array_pos].zip_key, current_record.zip_code, 8);
    index_array[array_pos].byte_offset = current_byte_pos;

    array_pos++;
    current_byte_pos += sizeof(ZipRecord);
    items_read = fread(&current_record, sizeof(ZipRecord), 1, main_database);
  }

  // 2. Ordenação do Índice
  qsort(index_array, total_records, sizeof(IndexEntry), compare_indices);

  // 3. Salvando o Índice
  index_file = fopen("index_data.dat", "w+b");
  fwrite(index_array, sizeof(IndexEntry), total_records, index_file);
  rewind(index_file);

  // ==========================================
  // 4. Busca Binária usando o Arquivo de Índice
  // ==========================================
  IndexEntry current_index_entry;
  long found_offset = -1;
  long left_bound = 0;
  long right_bound = total_records - 1;
  long mid_point = (left_bound + right_bound) / 2;

  fseek(index_file, mid_point * sizeof(IndexEntry), SEEK_SET);

  while (left_bound <= right_bound) {
    items_read = fread(&current_index_entry, sizeof(IndexEntry), 1, index_file);

    int cmp_result = strncmp(argv[1], current_index_entry.zip_key, 8);

    if (cmp_result == 0) {
      found_offset = current_index_entry.byte_offset;
      break;
    } else if (cmp_result < 0) { // Procurar na esquerda
      right_bound = mid_point - 1;
      mid_point = (left_bound + right_bound) / 2;
      fseek(index_file, mid_point * sizeof(IndexEntry), SEEK_SET);
    } else { // Procurar na direita
      left_bound = mid_point + 1;
      mid_point = (left_bound + right_bound) / 2;
      fseek(index_file, mid_point * sizeof(IndexEntry), SEEK_SET);
    }
  }

  if (found_offset == -1) {
    printf("\n--- RECORD NOT FOUND ---\n");
  } else {
    // 5. Pula cirurgicamente no arquivo gigante para resgatar os dados
    fseek(main_database, found_offset, SEEK_SET);
    items_read = fread(&current_record, sizeof(ZipRecord), 1, main_database);

    printf("\n--- RECORD FOUND (VIA INDEX) ---\n");
    print_clean("Street", current_record.street_name, 72);
    print_clean("Neighborhood", current_record.neighborhood, 72);
    print_clean("City", current_record.city_name, 72);
    print_clean("State", current_record.state_full, 72);
    print_clean("UF", current_record.state_abbr, 2);
    print_clean("ZIP Code", current_record.zip_code, 8);
    printf("--------------------------------\n");
  }

  fclose(main_database);
  fclose(index_file);
  free(index_array);

  return 0;
}