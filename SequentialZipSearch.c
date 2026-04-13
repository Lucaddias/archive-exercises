#include <stdio.h>
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

void print_clean(const char *label, const char *text, int max_length) {
  int len = max_length;
  while (len > 0 && (text[len - 1] == ' ' || text[len - 1] == '\n' ||
                     text[len - 1] == '\r')) {
    len--;
  }
  printf("%-14s: %.*s\n", label, len, text);
}

int main(int argc, char **argv) {

  if (argc < 2) {
    fprintf(stderr, "Usage: %s <ZIP_CODE>\n", argv[0]);
    return 1;
  }

  FILE *database_file = fopen("cep.dat", "rb");
  if (database_file == NULL) {
    fprintf(stderr, "Error: Could not open database file.\n");
    return 1;
  }

  ZipRecord current_record;
  int steps_taken = 0;
  int found = 0;

  printf("Record size: %lu bytes\n\n", sizeof(ZipRecord));

  while (fread(&current_record, sizeof(ZipRecord), 1, database_file) == 1) {
    steps_taken++; // Conta cada leitura feita no disco

    if (strncmp(argv[1], current_record.zip_code, 8) == 0) {
      found = 1;
      printf("--- RECORD FOUND ---\n");
      print_clean("Street", current_record.street_name, 72);
      print_clean("Neighborhood", current_record.neighborhood, 72);
      print_clean("City", current_record.city_name, 72);
      print_clean("State", current_record.state_full, 72);
      print_clean("UF", current_record.state_abbr, 2);
      print_clean("ZIP Code", current_record.zip_code, 8);
      printf("--------------------\n");

      break;
    }
  }

  if (found == 0) {
    printf("--- RECORD NOT FOUND ---\n");
  }

  printf("\nTotal reads performed: %d\n", steps_taken);

  fclose(database_file);
  return 0;
}