#include <stdio.h>
#include <string.h>

// Estrutura renomeada e variáveis traduzidas
typedef struct LocationData {
  char street_name[72];
  char neighborhood[72];
  char city_name[72];
  char state_full[72];
  char state_abbr[2];
  char zip_code[8];
  char padding[2];
} ZipRecord;

int main(int argc, char **argv) {
  // Verificação de argumentos
  if (argc < 2) {
    fprintf(stderr, "Usage: %s <ZIP_CODE>\n", argv[0]);
    return 1;
  }

  // Abertura do arquivo com tratamento de erro
  FILE *database_file = fopen("cep_ordenado.dat", "rb");
  if (database_file == NULL) {
    fprintf(stderr, "Error: Could not open database file.\n");
    return 1;
  }

  ZipRecord current_record;
  int steps_taken = 0;

  printf("Record size: %lu bytes\n\n", sizeof(ZipRecord));

  // Descoberta do tamanho do arquivo e total de registros
  fseek(database_file, 0, SEEK_END);
  long file_size_bytes = ftell(database_file);
  long total_entries = file_size_bytes / sizeof(ZipRecord);

  // Variáveis de limite para a busca binária
  long start = 0;
  long end = total_entries - 1;
  long mid_point;

  // Loop principal da busca binária
  while (start <= end) {
    steps_taken++;

    // Cálculo semotimizado para evitar overflow de variáveis long
    mid_point = start + (end - start) / 2;

    // Posiciona o cursor e lê o registro do meio
    fseek(database_file, mid_point * sizeof(ZipRecord), SEEK_SET);
    fread(&current_record, sizeof(ZipRecord), 1, database_file);

    // Armazena o resultado para evitar chamar strncmp múltiplas vezes
    int cmp_result = strncmp(argv[1], current_record.zip_code, 8);

    if (cmp_result < 0) {
      end = mid_point - 1; // Busca na metade inferior
    } else if (cmp_result > 0) {
      start = mid_point + 1; // Busca na metade superior
    } else {
      // CEP Encontrado! Imprimindo os dados formatados
      printf("--- RECORD FOUND ---\n");
      printf("Street : %.72s\n", current_record.street_name);
      printf("Dist   : %.72s\n", current_record.neighborhood);
      printf("City   : %.72s\n", current_record.city_name);
      printf("State  : %.72s\n", current_record.state_full);
      printf("UF     : %.2s\n", current_record.state_abbr);
      printf("ZIP    : %.8s\n", current_record.zip_code);
      break;
    }
  }

  printf("\nTotal reads: %d\n", steps_taken);
  fclose(database_file);

  return 0;
}