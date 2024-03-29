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
                        char ***args);

void format_msg(char **msg, char ***args) {
  int total_length = 0;
  int argc = 0;
  for (int i = 0; (*args)[i] != NULL; i++) {
    argc++;
    printf("%s\n", (*args)[i]);
    total_length += strlen((*args)[i]);
  }
  // printf("total length is: %d\n", total_length);

  *msg = malloc(total_length + (argc)); // seperating each arg with a \n ->
                                        // requires argc additional bytes
  if (msg == NULL) {
    printf("memory allocation failed");
    exit(1);
  }

  strcpy(*msg, "");
  for (int i = 0; (*args)[i] != NULL; i++) {
    strcat(*msg, (*args)[i]);
    if (i < argc - 1) {
      strcat(*msg, "\n");
    }
  }
}

int main(int argc, char **argv) {
  char *hostname = NULL;
  char *port = NULL;
  char *command = NULL;
  char **args = NULL;
  char *msg;

  parse_command_line(argc, argv, &hostname, &port, &args);

  format_msg(&msg, &args);

  printf("The formatted msg is:\n%s", msg);

  // printf("hostname: %s\n", hostname);
  // printf("port: %s\n", port);

  // for (int i = 0; args[i] != NULL; i++) {
  //  printf("arg: %s\n", args[i]);
  //}

  ///

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

  if (send(sockfd, msg, strlen(msg), 0) == -1)
    perror("send");

  close(sockfd);
  return 0;
}

void parse_command_line(int argc, char **argv, char **hostname, char **port,
                        char ***args) {
  // Catch insufficient args
  if (argc < 3) {
    printf("error not enough args\n");
    exit(1);
  }

  // allocate mem
  *args = malloc((sizeof(char *) * argc));
  if (*args == NULL) {
    printf("Memory allocation failed\n");
    exit(1);
  }

  // grab port:host pair
  *hostname = strtok(argv[1], ":");
  *port = strtok(NULL, ":");

  // extract args
  int ac = 0;
  for (int i = 2; i < argc; i++) {
    (*args)[ac] = malloc(strlen(argv[i]) + 1);
    if ((*args)[ac] == NULL) {
      printf("Memory allocation failed\n");
      exit(1);
    }
    strcpy((*args)[ac++], argv[i]);
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