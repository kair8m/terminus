#include <stdio.h>
#include <iostream>

class TerminusClientApplication {
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