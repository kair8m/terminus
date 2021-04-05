#include <iostream>
#include <cxxopts.hpp>
#include <server/Bridge.h>
#include <server/MessageServer.h>
#include <message/MessageParser.h>

class TerminusServerApplication : private MessageServer {
private:
  cxxopts::Options options;
public:
  TerminusServerApplication() :
    MessageServer(),
    options("Terminus server") {
    options.add_options()
      ("v,verbose", "enable verbose output", cxxopts::value<bool>())
      ("p,port", "specify port", cxxopts::value<int>())
      ("a,address", "specify listening address", cxxopts::value<std::string>())
      ("m,max-connections", "specify maximum amount of established connections", cxxopts::value<int>())
      ("b,buffer-size", "specify buffer size", cxxopts::value<int>())
      ("t,tcp-no-delay", "enable tcp no delay", cxxopts::value<bool>());
  }

  int process(int argc, char **argv) {
    int port = -1;
    std::string host = {};
    bool verbose = false;
    auto result = parseOptions(argc, argv, port, host, verbose);
    if (verbose) Logger::init(Logger::LogLevel::LogLevelDebug);
    if (result) return result;
    listen(host.c_str(), port);
    return 0;
  }

private:
  bool onData(const Buffer &data) override {
    auto parseResult = MessageParser::parse(data.getDataPtr(), data.getSize());
    if (!parseResult) {
      DERROR("failed to parse incoming data");
      return false;
    }
    switch (parseResult->getId()) {
      case PutCharMessage::id:
        DINFO("received put char msg");
        break;
      case ResizeTerminalMessage::id:
        DINFO("received resize terminal msg");
        break;
      case ResponseMessage::id:
        DINFO("received response msg");
        break;
      case ConnectMessage::id:
        DINFO("received connect msg");
        break;
    }
    return false;
  }

  int parseOptions(int argc, char **argv, int &port, std::string &host, bool &verbose) {
    try {
      auto result = options.parse(argc, argv);
      port = result["port"].as<int>();
      host = result["address"].as<std::string>();
      verbose = result["verbose"].as<bool>();
    } catch (const cxxopts::missing_argument_exception &ex) {
      DCRITICAL("%s", ex.what());
      return -1;
    } catch (const cxxopts::option_has_no_value_exception &ex) {
      DCRITICAL("%s", ex.what());
      return -1;
    } catch (const cxxopts::option_not_exists_exception &ex) {
      DCRITICAL("%s", ex.what());
      return -1;
    } catch (const cxxopts::option_not_has_argument_exception &ex) {
      DCRITICAL("%s", ex.what());
      return -1;
    } catch (const cxxopts::option_not_present_exception &ex) {
      DCRITICAL("%s", ex.what());
      return -1;
    } catch (const cxxopts::option_required_exception &ex) {
      DCRITICAL("%s", ex.what());
      return -1;
    } catch (const cxxopts::option_requires_argument_exception &ex) {
      DCRITICAL("%s", ex.what());
      return -1;
    } catch (const cxxopts::option_syntax_exception &ex) {
      DCRITICAL("%s", ex.what());
      return -1;
    }
    return 0;
  }
};

int main(int argc, char **argv) {
  TerminusServerApplication serverApplication;
  return serverApplication.process(argc, argv);
}
