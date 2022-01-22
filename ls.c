#include "fs.h"
#include "stat.h"
#include "user.h"

char* fmtname(char* path) {
  static char buf[DIRSIZ + 1];
  char* p = path + strlen(path);

  // Find first character after last slash.
  for(; p >= path && *p != '/'; p--) {}
  p++;

  // Return blank-padded name.
  if(strlen(p) >= DIRSIZ)
    return p;
  memmove(buf, p, strlen(p));
  memset(buf + strlen(p), ' ', DIRSIZ - strlen(p));
  return buf;
}

void ls(char* path) {
  char buf[512];

  int fd = open(path, 0);
  if(fd < 0) {
    printf(2, "ls: cannot open %s\n", path);
    return;
  }

  struct stat st;
  if(fstat(fd, &st) < 0) {
    printf(2, "ls: cannot stat %s\n", path);
    close(fd);
    return;
  }

  switch(st.type) {
    case T_FILE:
      printf(1, "%s %d %d %d\n", fmtname(path), st.type, st.ino, st.size);
      break;

    case T_DIR: {
      if(strlen(path) + 1 + DIRSIZ + 1 > sizeof(buf)) {
        printf(1, "ls: path too long\n");
        break;
      }
      strcpy(buf, path);
      char* p = buf + strlen(buf);
      *p++ = '/';
      struct dirent de;
      while(read(fd, &de, sizeof(de)) == sizeof(de)) {
        if(de.inum == 0)
          continue;
        memmove(p, de.name, DIRSIZ);
        p[DIRSIZ] = 0;
        if(stat(buf, &st) < 0) {
          printf(1, "ls: cannot stat %s\n", buf);
          continue;
        }
        printf(1, "%s %d %d %d\n", fmtname(buf), st.type, st.ino, st.size);
      }
      break;
    }
  }
  close(fd);
}

int main(int argc, char* argv[]) {
  if(argc < 2) {
    ls(".");
    exit();
  }
  for(int i = 1; i < argc; i++)
    ls(argv[i]);
  exit();
}
