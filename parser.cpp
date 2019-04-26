#include <iostream>
#include <vector>
extern "C" {
#include "lexical_analysis.h"
}

using namespace std;

int main(int argc, char *argv[]) {
  init_input(NULL);
  init_tokens();
  analysis();

  clean_tokens();
  return 0;
}
