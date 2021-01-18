#ifndef TERMINUS_MESSAGECLIENT_H
#define TERMINUS_MESSAGECLIENT_H

#include <boost/asio.hpp>

class MessageClient {
private:
  using namespace boost::asio;
  using ip::tcp;
  using std::string;
  using std::cout;
  using std::endl;
public:
  MessageClient(const std::string &url) {

  }

private:

};


#endif //TERMINUS_MESSAGECLIENT_H
