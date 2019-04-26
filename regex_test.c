#include <regex.h>
#include <stdbool.h>
#include <stdio.h>
#define RESSIZE 10

void print(char *regex_str, char *line, regmatch_t *res) {
  if (res == NULL)
    return;
  int begin = res->rm_so, end = res->rm_eo;
  if (begin < 0 || end < 0) {
    puts("Not found!");
    return;
  }
  puts(line);
  int t = end - begin - 1;
  printf("%*c%*c\n", begin + 1, '^', t, t > 0 ? '^' : ' ');
  printf("S: %lld, E: %lld\n", res->rm_so, res->rm_eo);
}

int main() {
  puts("Use ^C to exit.");
  while (true) {
    char line[1024];
    char regex_str[1024];
    printf("STR: ");
    // Don't use fgets, it has \n
    gets(line);
    printf("REG: ");
    gets(regex_str);
    regex_t compiled_reg;
    int flag = REG_EXTENDED;
    if (regcomp(&compiled_reg, regex_str, flag)) {
      puts("compile fail");
      puts("------------------------------");
      continue;
    }
    regmatch_t res[compiled_reg.re_nsub + 1];
    bool success =
        !regexec(&compiled_reg, line, compiled_reg.re_nsub + 1, res, 0);
    if (success) {
      for (int i = 0; i <= compiled_reg.re_nsub; i++) {
        if (i) {
          puts("----------");
          printf("subregex%d:\n", i);
        }
        print(regex_str, line, &res[i]);
      }
    } else {
      puts("Not found!");
    }
    puts("------------------------------");
    regfree(&compiled_reg);
  }
  return 0;
}
