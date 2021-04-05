#include <iostream>
#include <cxxopts.hpp>
#include <server/Bridge.h>
#include <server/MessageServer.h>
#include <message/MessageParser.h>

#include "SessionInfo.hpp"

class TerminusServerApplication : private MessageServer {
private:
  cxxopts::Options fOptions;
  MessageParser::Ptr fMessageParser;
  std::string fServerLogin;
  std::string fServerKey;
  std::string fServerAddress;
  int fServerPort = -1;
  bool fVerbose = false;
  std::map<std::string, SessionInfo> fSessionInfoMap;
public:
  TerminusServerApplication() :
    MessageServer(),
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
    if (result) {
      DCRITICAL("%s", fOptions.help().c_str());
      return result;
    }
    if (fVerbose) Logger::init(Logger::LogLevel::LogLevelDebug);
    fMessageParser = std::make_shared<MessageParser>(fServerLogin, fServerKey);
    listen(fServerAddress.c_str(), fServerPort);
    return 0;
  }

private:
  bool onData(const Buffer &data, const char *client) override {
    auto parseResult = fMessageParser->parse(data.getDataPtr(), data.getSize());
    if (!parseResult) {
      DERROR("failed to parse incoming data");
      return false;
    }
    switch (parseResult->getId()) {
      case ResponseMessage::id:
        DINFO("received response msg");
        return responseMessageHandler();
      case ConnectMessage::id:
        DINFO("received connect msg");
        return connectMessageHandler(client,parseResult);
    }
    return processClientMessage(client, parseResult->getBuffer());
  }

  void onClientDisconnected(const char *client) override {
    DERROR("client %s disconnected", client);
  }

  bool responseMessageHandler() {
    return false;
  }

  bool connectMessageHandler(const char *client, std::shared_ptr<Message> &parseResult) const {
    if (parseResult->cast<ConnectMessage>().getConnectOptions().getConnectionType() == ConnectionType::TypeMaster) {

    }


    return true;
  }

  bool processClientMessage(const char *client, const Buffer &buffer) {
    return false;
  }

  int parseOptions(int argc, char **argv, int &port, std::string &host, bool &verbose, std::string &serverLogin, std::string &serverKey) {
    try {
      auto result = fOptions.parse(argc, argv);
      port = result["port"].as<int>();
      host = result["address"].as<std::string>();
      verbose = result["verbose"].as<bool>();
      serverLogin = result["login"].as<std::string>();
      serverKey = result["key"].as<std::string>();
    } catch(...) {
      return -1;
    }
    return 0;
  }
};

int main(int argc, char **argv) {
  TerminusServerApplication serverApplication;
  return serverApplication.process(argc, argv);
}
