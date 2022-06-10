#ifndef REQUEST_H
#define REQUEST_H

#include <string>
#include <map>
#include <vector>

struct Request
{
  std::string query;
  std::string method;
  std::string uri;
  std::string protocol;
  std::map<std::string, std::string> headers;
  std::vector<char> body;
  int contentLength = 0;

  void clear();
  bool setQuery(const std::string &query);
  bool addHeader(const std::string &header);
  bool hasHeader(const std::string &header);
};

#endif
