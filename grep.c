// Simple grep.  Only supports ^ . * $ operators.

#include "stat.h"
#include "user.h"

char buf[1024];
int match(char*, char*);

void grep(char* pattern, int fd) {
  for(int n, m = 0; (n = read(fd, buf + m, sizeof(buf) - m - 1)) > 0;) {
    m += n;
    buf[m] = '\0';
    char* p = buf;
    for(char* q; (q = strchr(p, '\n'));) {
      *q = 0;
      if(match(pattern, p)) {
        *q = '\n';
        write(1, p, q + 1 - p);
      }
      p = q + 1;
    }
    if(p == buf)
      m = 0;
    if(m > 0) {
      m -= p - buf;
      memmove(buf, p, m);
    }
  }
}

int main(int argc, char* argv[]) {
  if(argc <= 1) {
    printf(2, "usage: grep pattern [file ...]\n");
    exit();
  }
  char* pattern = argv[1];

  if(argc <= 2) {
    grep(pattern, 0);
    exit();
  }

  for(int i = 2; i < argc; i++) {
    int fd = open(argv[i], 0);
    if(fd < 0) {
      printf(1, "grep: cannot open %s\n", argv[i]);
      exit();
    }
    grep(pattern, fd);
    close(fd);
  }
  exit();
}

// Regexp matcher from Kernighan & Pike,
// The Practice of Programming, Chapter 9.

int matchhere(char*, char*);
int matchstar(int, char*, char*);

int match(char* re, char* text) {
  if(re[0] == '^')
    return matchhere(re + 1, text);
  do { // must look at empty string
    if(matchhere(re, text))
      return 1;
  } while(*text++ != '\0');
  return 0;
}

// matchhere: search for re at beginning of text
int matchhere(char* re, char* text) {
  if(re[0] == '\0')
    return 1;
  if(re[1] == '*')
    return matchstar(re[0], re + 2, text);
  if(re[0] == '$' && re[1] == '\0')
    return *text == '\0';
  if(*text != '\0' && (re[0] == '.' || re[0] == *text))
    return matchhere(re + 1, text + 1);
  return 0;
}

// matchstar: search for c*re at beginning of text
int matchstar(int c, char* re, char* text) {
  do { // a * matches zero or more instances
    if(matchhere(re, text))
      return 1;
  } while(*text != '\0' && (*text++ == c || c == '.'));
  return 0;
}
