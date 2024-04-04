#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define MAXDATASIZE 1000
// 28 flags for whois * 2 (each arg gets a param) = 56
#define MAXARGS 56

void setup(int *, char *, char *);
void *get_in_addr(struct sockaddr *);
void parse_command_line(int, char **, char **, char **, char ***);

void format_msg(char **, char ***);

int main(int argc, char **argv) {
  char *hostname = NULL;
  char *port = NULL;
  char *command = NULL;
  char **args = NULL;
  char *msg;
  char *msg2;

  parse_command_line(argc, argv, &hostname, &port, &args);

  format_msg(&msg, &args);
  msg2 = msg;

  int sockfd;
  int numbytes;
  char buf[MAXDATASIZE];

  setup(&sockfd, argv[1], port);

  int bytes_sent = 0;
  while (bytes_sent < sizeof(msg)) {
    int x = send(sockfd, msg, strlen(msg) - bytes_sent, 0);
    if (x == -1 || x == 0) {
      perror("send");
      close(sockfd);
      exit(1);
    }
    msg += x;
    bytes_sent += x;
  }
  free(msg2);

  int loop_flag = 1;
  int eom_flag = 0;
  while (loop_flag) {
    if ((numbytes = recv(sockfd, buf, MAXDATASIZE - 1, 0)) == -1) {
      perror("recv");
      exit(1);
    } else if (numbytes == 0) {
      if (eom_flag)
        loop_flag = 0;
    } else {
      buf[numbytes] = '\0';
      printf("%s", buf);
      if (!eom_flag)
        eom_flag = 1;
    }
  }

  if (close(sockfd) == -1) {
    perror("close");
    exit(1);
  }

  free(msg);
  for (int i = 0; args[i] != NULL; i++) {
    free(args[i]);
  }
  free(args);

  return 0;
}

void format_msg(char **msg, char ***args) {
  int total_length = 0;
  int argc = 0;
  for (int i = 0; (*args)[i] != NULL; i++) {
    argc++;
    total_length += strlen((*args)[i]);
  }

  *msg = malloc(total_length + (argc)); // seperating each arg with a \n ->
                                        // requires argc additional bytes
  if (msg == NULL) {
    perror("malloc");
    exit(1);
  }

  strcpy(*msg, "");
  for (int i = 0; (*args)[i] != NULL; i++) {
    strcat(*msg, (*args)[i]);
    if (i < argc) { // this is redundent was argc -1
      strcat(*msg, " ");
    }
  }
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
    perror("malloc");
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
      perror("malloc");
      exit(1);
    }
    strcpy((*args)[ac++], argv[i]);
  }
}

void setup(int *sockfd, char *argv1, char *port) {
  struct addrinfo hints, *servinfo, *p;
  int rv;
  char s[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(argv1, port, &hints, &servinfo)) != 0) {
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
      if (close(*sockfd) == -1) {
        perror("close");
        exit(1);
      }
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
  // printf("client: connecting to %s\n", s);

  freeaddrinfo(servinfo);
}

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}
