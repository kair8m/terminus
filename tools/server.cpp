#include <iostream>
#include <cxxopts.hpp>
#include <server/Bridge.h>
#include <server/MessageServer.h>

class TerminusServer : public MessageServer {

};

class TerminusServerApplication {
public:
  TerminusServerApplication() {

  }

  int process(int argc, char **argv) {
    return 0;
  }
};

int main(int argc, char **argv) {
  TerminusServerApplication serverApplication;
  return serverApplication.process(argc, argv);
}
