// init: The initial user-level program
#include "user.h"

char* argv[] = {"sh", 0};

int main(void) {
  int pid, wpid;
  printf("init: HERE\n");

  if(open("console", O_RDWR) < 0) {
    mknod("console", 1, 1);
    open("console", O_RDWR);
  }
  dup(0); // stdout
  dup(0); // stderr

  for(;;) {
    printf("init: starting sh\n");
    pid = fork();
    if(pid < 0) {
      printf("init: fork failed\n");
      exit();
    }
    if(pid == 0) {
      exec("sh", argv);
      printf("init: exec sh failed\n");
      exit();
    }
    while((wpid = wait()) >= 0 && wpid != pid)
      printf("zombie!\n");
  }
}
