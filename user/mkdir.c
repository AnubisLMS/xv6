#include "user.h"

int main(int argc, char* argv[]) {
  if(argc < 2) {
    fprintf(2, "Usage: mkdir files...\n");
    exit();
  }

  for(int i = 1; i < argc; i++) {
    if(mkdir(argv[i]) < 0) {
      fprintf(2, "mkdir: %s failed to create\n", argv[i]);
      break;
    }
  }

  exit();
}
