#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
  // arguments are valid in range 1->argc-1
  printf("hello from client\n");
  printf("You have entered %d args:\n", argc);
  for (int i = 0; i < argc; i++) {
    printf("%s\n", argv[i]);
  }
}
