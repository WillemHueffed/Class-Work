#include <functional>
#include <iostream>
#include <string>
using namespace std;
int allocate_port(const string &student_id) {
  // Use std::hash function to hash the student ID
  size_t hash_value = hash<string>{}(student_id);
  // Map the hash value to the range of 0-99
  int port_number = static_cast<int>(hash_value % 100);
  // Map the port number to the range of 10401-10500
  port_number += 10401;
  return port_number;
}

int main(int argc, char *argv[]) {
  string id = string(argv[1]);
  cout << allocate_port(id) << endl;
  return 0;
}
