#include "user.h"

int main(int argc, char** argv) {
  if(argc < 2) {
    fprintf(2, "usage: kill pid...\n");
    exit();
  }
  for(int i = 1; i < argc; i++)
    kill(atoi(argv[i]));
  exit();
}
