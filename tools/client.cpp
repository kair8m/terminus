#include <iostream>
#include <cxxopts.hpp>
#include <logger/Logger.h>
#include <terminal/console.hpp>
#include <terminal/terminal.hpp>
#include <client/Master.h>
#include <client/Slave.h>
#include <client/MessageClient.h>

class ClientConsole : public Console {

};

class ShellTerminal : public Terminal {

};

class TerminusClientApplication {
private:
  std::shared_ptr<ShellTerminal> fShellTerminal;
  std::shared_ptr<ClientConsole> fClientConsole;
public:
  TerminusClientApplication() {

  }

  int process(int argc, char **argv) {
    return 0;
  }
};

int main(int argc, char **argv) {
  TerminusClientApplication clientApplication;
  return clientApplication.process(argc, argv);
}
