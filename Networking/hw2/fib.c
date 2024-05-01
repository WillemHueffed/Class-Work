// Credit: https://www.geeksforgeeks.org/c-fibonacci-series/#

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
  char *key;
  char *val;
} key_val;

int isConvertible(char *str) {
  if (!str || !*str) {
    return 1;
  }
  for (const char *c = str; *c; ++c) {
    if (!isdigit(*c)) {
      return 1;
    }
  }
  int number = atoi(str);
  if (number < 0 || number > 10000) {
    return 1;
  }
  return 0;
}

void alloc_http_msg(char **http_resp, char *body, char *status,
                    int content_length) {
  char *header = (char *)malloc(1000);
  const char *date = "4/16/24";
  const char *content_type = "text/html";
  snprintf(header, 2000,
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
}

void fibN(int n, char *user, char **msg) {
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
  sprintf(
      *msg,
      "%s, welcome to the CGI Program!\nThe %dth Fibonacci number is %lld.\n",
      user, n, sum);
}

int main() {
  char *unp_args = getenv("QUERY_STRING");
  // char unp_args[] = "user=willem&n=2wo";

  if (!strcmp(unp_args, "NULL")) {
    char *e_msg = "error args null\n";
    char *status = "400 Bad Request";
    char *http_resp;
    alloc_http_msg(&http_resp, e_msg, status, strlen(e_msg));
    printf("%s", http_resp);
    free(http_resp);
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
    char *del_pos = strchr(tok, '=');
    int kl = del_pos - tok;
    char *key = (char *)malloc(kl + 1);
    key[kl] = '\0';
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
    char *status = "400 Bad Request";
    char *http_resp;
    alloc_http_msg(&http_resp, e_msg, status, strlen(e_msg));
    printf("%s", http_resp);
    for (int i = 0; args[i]; i++) {
      if (args[i]->key)
        free(args[i]->key);
      if (args[i]->val)
        free(args[i]->val);
    }
    free(args);
    free(http_resp);
    exit(1);
  }

  char *n = NULL;
  char *user = NULL;

  for (int i = 0; i < argc; i++) {
    if (args[i] != NULL) {
      if (!strcmp(args[i]->key, "user")) {
        user = strdup(args[i]->val);
      }
      if (!strcmp(args[i]->key, "n")) {
        n = strdup(args[i]->val);
      }
    }
  }

  if (!n || !user) {
    char *e_msg = "error only \"user\" and \"n\" params allowed\n";
    char *status = "400 Bad Request";
    char *http_resp;
    alloc_http_msg(&http_resp, e_msg, status, strlen(e_msg));
    printf("%s", http_resp);
    free(n);
    free(user);
    free(http_resp);
    for (int i = 0; args[i]; i++) {
      if (args[i]->key)
        free(args[i]->key);
      if (args[i]->val)
        free(args[i]->val);
    }
    free(args);
    exit(1);
  }

  // Validate n's value
  if (isConvertible(n) != 0) {
    char *e_msg = "n must be an positive integer < 10000\n";
    char *status = "500 Internal Server Error";
    char *http_resp;
    alloc_http_msg(&http_resp, e_msg, status, strlen(e_msg));
    printf("%s", http_resp);
    exit(1);
  }

  char *body;
  fibN(atoi(n), user, &body);
  char *status = "200 OK";
  char *http_resp;
  alloc_http_msg(&http_resp, body, status, strlen(body));
  printf("%s", http_resp);
  if (unsetenv("QUERY_STRING") != 0) {
    perror("unsetenv");
    return 1;
  }
  free(n);
  free(user);
  for (int i = 0; args[i]; i++) {
    if (args[i]->key)
      free(args[i]->key);
    if (args[i]->val)
      free(args[i]->val);
  }
  free(args);
  return 0;
}
