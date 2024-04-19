#include <arpa/inet.h>
#include <getopt.h>
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

void setup(int *sockfd, char *hostname, char *port) {
  struct addrinfo hints, *servinfo, *p;
  int rv;
  char s[INET6_ADDRSTRLEN];

  memset(&hints, 0, sizeof(hints));
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  if ((rv = getaddrinfo(hostname, port, &hints, &servinfo)) != 0) {
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
}

void extract_host_path(char *server, char **host, char **path) {
  char *del = strchr(server, '/');
  if (del != NULL) {
    int hostname_length = del - server;
    *host = (char *)malloc(hostname_length + 1);
    strncpy(*host, server, hostname_length);
    (*host)[hostname_length] = '\0';

    *path = (char *)malloc(strlen(del));
    *path = strdup(del);

  } else {
    perror("Delimiter not found in the URL\n");
    exit(1);
  }
}

void print_usage() {
  perror("Error: call the script like \"./client [-s server] [-p port]\"\n");
}

void compose_http_req(char **msg, const char *path, const char *hostname) {
  char buf[5000];
  sprintf(buf,
          "GET %s HTTP/1.1\r\n"
          "Host: %s\r\n"
          "\r\n",
          path, hostname);
  *msg = strdup(buf);
}

int main(int argc, char **argv) {
  if (argc != 5) {
    print_usage();
    return (1);
  }
  int option;
  int sflag = 0;
  int pflag = 0;
  char *server;
  char *port;

  while ((option = getopt(argc, argv, "s:p:")) != -1) {
    switch (option) {
    case 's':
      sflag++;
      server = strdup(optarg);
      break;
    case 'p':
      pflag++;
      port = strdup(optarg);
      break;
    default:
      print_usage();
      exit(1);
    }
  }
  if (sflag && !pflag) {
    free(server);
  }
  if (pflag && !sflag) {
    free(port);
  }

  char *hostname;
  char *path;
  extract_host_path(server, &hostname, &path);
  char *req;
  compose_http_req(&req, path, hostname);

  int sockfd;
  int numbytes;
  char buf[MAXDATASIZE];

  setup(&sockfd, hostname, port);
  free(hostname);
  free(path);
  // at this point the only thing on heap should be the http req

  int bytes_sent = 0;
  while (bytes_sent < sizeof(req)) {
    int x = send(sockfd, req, strlen(req) - bytes_sent, 0);
    if (x == -1 || x == 0) {
      perror("send");
      free(req);
      close(sockfd);
      exit(1);
    }
    req += x;
    bytes_sent += x;
  }
  req -= bytes_sent;
  free(req);

  char *resp;
  getHTTPReq(sockfd, &resp);
  printf("%s", resp);
  free(resp);
  if (close(sockfd) == -1) {
    perror("close");
    return 1;
  }
  return 0;
}
