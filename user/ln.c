#include "user.h"

int main(int argc, char* argv[]) {
  if(argc != 3) {
    fprintf(2, "Usage: ln old new\n");
    exit();
  }
  if(link(argv[1], argv[2]) < 0)
    fprintf(2, "link %s %s: failed\n", argv[1], argv[2]);
  exit();
}
