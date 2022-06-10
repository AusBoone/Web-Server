#include "response.h"
#include "utils.h"

#include <string>
#include <map>
#include <vector>
#include <algorithm>
#include <ctime>
using namespace std;

static const string SERVER_NAME = "CSCI 330 My Own Web Server";

static const string NOT_FOUND_BODY =
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\"><html>"
    "<title>Page Not Found</title>"
    "<body>"
    "<h1>Page Not Found</h1>"
    "<p>Requested resource is not found on the server.</p>"
    "</body></html>";

static const string BAD_REQUEST_BODY =
    "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\"><html>"
    "<title>Bad Request</title>"
    "<body>"
    "<h1>Bad Request</h1>"
    "<p>The request could not be understood by the server due to malformed syntax.</p>"
    "</body></html>";

void Response::setStatus(const std::string &s)
{
  this->status = s;
}

void Response::setStatus(int code)
{
  switch (code)
  {
  case 200:
    this->status = "HTTP/1.1 200 OK";
    break;
  case 304:
    this->status = "HTTP/1.1 304 Not Modified";
    break;
  case 400:
    this->status = "HTTP/1.1 400 Bad Request";
    break;
  case 404:
    this->status = "HTTP/1.1 404 Not Found";
    break;
  default:
    break;
  }
}

void Response::addHeader(const std::string &key, const std::string &value)
{
  headers.insert(make_pair(key, value));
}

void Response::setBody(const std::string &s)
{
  setBody(s.data(), s.size());
}

void Response::setBody(const std::vector<char> &v)
{
  setBody(v.data(), v.size());
}

void Response::setBody(const char *data, int size)
{
  body.assign(data, data + size);
  contentLength = size;
  addHeader("Content-Length", to_string(size));
}

void Response::send(Socket *socket)
{
  headers["Server"] = SERVER_NAME;
  headers["Date"] = formatDate(time(NULL));

  string headersBuffer;
  headersBuffer.append(status).append("\r\n");
  for (const auto &kv : headers)
  {
    headersBuffer.append(kv.first).append(": ").append(kv.second).append("\r\n");
  }
  headersBuffer.append("\r\n");

  int dataSize = headersBuffer.size() + contentLength;
  char *buffer = new char[headersBuffer.size() + contentLength];
  copy(headersBuffer.data(), headersBuffer.data() + headersBuffer.size(), buffer);
  copy(body.data(), body.data() + body.size(), buffer + headersBuffer.size());

  socket->send(buffer, dataSize);

  delete[] buffer;
}

void Response::send304(Socket *socket, std::vector<std::pair<std::string, std::string>> headers)
{
  Response response;
  response.setStatus(304);   
  for (const auto h: headers)
  {
    response.addHeader(h.first, h.second);
  }
  response.send(socket);
}

void Response::send400(Socket *socket, std::vector<std::pair<std::string, std::string>> headers)
{
  Response response;
  response.setStatus(400);
  response.addHeader("Content-Type", "text/html");  
  for (const auto h: headers)
  {
    response.addHeader(h.first, h.second);
  }
  response.addHeader("Connection", "close"); // close connection if malformed request received
  response.setBody(BAD_REQUEST_BODY);
  response.send(socket);
}

void Response::send404(Socket *socket, std::vector<std::pair<std::string, std::string>> headers)
{
  // send response
  Response response;
  response.setStatus(404);
  response.addHeader("Content-Type", "text/html");
  for (const auto h: headers)
  {
    response.addHeader(h.first, h.second);
  }
  response.setBody(NOT_FOUND_BODY);
  response.send(socket);
}
