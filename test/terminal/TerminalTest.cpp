#include "gtest/gtest.h"
#include "terminal/terminal.hpp"

class TestApplication {
public:
  TestApplication() {
    fTerminal = std::make_shared<Terminal>(80, 24, false);
  }

  void run() {

  }
private:
  std::shared_ptr<Terminal> fTerminal = nullptr;
};

TEST(TerminalTest, SingleCommandTest) {
  auto result = Terminal::execute("ls /");
  ASSERT_TRUE(result.first == 0);
  ASSERT_TRUE(!result.second.empty());
  std::cout << result.second << std::endl;
  Terminal terminal(80, 10, false);
}