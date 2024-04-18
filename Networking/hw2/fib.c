// Credit: https://www.geeksforgeeks.org/c-fibonacci-series/#

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *key;
  char *val;
} key_val;

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
  char *unp_args = getenv("QUERY_STRING");

  if (!strcmp(unp_args, "NULL")) {
    char *e_msg = "error args null\n";
    char *status = "200 OK";
    char *http_resp;
    // alloc_http_msg(&http_resp, body, status, strlen(body));
    alloc_http_msg(&http_resp, e_msg, status, strlen(e_msg));
    printf("%s", http_resp);
    exit(1);
  }

  int argc = 1;
  char *query = (char *)malloc(strlen(unp_args));
  query = strdup(unp_args);
  for (int i = 0; i < strlen(unp_args); i++) {
    if (unp_args[i] == '&')
      argc++;
  }
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

  if (argc != 2) {
    char *e_msg = "error only 2 args allowed\n";
    char *status = "200 OK";
    char *http_resp;
    // alloc_http_msg(&http_resp, body, status, strlen(body));
    alloc_http_msg(&http_resp, e_msg, status, strlen(e_msg));
    printf("%s", http_resp);
    exit(1);
  }

  char *body;
  fibN(atoi(unp_args), &body);
  // TODO: Add logic to check if fibN is unhappy (n<1);
  char *status = "200 OK";
  char *http_resp;
  // alloc_http_msg(&http_resp, body, status, strlen(body));
  alloc_http_msg(&http_resp, unp_args, status, strlen(body));
  printf("%s", http_resp);
  if (unsetenv("QUERY_STRING") != 0) {
    perror("unsetenv");
    return 1;
  }
  return 0;
}
