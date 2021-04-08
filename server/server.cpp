#include <iostream>
#include <cxxopts.hpp>
#include <server/MessageServer.h>

class TerminusServerApplication {
private:
  cxxopts::Options fOptions;
  std::string fServerLogin;
  std::string fServerKey;
  std::string fServerAddress;
  int fServerPort = -1;
  bool fVerbose = false;
  std::shared_ptr<MessageServer> fMessageServer = nullptr;
public:
  TerminusServerApplication() :
    fOptions("Terminus server") {
    fOptions.add_options()
      ("v,verbose", "enable verbose output", cxxopts::value<bool>())
      ("p,port", "specify port", cxxopts::value<int>())
      ("a,address", "specify listening address", cxxopts::value<std::string>())
      ("l,login", "specify server login", cxxopts::value<std::string>())
      ("k,key", "specify server key", cxxopts::value<std::string>())
      ("m,max-connections", "specify maximum amount of established connections", cxxopts::value<int>())
      ("b,buffer-size", "specify buffer size", cxxopts::value<int>())
      ("t,tcp-no-delay", "enable tcp no delay", cxxopts::value<bool>());
  }

  int process(int argc, char **argv) {
    auto result = parseOptions(argc, argv, fServerPort, fServerAddress, fVerbose, fServerLogin, fServerKey);
    if (!result) {
      DCRITICAL("%s", fOptions.help().c_str());
      return -1;
    }
    if (fVerbose) Logger::init(Logger::LogLevel::LogLevelDebug);

    fMessageServer = std::make_shared<MessageServer>(fServerLogin, fServerKey);

    fMessageServer->enableKeepAlive(10);

    fMessageServer->listen(fServerAddress.c_str(), fServerPort);
    return 0;
  }

private:

  bool parseOptions(int argc, char **argv, int &port, std::string &host, bool &verbose, std::string &serverLogin, std::string &serverKey) {
    try {
      auto result = fOptions.parse(argc, argv);
      port = result["port"].as<int>();
      host = result["address"].as<std::string>();
      verbose = result["verbose"].as<bool>();
      serverLogin = result["login"].as<std::string>();
      serverKey = result["key"].as<std::string>();
    } catch (...) {
      return false;
    }
    return true;
  }
};

int main(int argc, char **argv) {
  TerminusServerApplication serverApplication;
  return serverApplication.process(argc, argv);
}
