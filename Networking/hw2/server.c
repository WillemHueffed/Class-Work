#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <semaphore.h>
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

// #define PORT "10485"
#define PORT "10488"
#define BACKLOG 20
#define MAXDATASIZE 1000
#define MAX_METHOD_LEN 10
#define MAX_PATH_LEN 100
#define MAX_VERSION_LEN 10
#define THREAD_NUM 10
#define BUFFER_SIZE 25

typedef struct {
  char *key;
  char *val;
} key_val;

typedef struct {
  int portal_fd;
  int conn_fd;
} file_descriptors;

typedef struct {
  char *method;
  char *path;
  char *query;
  char *version;
} HttpRequest;

// Credit for producer/consumer semaphore code:
// https://code-vault.net/lesson/tlu0jq32v9:1609364042686
sem_t semEmpty;
sem_t semFull;
pthread_mutex_t mutexBuffer;
file_descriptors *buffer[BUFFER_SIZE];
int buff_count = 0;

void print_usage() {
  perror("Error: call the script like \"./server [-p port] [-t threads] [-b "
         "buffers]\"\n");
}

void parse_command_line(int argc, char **argv, char **ports_num,
                        char **threads_num, char **client_buff_num) {
  if (argc != 7) {
    print_usage();
    exit(1);
  }
  int option;
  int pflag = 0;
  int tflag = 0;
  int bflag = 0;
  // char *server;
  // char *port;

  *ports_num = NULL;
  *threads_num = NULL;
  *client_buff_num = NULL;

  while ((option = getopt(argc, argv, "p:t:b:")) != -1) {
    switch (option) {
    case 'p':
      pflag++;
      *ports_num = strdup(optarg);
      break;
    case 't':
      tflag++;
      *threads_num = strdup(optarg);
      break;
    case 'b':
      bflag++;
      *client_buff_num = strdup(optarg);
    default:
      print_usage();
      exit(1);
    }
  }

  // free if event of errors
  if (!*ports_num || !*threads_num || !*client_buff_num) {
    if (!*ports_num) {
      if (*threads_num)
        free(*threads_num);
      if (*client_buff_num)
        free(*client_buff_num);
    }
    if (!*threads_num) {
      if (*ports_num)
        free(*ports_num);
      if (*client_buff_num)
        free(*client_buff_num);
    }
    if (!*client_buff_num) {
      if (*ports_num)
        free(*ports_num);
      if (*threads_num)
        free(*threads_num);
    }
    print_usage();
  }
  // TODO: should I exit here or go back to calling function to do deallocation?
}

void parse_http_request(char *request_str, HttpRequest *request);
void getHTTPReq(int fd, char **req);
void doChildProcess(int fd, char **args, char **envp);
void send_msg(char *http_resp, int fd);
int mmap_file(const char *, char **);
void alloc_http_msg(char **msg, char *, char *status, int content_length);
void sigchld_handler(int);
void *get_in_addr(struct sockaddr *);
void setup(int *);
int has_adjacent_periods(const char *);
void serve_static(int fd, HttpRequest *req);
void *consumer(void *);

int main() {
  pthread_mutex_init(&mutexBuffer, NULL);
  sem_init(&semEmpty, 0, BUFFER_SIZE);
  sem_init(&semFull, 0, 0);
  pthread_t th[THREAD_NUM];

  for (int i = 0; i < THREAD_NUM; i++) {
    if (pthread_create(&th[i], NULL, &consumer, NULL) != 0) {
      perror("Failed to create thread");
      exit(1);
    }
  }

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

    file_descriptors *fds =
        (file_descriptors *)malloc(sizeof(file_descriptors));
    fds->portal_fd = sockfd;
    fds->conn_fd = new_fd;
    sem_wait(&semEmpty);
    pthread_mutex_lock(&mutexBuffer);
    buffer[buff_count] = fds;
    buff_count++;
    pthread_mutex_unlock(&mutexBuffer);
    sem_post(&semFull);
  }

  for (int i = 0; i < THREAD_NUM; i++) {
    if (pthread_join(th[i], NULL) != 0) {
      perror("Failed to join thread\n");
    }
  }

  sem_destroy(&semEmpty);
  sem_destroy(&semFull);
  pthread_mutex_destroy(&mutexBuffer);

  printf("exiting\n");
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

void serve_static(int fd, HttpRequest *req) {
  char *http_resp;
  char *status = "200 OK";
  char *file;
  int ret_val = mmap_file(req->path, &file);
  if (ret_val != 0) {
    if (ret_val == 1) {
      status = "404";
      file = "error file not found\n";
    }
    if (ret_val == 2) {
      status = "500";
      file = "internal server error\n";
    }
  }
  alloc_http_msg(&http_resp, file, status, strlen(file));

  send_msg(http_resp, fd);

  // Close junk out
  if (close(fd) == -1) {
    perror("close");
    exit(1);
  }
  if (!ret_val && file != NULL && munmap(file, strlen(file)) == -1) {
    perror("munmap");
    exit(1);
  }
}

