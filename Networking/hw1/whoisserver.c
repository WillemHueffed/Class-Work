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
#define BACKLOG 10
#define MAXDATASIZE 1000

void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);
void setup(int *sockfd);
void childProcess(int, int);
void parse_msg(char **command, char ***args, int numbytes, char *buf);

int main() {
  int sockfd, new_fd;
  struct sockaddr_storage their_addr;
  socklen_t sin_size;
  char s[INET6_ADDRSTRLEN];

  setup(&sockfd);

  while (1) {
    sin_size = sizeof their_addr;
    new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
    if (new_fd == -1) {
      perror("accept");
      continue;
    }

    inet_ntop(their_addr.ss_family, get_in_addr((struct sockaddr *)&their_addr),
              s, sizeof s);
    printf("server: got connection from %s\n", s);

    if (!fork()) {
      childProcess(sockfd, new_fd);
    }
    close(new_fd);
  }

  return 0;
}

void parse_msg(char **command, char ***args, int numbytes, char *buf) {
  // Get len of command
  /*
  int cc = 0;
  for (int i = 0; buf[i] != '\n'; i++) {
    cc++;
  }

  *command = malloc(sizeof(char) * cc + 1);
  printf("buf is: %s", buf);
  strncpy(*command, buf, cc);
  (*command)[cc] = '\0';
  */

  // get count of how many words there are
  int c = 0;
  for (int i = 0; i < strlen(buf); i++) {
    if (buf[i] == '\n') {
      c++;
    }
  }

  // if its not just "whois" then -> we have params we need to allocate for
  if (c > 1) {
    *args = malloc(sizeof(char *) * (c - 1));
  }

  char *tok;
  int count = 0;
  tok = strtok(buf, "\n");
  printf("parsing tokens\n");
  while (tok != NULL) {
    printf("%s\n", tok);

    if (count == 0) { // extract command
      *command = malloc(strlen(tok));
      strcpy(*command, tok);
    } else { // extract args
      (*args)[count] = malloc(strlen(tok));
      strcpy((*args)[count], tok);
    }
    count++;
    tok = strtok(NULL, "\n");
  }
  printf("exiting string parsing\n");
}

void childProcess(int sockfd, int new_fd) {
  close(sockfd);
  if (send(new_fd, "Hello, world!", 13, 0) == -1)
    perror("send");

  char buf[MAXDATASIZE];
  int numbytes;

  if ((numbytes = recv(new_fd, buf, MAXDATASIZE - 1, 0)) == -1) {
    perror("recv");
    exit(1);
  }
  buf[numbytes] = '\0';
  printf("server: received '%s'\n", buf);

  char *command;
  char **args;
  printf("calling parse msg\n");
  parse_msg(&command, &args, numbytes, buf);

  close(new_fd);

  printf("exiting child process\n");
  exit(0);
}

void sigchld_handler(int s) {
  s = s; // Surpress werror about s
  int saved_errno = errno;
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
  errno = saved_errno;
}

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void setup(int *sockfd) {
  printf("Hello from server\nlistening on port: %s\n", PORT);
  // int sockfd, new_fd;
  struct addrinfo hints, *servinfo, *p;
  // struct addrinfo hints, *servinfo, *p;
  // struct sockaddr_storage their_addr;
  // socklen_t sin_size;
  struct sigaction sa;
  // char s[INET6_ADDRSTRLEN];
  int yes = 1;

  int rv;

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE;

  if ((rv = getaddrinfo(NULL, PORT, &hints, &servinfo)) != 0) {
    fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
    exit(1);
  }

  for (p = servinfo; p != NULL; p = p->ai_next) {
    if ((*sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) ==
        -1) {
      perror("server: socket");
      continue;
    }
    if (setsockopt(*sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) ==
        -1) {
      perror("setsockopt");
      exit(1);
    }

    if (bind(*sockfd, p->ai_addr, p->ai_addrlen) == -1) {
      close(*sockfd);
      perror("server: bind");
      continue;
    }
    break;
  }

  freeaddrinfo(servinfo);

  if (p == NULL) {
    fprintf(stderr, "server failed to bind\n");
    exit(1);
  }

  if (listen(*sockfd, BACKLOG) == -1) {
    perror("listen");
    exit(1);
  }

  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  printf("server: waiting for connections...\n");
}
