#include "gtest/gtest.h"
#include "terminal/terminal.hpp"

class TestApplication : public Terminal {
public:
  TestApplication(int width, int height, bool withAuthentication) : Terminal(width, height, withAuthentication) {
  }

  void onData(const std::string &data) override {
    ::write(STDOUT_FILENO, data.c_str(), data.size());
  }
};

TEST(TerminalTest, SingleCommandTest) {
  auto result = Terminal::execute("ls /");
  ASSERT_TRUE(result.first == 0);
  ASSERT_TRUE(!result.second.empty());
  std::cout << result.second << std::endl;
  TestApplication terminal(80, 10, false);
  ASSERT_TRUE(terminal.open());
  terminal.write("ls -a\n");
}