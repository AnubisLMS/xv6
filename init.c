// init: The initial user-level program

#include "fcntl.h"
#include "stat.h"
#include "user.h"

char* argv[] = {"sh", 0};

int main(void) {
  if(open("console", O_RDWR) < 0) {
    mknod("console", 1, 1);
    open("console", O_RDWR);
  }
  dup(0); // stdout
  dup(0); // stderr

  for(;;) {
    printf(1, "init: starting sh\n");
    int pid = fork();
    if(pid < 0) {
      printf(1, "init: fork failed\n");
      exit();
    }
    if(pid == 0) {
      exec("sh", argv);
      printf(1, "init: exec sh failed\n");
      exit();
    }
    for(int wpid = wait(); wpid >= 0 && wpid != pid; wpid = wait())
      printf(1, "zombie!\n");
  }
}
