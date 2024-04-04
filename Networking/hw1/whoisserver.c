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

#define PORT "10485"
#define BACKLOG 20
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

    if (!fork()) {
      childProcess(sockfd, new_fd);
    }
    close(new_fd);
  }

  return 0;
}

void parse_msg(char **command, char ***args, int numbytes, char *buf) {
  // get count of how many words there are
  int c = 0;
  for (int i = 0; i < strlen(buf); i++) {
    if (buf[i] == ' ') {
      c++;
    }
  }

  // allocate memory for command and arguments
  *args = malloc(sizeof(char *) * c);
  if (*args == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }
  (*args)[0] = malloc(sizeof(char) * strlen(buf)); // allocate for the command
  if ((*args)[0] == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  char *tok;
  int count = 0;
  tok = strtok(buf, " ");
  while (tok != NULL) {
    if (count == 0) {          // if it's the first token, it's the command
      strcpy((*args)[0], tok); // copy the command
      *command = (*args)[0];
    } else {
      (*args)[count] =
          malloc(sizeof(char) * strlen(tok)); // allocate for the argument
      if ((*args)[count] == NULL) {
        perror("malloc");
        exit(EXIT_FAILURE);
      }
      strcpy((*args)[count], tok); // copy the argument
    }
    count++;
    tok = strtok(NULL, " ");
  }
  (*args)[count] = NULL;
}

void childProcess(int sockfd, int new_fd) {
  if (close(sockfd) == -1) {
    perror("close");
    exit(1);
  }

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

  parse_msg(&command, &args, numbytes, buf);

  if (strcmp(command, "whois") != 0) {
    int bytes_sent = 0;
    char *msg = "Internal error: the command is not supported!\n";
    int x = send(new_fd, &msg, strlen(msg) - bytes_sent, 0) == -1;
    if (x == 0 || x == 1) {
      perror("send");
      close(new_fd);
      exit(1);
    }
    msg += x;
    bytes_sent += x;
  } else {
    if ((dup2(new_fd, 1) == -1) || (dup2(new_fd, 2) == -1)) {
      perror("dup2");
      exit(1);
    }
    if (execvp(command, args) == -1) {
      perror("execvp");
      exit(1);
    }
  }

  if (close(new_fd) == -1) {
    perror("close");
    exit(1);
  }

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
  struct addrinfo hints, *servinfo, *p;
  struct sigaction sa;
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
      continue;
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

  printf("Listening on port %s\n", PORT);

  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, NULL) == -1) {
    perror("sigaction");
    exit(1);
  }

  printf("server: waiting for connections...\n");
}
