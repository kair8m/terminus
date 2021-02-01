#ifndef TERMINUS_CONSOLE_HPP
#define TERMINUS_CONSOLE_HPP


class Console {
private:
  termios fSave;
  termios fWindow;
public:
  Console() {}

  bool setup() {

  }

  void write(const std::string &data) {

  }

protected:

  virtual void onData(const std::string &data) {
    (void) data;
  }
};

#endif //TERMINUS_CONSOLE_HPP
