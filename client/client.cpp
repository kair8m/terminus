#include <iostream>
#include <cxxopts.hpp>
#include <logger/Logger.h>
#include <terminal/console.hpp>
#include <terminal/terminal.hpp>
#include <client/MessageClient.h>
#include <message/MessageParser.h>

class ClientConsole : public Console {

};

class ShellTerminal : public Terminal {

};

class TerminusClientApplication {
private:
  std::shared_ptr<ShellTerminal> fShellTerminal;
  std::shared_ptr<ClientConsole> fClientConsole;
  std::shared_ptr<MessageClient> fMessageClient;
  cxxopts::Options fOptions;
  int fServerPort;
  bool fVerbose;
  std::string fServerAddress;
  std::string fApplicationType;
  std::string fServerLogin;
  std::string fServerKey;
  std::string fClientId;
public:

  TerminusClientApplication() : fOptions("Terminus client") {
    fOptions.add_options()
      ("v,verbose", "enable verbose output", cxxopts::value<bool>())
      ("p,port", "server port", cxxopts::value<int>())
      ("a,address", "server address", cxxopts::value<std::string>())
      ("t,type", "client type (master/slave)", cxxopts::value<std::string>())
      ("l,login", "server login", cxxopts::value<std::string>())
      ("k,key", "server key", cxxopts::value<std::string>())
      ("i,identifier", "specify client id", cxxopts::value<std::string>())
      ("b,buffer-size", "specify buffer size", cxxopts::value<int>());
  }

  int process(int argc, char **argv) {
    auto result = parseOptions(argc, argv, fServerPort, fServerAddress, fVerbose,
                               fServerLogin, fServerKey, fApplicationType, fClientId);
    if (!result) {
      DCRITICAL("%s", fOptions.help().c_str());
      return -1;
    }

    fMessageClient = std::make_shared<MessageClient>();

    fMessageClient->connect(fServerAddress, fServerPort);

    ConnectOptions opts(fApplicationType == "master" ? ConnectionType::TypeMaster : ConnectionType::TypeSlave, fClientId);

    ConnectMessage::Ptr connectMessage = MessageFactory::create<ConnectMessage>(opts);
    auto msg = MessageFactory::create<EncryptedMessage>(connectMessage, fServerLogin, fServerKey);
    fMessageClient->sendMsg((char *) msg->getBuffer().getDataPtr(), msg->getBuffer().getSize());

    return 0;
  }

private:

  bool parseOptions(int argc, char **argv, int &port, std::string &serverAddress, bool &verbose,
                    std::string &serverLogin, std::string &serverKey, std::string &applicationType, std::string &clientId) {
    try {
      auto result = fOptions.parse(argc, argv);
      port = result["port"].as<int>();
      serverAddress = result["address"].as<std::string>();
      verbose = result["verbose"].as<bool>();
      serverLogin = result["login"].as<std::string>();
      serverKey = result["key"].as<std::string>();
      applicationType = result["type"].as<std::string>();
      clientId = result["identifier"].as<std::string>();
      if (applicationType != "master" && applicationType != "slave") return false;
    } catch (...) {
      return false;
    }
    return true;
  }
};

int main(int argc, char **argv) {
  TerminusClientApplication clientApplication;
  return clientApplication.process(argc, argv);
}
