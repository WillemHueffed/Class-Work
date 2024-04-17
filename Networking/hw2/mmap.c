#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main() {
  int fd;
  struct stat sb;
  void *mapped;

  fd = open("page1.html", O_RDONLY);
  if (fd == -1) {
    perror("file open");
    exit(1);
  }
  if (fstat(fd, &sb) == -1) {
    perror("fstat");
    close(fd);
    exit(1);
  }

  mapped = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (mapped == MAP_FAILED) {
    perror("mmap");
    close(fd);
    exit(1);
  }

  printf("File contents:\n%s\n", (char *)mapped);

  if (munmap(mapped, sb.st_size) == -1) {
    perror("munmap");
    exit(1);
  }
  return 0;
}
