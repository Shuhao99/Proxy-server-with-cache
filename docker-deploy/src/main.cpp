#include "proxy_server.h"

int main() {
  const char * port = "12345";
  proxy_server * my_server = new proxy_server(port);
  my_server->run();
  return 0;
}