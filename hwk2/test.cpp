#include "cache.hpp"
#include <iostream>
int main() {
  response *res = new response();
  Cache *c = new Cache();
  std::cout << c->checkIfStale(*res) << std::endl;
  return 0;
}
