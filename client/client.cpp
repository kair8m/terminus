#include <iostream>
#include <cxxopts.hpp>
#include <logger/Logger.h>
#include <terminal/console.hpp>
#include <terminal/terminal.hpp>
#include <client/MessageClient.h>
#include <message/MessageParser.h>

class TerminusClientApplication {
private:
  std::shared_ptr<Terminal> fShellTerminal;
  std::shared_ptr<Console> fClientConsole;
  std::shared_ptr<MessageClient> fMessageClient;
  std::shared_ptr<MessageParser> fMessageParser;
  cxxopts::Options fOptions;
  int fServerPort;
  bool fVerbose;
  std::string fServerAddress;
  std::string fApplicationType;
  std::string fServerLogin;
  std::string fServerKey;
  std::string fClientId;
  bool fReset = false;
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

    if (!sendConnect()) {
      DERROR("failed to connect to server");
      return -1;
    }

    fMessageParser = std::make_shared<MessageParser>(fServerLogin, fServerKey);

    processSession();

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

  bool sendConnect() {
    fMessageClient->connect(fServerAddress, fServerPort, false);

    ConnectOptions opts(fApplicationType == "master" ? ConnectionType::TypeMaster : ConnectionType::TypeSlave, fClientId);

    ConnectMessage::Ptr connectMessage = MessageFactory::create<ConnectMessage>(opts);
    auto msg = MessageFactory::create<EncryptedMessage>(connectMessage, fServerLogin, fServerKey);
    return fMessageClient->sendData((char *) msg->getBuffer().getDataPtr(), msg->getBuffer().getSize());
  }

  void processSession() {
    if (fApplicationType == "master") {
      processMasterSession();
      return;
    }
    processSlaveSession();
  }

  void processSlaveSession() {
    fShellTerminal = std::make_shared<Terminal>(80, 80, true);
    fShellTerminal->open(false);
    std::thread recvThread(&TerminusClientApplication::slaveReceive, this);

    std::thread sendTread(&TerminusClientApplication::slaveSend, this);

    sendTread.join();
    fMessageClient.reset();
    fShellTerminal.reset();
    recvThread.join();
  }

  void slaveSend() {
    while (!fReset) {
      auto buffer = fShellTerminal->receive();
      if (buffer.getSize() == 0) break;
      PutCharMessage::Ptr putCharMessage = MessageFactory::create<PutCharMessage>(
        std::string((char *) buffer.getDataPtr(), buffer.getSize()));
      EncryptedMessage::Ptr encrypted = MessageFactory::create<EncryptedMessage>(putCharMessage, fServerLogin, fServerKey);
      if (!fMessageClient->sendData((char *) encrypted->getBuffer().getDataPtr(), encrypted->getBuffer().getSize())) break;
    }
    fReset = true;
  }

  void slaveReceive() {
    static const std::map<uint32_t, std::function<void(Message &)>> messageMap = {
      {ResizeTerminalMessage::id, [&](Message &msg) {
        auto resizeMsg = msg.cast<ResizeTerminalMessage>();
        fShellTerminal->setSize((int) resizeMsg.getWidth(), (int) resizeMsg.getHeight());
      }},
      {PutCharMessage::id,        [&](Message &msg) {
        auto putCharMsg = msg.cast<PutCharMessage>();
        fShellTerminal->write(putCharMsg.getChars());
      }},
    };
    while (!fReset) {
      auto buffer = fMessageClient->receiveData();
      if (buffer.getSize() == 0) break;
      auto result = fMessageParser->parse(buffer.getDataPtr(), buffer.getSize());
      if (!result) break;
      auto item = messageMap.find(result->getId());
      if (item == messageMap.end()) break;
      Message &msg = *result;
      item->second(msg);
    }
    fReset = true;
  }

  void processMasterSession() {
    fClientConsole = std::make_shared<Console>();
    fClientConsole->setup();

    std::thread recvThread(&TerminusClientApplication::masterReceive, this);

    std::thread sendTread(&TerminusClientApplication::masterSend, this);

    sendTread.join();
    fMessageClient.reset();
    fClientConsole.reset();
    recvThread.join();
  }

  void masterReceive() {
    const static std::map<uint32_t, std::function<void(Message &)>> messageMap = {
      {PutCharMessage::id, [&](Message &msg) {
        auto putCharMsg = msg.cast<PutCharMessage>();
        fClientConsole->display(putCharMsg.getChars());
      }}
    };
    while (!fReset) {
      auto buffer = fMessageClient->receiveData();
      if (buffer.getSize() == 0) break;
      auto result = fMessageParser->parse(buffer.getDataPtr(), buffer.getSize());
      if (!result) break;
      auto item = messageMap.find(result->getId());
      if (item == messageMap.end()) break;
      Message &msg = *result;
      item->second(msg);
    }
    fReset = true;
  }

  void masterSend() {
    while (!fReset) {
      auto buffer = fClientConsole->read();
      if (buffer.getSize() == 0) break;
      PutCharMessage::Ptr putCharMessage = MessageFactory::create<PutCharMessage>(
        std::string((char *) buffer.getDataPtr(), buffer.getSize()));
      EncryptedMessage::Ptr encrypted = MessageFactory::create<EncryptedMessage>(putCharMessage, fServerLogin, fServerKey);
      if (!fMessageClient->sendData((char *) encrypted->getBuffer().getDataPtr(), encrypted->getBuffer().getSize())) break;
    }
    fReset = true;
  }
};

int main(int argc, char **argv) {
  TerminusClientApplication clientApplication;
  return clientApplication.process(argc, argv);
}
