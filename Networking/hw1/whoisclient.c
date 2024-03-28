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
// 28 flags for whois * 2 (each arg gets a param) = 56
#define MAXARGS 56

void setup(int *, char *);
void *get_in_addr(struct sockaddr *);
void parse_command_line(int argc, char **argv, char **hostname, char **port,
                        char **command, char ***args);
void test(int argc, char **argv, char ***args);

int main(int argc, char **argv) {
  char *hostname = NULL;
  char *port = NULL;
  char *command = NULL;
  char **args = NULL;

  test(argc, argv, &args);
  printf("back in main\n");
  // parse_command_line(argc, argv, &hostname, &port, &command, &args);

  // printf("main - hostname: %s\n", hostname);
  // printf("main - port: %s\n", port);
  // printf("args[0] - %s\n", args[0]);

  return 0;

  printf("line 30");
  for (char **p = args; p != NULL; p++) {
    printf("args registered as: %s", *p);
  }

  return 0;

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

void test(int argc, char **argv, char ***args) {
  if (argc < 3) {
    printf("error not enough args\n");
    exit(1);
  }
  *args = malloc(sizeof(char *) * (argc));
  for (int i = 0; i < argc; i++) {
    printf("i = %d | argv[%d] = %s\n", i, i, argv[i]);

    strcpy(*args[i], argv[i]);
  }
}

void parse_command_line(int argc, char **argv, char **hostname, char **port,
                        char **command, char ***args) {
  if (argc < 3) {
    printf("not enough args\n");
    exit(0);
  }
  *args = malloc(sizeof(char *) * (argc - 2)); // should be argc-2

  for (int i = 1; i < argc + 1; i++) {
    printf("i = %d\n", i);
    // hostname:server_port
    if (i == 1) {
      *hostname = strtok(argv[i], ":");
      *port = strtok(NULL, ":");
      printf("in parse: %s --- %s\n", *hostname, *port);
    }
    // command
    else if (i == 2) {
      printf("in i==2\n");
      *command = malloc(strlen(argv[i]) + 1);
      if (*command == NULL) {
        printf("mem alloc failed\n");
      }
      printf("malloc'd\n");
      strcpy(*command, argv[i]);
      // printf("cpy succ\n");
    }
    /*
      else if (i == argc) {
      printf("in i == argc\n");
      args[i] = NULL;
      break;
    }
    */
    else {
      printf("in i==3\n");
      // printf("%s\n", *args[0]);
      //(*args[0] = malloc(strlen(argv[i - 1])));
      //(*args)[i - 3] = malloc(strlen(argv[i - 2]));
      printf("succ\n");
      if ((*args)[i - 3] == NULL) {
        printf("*args i - 3\n");
      }
      printf("i is: %d\n", i);
      strcpy((*args)[i - 3], argv[i]);
      printf("copied %s to args[%d]\n", argv[i], i - 3);
      // printf("arg: %s", args[i - 3]);
    }
  }
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
