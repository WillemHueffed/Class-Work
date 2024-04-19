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
#define MAX_PARAM_LEN 1000

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
  char *query;
  key_val **args;
} HttpRequest;

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
    request->path = path;
    request->version = version;
    return;
  }

  char *del = strchr(path, '?');
  int len = del - path;
  char *just_path = (char *)malloc(len);
  strncpy(just_path, path + 1, len - 1);

  char *unp_args = (char *)malloc(strlen(path) - len + 1);
  strncpy(unp_args, path + len, strlen(path) - len);
  unp_args++; // get rid of / in string

  int argc = 1;
  char *query = (char *)malloc(strlen(unp_args));
  query = strdup(unp_args);
  printf("The query is: %s\n", query);
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

  request->query = query;
  request->method = method;
  request->path = just_path;
  request->version = version;
  request->args = args;
  request->argc = argc;
}

int main() {

  char req[] = "GET /fib.cgi?x=1&y=2&zhu=hello HTTP/1.1\r\n"
               "Host: cs2.seattleu.edu\r\n"
               "\r\n";

  char req1[] = "GET /../fib.cgi HTTP/1.1\r\n"
                "Host: cs2.seattleu.edu\r\n"
                "\r\n";
  HttpRequest parsed_req;
  parse_http_request(req, &parsed_req);

  printf("path: %s\n", parsed_req.path);
  printf("method: %s\n", parsed_req.method);
  printf("params: %s\n", parsed_req.params);
  printf("version: %s\n", parsed_req.version);
  printf("argc is: %d\n", parsed_req.argc);
  printf("query is: %s\n", parsed_req.query);
  for (int i = 0; i < parsed_req.argc; i++) {
    printf("%s : %s\n", parsed_req.args[i]->key, parsed_req.args[i]->val);
  }
}
