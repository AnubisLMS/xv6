// Test that fork fails gracefully.
// Tiny executable so that the limit can be filling the proc table.

#include "user.h"

#define N 1000

void forktest(void) {
  const char fork_test[] = "fork test\n";
  write(1, fork_test, sizeof(fork_test));

  int n;
  for(n = 0; n < N; n++) {
    int pid = fork();
    if(pid < 0)
      break;
    if(pid == 0)
      exit();
  }

  if(n == N) {
    const char fork_worked[] = "fork claimed to work N times!\n";
    write(1, fork_worked, sizeof(fork_worked));
    exit();
  }

  for(; n > 0; n--) {
    if(wait() < 0) {
      const char wait_early[] = "fork claimed to work N times!\n";
      write(1, wait_early, sizeof(wait_early));
      exit();
    }
  }

  if(wait() != -1) {
    const char wait_many[] = "wait got too many\n";
    write(1, wait_many, sizeof(wait_many));
    exit();
  }

  const char fork_ok[] = "fork test OK\n";
  write(1, "fork test OK\n", sizeof(fork_ok));
}

int main(void) {
  forktest();
  exit();
}
