#ifndef RESPONSE_H
#define RESPONSE_H

#include "socket.h"
#include <string>
#include <map>
#include <vector>

struct Response
{
  std::string status;
  std::map<std::string, std::string> headers;
  std::vector<char> body;
  int contentLength = 0;

  void setStatus(const std::string &s);
  void setStatus(int code);

  void addHeader(const std::string &key, const std::string &value);

  void setBody(const char *data, int size);
  void setBody(const std::string &s);
  void setBody(const std::vector<char> &v);

  void send(Socket *socket);

  static void send304(Socket *socket, std::vector<std::pair<std::string, std::string>> headers = std::vector<std::pair<std::string, std::string>>());

  static void send400(Socket *socket, std::vector<std::pair<std::string, std::string>> headers = std::vector<std::pair<std::string, std::string>>());

  static void send404(Socket *socket, std::vector<std::pair<std::string, std::string>> headers = std::vector<std::pair<std::string, std::string>>());

};

#endif