void send_msg(char *http_resp, int fd) {
  int bytes_sent = 0;
  int msg_length = strlen(http_resp);

  while (bytes_sent < msg_length) {
    int x = send(fd, http_resp + bytes_sent, msg_length - bytes_sent, 0);
    if (x == -1 || x == 0) {
      perror("send");
      exit(1);
    }
    bytes_sent += x;
  }
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
  free(header);
}

int mmap_file(const char *path, char **mapped) {
  int fd;
  struct stat sb;
  fd = open(path, O_RDONLY);
  if (fd == -1) {
    perror("file open");
    return 1;
  }
  if (fstat(fd, &sb) == -1) {
    perror("fstat");
    close(fd);
    return 2;
  }

  *mapped = mmap(NULL, sb.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
  if (*mapped == MAP_FAILED) {
    perror("mmap");
    close(fd);
    return 3;
  }

  return 0;
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
    request->path = strdup(++path);
    request->version = strdup(version);
    request->query = NULL;
    return;
  }

  char *del = strchr(path, '?');
  int len = del - path;
  // TODO: double check this pointer arithmatic when I'm not tired af
  char *just_path = (char *)malloc(len);
  strncpy(just_path, path + 1, len - 1);

  char *unp_args = (char *)malloc(strlen(path) - len + 1);
  strncpy(unp_args, path + len, strlen(path) - len);
  unp_args++; // get rid of / in string

  char *query = (char *)malloc(strlen(unp_args));
  query = strdup(unp_args);

  request->query = strdup(query);
  request->method = strdup(method);
  request->path = strdup(just_path); // ofset to get rid of `\`
  request->version = strdup(version);
  // request->params
}

void doChildProcess(int fd, char **args, char **envp) {
  if ((dup2(fd, 1) == -1) || (dup2(fd, 2) == -1)) {
    perror("dup2");
    exit(1);
  }
  if (execve("fib.cgi", args, envp) == -1) {
    perror("execvp");
    exit(1);
  }
}

void getHTTPReq(int fd, char **req) {
  *req = malloc(1000);
  int numbytes = 0;
  char *end_of_hdr;
  int bytes_recvd = 0;
  while (1) {
    bytes_recvd = recv(fd, *req, MAXDATASIZE, MSG_PEEK);
    if (bytes_recvd < 0) {
      perror("recv");
      exit(1);
    } else {
      end_of_hdr = strstr(*req, "\r\n\r\n");
      if (end_of_hdr != NULL) {
        break;
      }
    }
  }
  bytes_recvd = recv(fd, *req, MAXDATASIZE, 0);
  if (bytes_recvd < 0) {
    perror("recv");
    exit(1);
  }
}

int has_adjacent_periods(const char *str) {
  while (*str) {
    if (*str == '.') {
      if (*(str + 1) == '.') {
        return 1;
      }
    }
    str++;
  }
  return 0;
}

void *consumer(void *args) {
  while (1) {
    sem_wait(&semFull);
    pthread_mutex_lock(&mutexBuffer);
    file_descriptors *fds = buffer[buff_count - 1];
    pthread_mutex_unlock(&mutexBuffer);
    sem_post(&semEmpty);

    char *incoming_msg;
    getHTTPReq(fds->conn_fd, &incoming_msg);

    // Parse HTTP req
    HttpRequest req;
    parse_http_request(incoming_msg, &req);

    // TODO: Refactor this error checking -> doesn't all need to be in main
    if (strcmp(req.method, "GET")) {
      char *resp;
      char *msg = "server does not implement this method\n";
      char *status = "501";
      alloc_http_msg(&resp, msg, status, strlen(msg));
      send_msg(resp, fds->conn_fd);
      printf("get\n");
      free(resp);
      continue;
    }
    if (strcmp(req.version, "HTTP/1.1")) {
      char *resp;
      char *msg = "server does not support this version\n";
      char *status = "502";
      alloc_http_msg(&resp, msg, status, strlen(msg));
      send_msg(resp, fds->conn_fd);
      printf("version\n");
      free(resp);
      continue;
    }

    if (has_adjacent_periods(req.path)) {
      char *resp;
      char *msg = "server could not read this file\n";
      char *status = "403";
      alloc_http_msg(&resp, msg, status, strlen(msg));
      send_msg(resp, fds->conn_fd);
      free(resp);
      continue;
    }

    char *file = NULL;
    char *args[] = {"./fib.cgi", NULL};
    char *q_string;
    if (req.query != NULL) {
      q_string = (char *)malloc(strlen("QUERY_STRING=") + strlen(req.query));
      strcpy(q_string, "QUERY_STRING=");
      strcat(q_string, req.query);
    } else {
      q_string = "QUERY_STRING=NULL";
    }
    char *envp[] = {q_string, NULL};
    if (!strcmp(req.path, "fib.cgi")) {
      pid_t pid;
      int status;
      pid = fork();
      if (!pid) {
        if (close(fds->portal_fd) == -1) {
          perror("close");
          exit(1);
        }
        doChildProcess(fds->conn_fd, args, envp);
      }
      waitpid(pid, &status, 0);
    } else {
      serve_static(fds->conn_fd, &req);
    }
    close(fds->conn_fd);
    if (req.path)
      free(req.path);
    if (req.query)
      free(req.query);
    if (req.method)
      free(req.method);
    if (req.version)
      free(req.version);
    free(fds);
  }
}
