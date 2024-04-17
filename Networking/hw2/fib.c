// Credit: https://www.geeksforgeeks.org/c-fibonacci-series/#

#include <stdio.h>
#include <stdlib.h>

void fibN(int n) {
  if (n < 1) {
    printf("Error, the fib function requires n >= 1");
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
  printf("Fib(%d) is %lld\n", n, sum);
}

int main() {
  const char *name = "FIB_N";
  const char *value = "5";
  if (setenv(name, value, 1) != 0) {
    perror("setenv");
    return 1;
  }
  const char *result = getenv(name);
  fibN(atoi(result));
  if (result != NULL) {
    printf("%s=%s\n", name, result);
  } else {
    printf("Environment variable %s not found\n", name);
  }
  if (unsetenv(name) != 0) {
    perror("unsetenv");
    return 1;
  }
  return 0;
}
