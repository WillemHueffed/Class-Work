#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
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

#define PORT "10485"
#define BACKLOG 20
#define MAXDATASIZE 1000
#define MAX_METHOD_LEN 10
#define MAX_PATH_LEN 100
#define MAX_VERSION_LEN 10

typedef struct {
  char *key;
  char *val;
} key_val;

typedef struct {
  char *method;
  char *path;
  char *params;
  int argc;
  char *version;
  key_val **args;
} HttpRequest;

void parse_http_request(char *request_str, HttpRequest *request);
void mmap_file(const char *, char **);
void alloc_http_msg(char **msg, char *, char *status, int content_length);
void sigchld_handler(int);
void *get_in_addr(struct sockaddr *);
void setup(int *);
void childProcess(int, int);

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

void *get_in_addr(struct sockaddr *sa) {
  if (sa->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)sa)->sin_addr);
  }
  return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void sigchld_handler(int s) {
  s = s; // Surpress werror about s
  int saved_errno = errno;
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
  errno = saved_errno;
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

void childProcess(int sockfd, int new_fd) {
  if (close(sockfd) == -1) {
    perror("close");
    exit(1);
  }

  // Get incoming HTTP req
  char *incoming_msg = malloc(1000);
  int numbytes = 0;
  char *end_of_hdr;
  int bytes_recvd = 0;

  while (1) {
    bytes_recvd = recv(new_fd, incoming_msg, MAXDATASIZE, MSG_PEEK);
    if (bytes_recvd < 0) {
      perror("recv");
      exit(1);
    } else {
      end_of_hdr = strstr(incoming_msg, "\r\n\r\n");
      if (end_of_hdr != NULL) {
        break;
      }
    }
  }
  bytes_recvd = recv(new_fd, incoming_msg, MAXDATASIZE, 0);
  if (bytes_recvd < 0) {
    perror("recv");
    exit(1);
  }
  printf("The incoming msg is:\n%s===============\n", incoming_msg);

  // Parse HTTP req
  HttpRequest req;
  parse_http_request(incoming_msg, &req);

  char *status = "200 OK";
  char *http_resp;
  char *hdr;
  char *file = NULL;
  char *args[] = {"./fib.cgi", NULL};
  char *envp[] = {NULL};

  // Check this logic. if execvp fails what gets sent to client?
  printf("req path is: %s\n", req.path);
  if (!strcmp(req.path, "fib.cgi")) {
    if ((dup2(new_fd, 1) == -1) || (dup2(new_fd, 2) == -1)) {
      perror("dup2");
      exit(1);
    }
    printf("executing fib.cgi...\n");
    if (execve("fib.cgi", args, envp) == -1) {
      perror("execvp");
      exit(1);
    }
  } else {
    mmap_file(req.path, &file);
    alloc_http_msg(&http_resp, file, status, strlen(file));
  }

  // Form HTTP resp (for static content)

  // Send resp
  int bytes_sent = 0;
  printf("%s", http_resp);
  int msg_length = strlen(http_resp);

  while (bytes_sent < msg_length) {
    int x = send(new_fd, http_resp + bytes_sent, msg_length - bytes_sent, 0);
    if (x == -1 || x == 0) {
      perror("send");
      exit(1);
    }
    bytes_sent += x;
  }

  // Close junk out
  if (close(new_fd) == -1) {
    perror("close");
    exit(1);
  }
  if (file != NULL && munmap(file, strlen(file)) == -1) {
    printf("munmap\n");
    perror("munmap");
    exit(1);
  }
  exit(0);
}

// TODO: This most definetly has memory leaks
void alloc_http_msg(char **http_resp, char *body, char *status,
                    int content_length) {
  char *header = (char *)malloc(1000);
  const char *date = "4/16/24";
  const char *content_type = "text/html";
  snprintf(header, 1000,
           "HTTP/1.1 %s\r\n"
           "Connection: close\r\n"
           "Date: %s\r\n"
           "Content-Length: %d\r\n"
           "Content-Type: %s\r\n"
           "Server: cpsc4510 web server 1.0\r\n"
           "\r\n",
           status, date, content_length, content_type);
  *http_resp = malloc(strlen(header) + strlen(body) + 1);
  // TODO: Check return vals
  strcpy(*http_resp, header);
  strcat(*http_resp, body);
  // printf("header msg\n");
  // printf("%s", *msg);
}

void mmap_file(const char *path, char **mapped) {
  int fd;
  printf("The file path is: %s\n", path);
  struct stat sb;
  fd = open(path, O_RDONLY);
  if (fd == -1) {
    perror("file open");
    exit(1);
  }
  if (fstat(fd, &sb) == -1) {
    perror("fstat");
    close(fd);
    exit(1);
  }

  *mapped = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (*mapped == MAP_FAILED) {
    perror("mmap");
    close(fd);
    exit(1);
  }

  // printf("File contents:\n%s\n", (char *)*mapped);
}

void parse_http_request(char *request_str, HttpRequest *request) {
  char *method;
  char *path;
  char *version;

  const char *end_of_request_line = strchr(request_str, '\n');
  if (!end_of_request_line) {
    printf("Invalid request format: No end of request line found.\n");
    exit(1);
  }

  int spc_count = 0;
  const char *cr = strpbrk(request_str, "\r\n");
  if (!cr) {
    printf("malformed: no \\r\\n found\n");
  }
  for (int i = 0; i < cr - request_str; i++) {
    if (request_str[i] == ' ')
      spc_count++;
  }
  if (spc_count != 2) {
    printf("malformed request");
  }

  method = strtok(request_str, " ");
  path = strtok(NULL, " ");
  version = strtok(NULL, "\r\n");

  const char *params_check = strchr(path, '?');

  if (!params_check) {
    request->method = method;
    // TODO: Validate that offsetting the path doesn't mess up calling free
    // later
    request->path = ++path;
    request->version = version;
    return;
  }

  char *del = strchr(path, '?');
  int len = del - path;
  // TODO: double check this pointer arithmatic when I'm not tired af
  printf("CALLED\n");
  char *just_path = (char *)malloc(len);
  strncpy(just_path, path + 1, len - 1);

  char *unp_args = (char *)malloc(strlen(path) - len + 1);
  strncpy(unp_args, path + len, strlen(path) - len);
  unp_args++; // get rid of / in string

  int argc = 1;
  for (int i = 0; i < strlen(unp_args); i++) {
    if (unp_args[i] == '&')
      argc++;
  }
  // printf("counted %d args\n", argc);
  key_val **args = (key_val **)malloc(sizeof(key_val *) * argc + 1);
  args[argc] = NULL;

  int i = 0;
  for (char *tok = strtok(unp_args, "&"); tok != NULL;
       tok = strtok(NULL, "&")) {
    // printf("Token: %s\n", tok);
    // printf("strlen(tok) = %d\n", (int)strlen(tok));
    char *del_pos = strchr(tok, '=');
    int kl = del_pos - tok;
    char *key = (char *)malloc(kl + 1);
    key[kl] = '\0';
    // char *val = (char *)malloc(strlen(tok) - kl + 1);
    char *val = strdup(tok + kl + 1);
    strncpy(key, tok, kl);
    strncpy(val, tok + kl, strlen(key) - kl);
    args[i] = (key_val *)malloc(sizeof(key_val));
    args[i]->key = key;
    args[i]->val = val;
    i++;
  }

  request->method = method;
  request->path = just_path; // ofset to get rid of `\`
  request->version = version;
  request->args = args;
  request->argc = argc;
}
