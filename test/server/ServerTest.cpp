#include "gtest/gtest.h"
#include "server/MessageServer.h"

bool gotData = false;

class TestServer : public MessageServer {
  void onData(const Buffer &data) override {
    gotData = true;
  }
};

TEST(ServerTest, network) {
  TestServer messageServer;
  int port = 1234;
  std::thread([&] {
    messageServer.listen("127.0.0.1", port);
  }).detach();
  std::this_thread::sleep_for(std::chrono::seconds(1));
  system(R"(curl -X POST "http://localhost:1234/test" -d "" &)");
  std::this_thread::sleep_for(std::chrono::seconds(1));
  ASSERT_TRUE(gotData);
  messageServer.stop();
}

