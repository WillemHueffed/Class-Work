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

#define PORT "3490"
#define MAXDATASIZE 100

void setup(int *, char *);
void *get_in_addr(struct sockaddr *);
void parse_command_line(int argc, char **argv, char *hostname, char *port,
                        char *command, char **options, char **arg_list);

int main(int argc, char **argv) {
  char *hostname = NULL;
  char *port = NULL;
  char *command = NULL;
  char **options = NULL;
  char **arg_list = NULL;
  parse_command_line(argc, argv, hostname, port, command, options, arg_list);

  int sockfd;
  int numbytes;
  char buf[MAXDATASIZE];

  setup(&sockfd, argv[1]);

  if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
    perror("recv");
    exit(1);
  }

  buf[numbytes] = '\0';

  printf("client: received '%s'\n", buf);
  close(sockfd);
  return 0;
}

void parse_command_line(int argc, char **argv, char *hostname, char *port,
                        char *command, char **options, char **arg_list) {
  if (argc < 2) {
    printf("not enough args\n");
    exit(1);
  }
  for (int i = 0; i < argc; i++) {
    // hostname:server_port
    if (i == 1) {
      hostname = strtok(argv[i], ":");
      port = strtok(NULL, ":");
      printf("%s --- %s", hostname, port);
    }
    // command
    if (i == 2) {
    }

    if (i > 2) {
    }

    port = NULL;
    command = NULL;
    options = NULL;
    arg_list = NULL;
    arg_list = NULL;
  }
  exit(0);
}

void setup(int *sockfd, char *argv1) {
  struct addrinfo hints, *servinfo, *p;
  int rv;
  char s[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(argv1, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "get address info: %s\n", gai_strerror(rv));
    exit(1);
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((*sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
        -1) {
      perror("client: socket");
      continue;
    }
    if (connect(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(*sockfd);
      perror("client: listen");
      continue;
    }
    break;
  }

  if (p == NULL) {
    fprintf(stderr, "client: failed to connect\n");
    exit(2);
  }

  inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr), s,
            sizeof(s));
  printf("client: connecting to %s\n", s);

  freeaddrinfo(servinfo);
}

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}
