// Credit: https://www.geeksforgeeks.org/c-fibonacci-series/#

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

void fibN(int n, char **msg) {
  if (n < 1) {
    *msg = malloc(strlen("Error, the fib function requires n >= 1"));
    strcpy(*msg, "Error, the fib function requires n >= 1");
    return;
  }
  long long x = 0;
  long long y = 1;
  long long int sum = 1;
  for (int i = 2; i < n; i++) {
    sum = x + y;
    x = y;
    y = sum;
  }
  sum %= 1000000007;
  char buf[1000];
  *msg = malloc(1000);
  sprintf(*msg, "Fib(%d) is %lld\n", n, sum);
}

int main() {
  const char *name = "FIB_N";
  const char *value = "5";
  if (setenv(name, value, 1) != 0) {
    perror("setenv");
    return 1;
  }
  const char *result = getenv(name);
  char *body;
  fibN(atoi(result), &body);
  // TODO: Add logic to check if fibN is unhappy (n<1);
  char *status = "200 OK";
  char *http_resp;
  alloc_http_msg(&http_resp, body, status, strlen(body));
  printf("%s", http_resp);
  if (unsetenv(name) != 0) {
    perror("unsetenv");
    return 1;
  }
  return 0;
}
