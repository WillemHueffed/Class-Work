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

#define MAX_METHOD_LEN 10
#define MAX_PATH_LEN 100
#define MAX_VERSION_LEN 10

typedef struct {
  char method[MAX_METHOD_LEN];
  char path[MAX_PATH_LEN];
  char version[MAX_VERSION_LEN];
} HttpRequest;

int parse_http_request(const char *request_str, HttpRequest *request) {
  // Find the end of the request line
  const char *end_of_request_line = strchr(request_str, '\n');
  if (!end_of_request_line) {
    printf("Invalid request format: No end of request line found.\n");
    return -1;
  }

  // Parse the request line
  if (sscanf(request_str, "%9s %99s %9s", request->method, request->path,
             request->version) != 3) {
    printf("Invalid request format: Unable to parse request line.\n");
    return -1;
  }

  return 0;
}

int main() {
  const char *req = "GET index.html HTTP/1.1\r\n"
                    "Host: cs2.seattleu.edu\r\n"
                    "\r\n";
  HttpRequest parsed_req;
  parse_http_request(req, &parsed_req);
  // printf("method: %s\npath: %s\nversion: %s\n", parsed_req.method,
  //      parsed_req.path, parsed_req.version);

  char *header;
  header = (char *)malloc(1000);
  const char *status = "200 OK";
  const char *date = "4/16/24";
  int content_length = 420;
  const char *content_type = "text/html";
  snprintf(header, 1000,
           "HTTP/1.1 %s\r\n"
           "Connection: close\r\n"
           "Date: %s\r\n"
           "Content-Length: %d\r\n"
           "Content-Type: %sr\n"
           "Server: cpsc4510 web server 1.0\r\n"
           "\r\n",
           status, date, content_length, content_type);
  printf("%s", header);
}
