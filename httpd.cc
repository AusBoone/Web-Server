/* httpd.cc
 *
 * Austin Boone
 *
 * My Own Web Server
 * 
 */

#include "socket.h"
#include "fdset.h"
#include "request.h"
#include "response.h"
#include "utils.h"

#include <unistd.h>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>
#include <iomanip>
#include <sstream>
#include <vector>
#include <string>
#include <map>
using namespace std;

static const int PORT_NUMBER = 30000;
static const int MAX_CLIENTS = 10;

static const int INPUT_BUFFER_SIZE = 4096;

static const string WWW = "www"; // http root directory
static const string SERVER_NAME = "CSCI 330 My Own Web Server";

// Reading a request has three phases
enum Phase
{
  REQUEST, // the first line
  HEADER,  // the headers
  BODY     // the body
};

struct Client
{
  Socket *socket;

  Request request; // current request

  string currentLine;           // current line which is being read right now
  char *inputBuffer;            // buffer for reading from the socket
  Phase phase = Phase::REQUEST; // current phase of the reading
  time_t lastActivity;

  Client(Socket *socket = NULL) : socket(socket), inputBuffer(new char[INPUT_BUFFER_SIZE]) {}

  ~Client()
  {
    socket->close();
    delete socket;
    delete[] inputBuffer;
  }
  Client(const Client &) = delete;
  void operator=(const Client &) = delete;
};

bool addClient(Client *clients[], int &numClients, Client *client)
{
  if (numClients == MAX_CLIENTS)
  {
    cerr << "Refused client" << endl;
    delete client;
    return false;
  }
  cerr << "Added client" << endl;
  clients[numClients++] = client;
  client->lastActivity = time(NULL);
  return true;
}

Client *findClient(int fd, Client *clients[], int numClients)
{

  for (int i = 0; i < numClients; i++)
  {
    if (clients[i]->socket->fd() == fd)
    {
      return clients[i];
    }
  }
  return NULL;
}

bool removeClient(Client *clients[], int &numClients, Client *client, FDSet &inputSet)
{
  for (int i = 0; i < numClients; i++)
  {
    if (clients[i]->socket == client->socket)
    {
      cout << "Client " << client->request.query
           << " disconnected." << endl;

      clients[i] = clients[--numClients];
      inputSet.removeFD(client->socket->fd());
      delete client;
      return true;
    }
  }
  return false;
}

void removeTimedOutClients(Client *clients[], int &numClients, FDSet &inputSet)
{
  time_t now = time(NULL);
  int i = 0;
  while (i < numClients)
  {
    if (difftime(now, clients[i]->lastActivity) > 10)
    {
      cout << "Close connection to client " << clients[i]->request.query
           << " due timeout." << endl;
      removeClient(clients, numClients, clients[i], inputSet);
    }
    else
    {
      ++i;
    }
  }
}

// process the fully read request and send a response to the client
bool processRequest(Client *client, Request &request)
{
  client->phase = Phase::REQUEST;

  // print all headers (for debug)
  cout << request.query << endl;
  for (const auto kv : request.headers)
  {
    cout << kv.first << ": " << kv.second << endl;
  }

  // return false if client disconnected
  bool keepalive = true;
  string connection = "keep-alive";
  if (client->request.hasHeader("Connection") && client->request.headers["Connection"] == "close")
  {
    keepalive = false; // client want to close connection
    connection = "close";
  }
  vector<pair<string, string>> headers({make_pair("Connection", connection)});

  // security feature
  if (request.uri.find("../") != request.uri.npos)
  {
    request.clear();
    Response::send404(client->socket, headers); // security
    return keepalive;
  }

  // make full path to the requested resource
  string filename = makeFilename(request.uri);

  cout << endl
       << "Requested resource: " << request.uri << " (" << filename << ")" << endl;

  if (!isFileExist(filename))
  {
    request.clear();
    Response::send404(client->socket, headers);
    return keepalive;
  }

  time_t lastModified = getLastModified(filename);

  if (request.hasHeader("If-Modified-Since")) // process conditional request
  {
    time_t since = parseDate(request.headers["If-Modified-Since"]);

    if (difftime(lastModified, since) <= 0)
    {
      //cout << "Not modified " << lastModified << " <= " << since << endl;
      request.clear();
      Response::send304(client->socket, headers); // not modified
      return keepalive;
    }
    else
    {
      //cout << "Modified " << lastModified << " == " << since << endl;
    }
  }

  // send response
  Response response;
  response.setStatus(200);
  response.addHeader("Content-Type", contentTypeFromFilename(filename));
  response.addHeader("Last-Modified", formatDate(lastModified));
  response.addHeader("Connection", connection);
  response.setBody(readFileContent(filename));
  response.send(client->socket);

  request.clear();
  return keepalive;
}

// The client can send a request in several packets. For example,
// the headers in the first packet and the request body in the second packet.
// Therefore, this function can be called multiple times for a single
// request until the entire request has been read.
bool processClient(Client *client, Client *clients[], int numClients)
{
  // read request
  ssize_t r = client->socket->recv(client->inputBuffer, INPUT_BUFFER_SIZE);
  cout << "Received " << r << " bytes" << endl;

  if (r == 0)
    return false; // client disconnected

  client->lastActivity = time(NULL); // update last activity time

  // split request to Request Line, Request Headers and Request Body
  int i = 0;
  // for each character in the buffer
  while (i < r)
  {
    if (client->phase == Phase::BODY) // the third phase of reading request
    {
      if (client->request.body.size() == client->request.contentLength)
      {
        if (!processRequest(client, client->request))
          return false; // need close connection
        continue;
      }
      // append character to request body
      client->request.body.push_back(client->inputBuffer[i]);
      ++i;
    }
    else
    {
      // append character to current line
      client->currentLine += client->inputBuffer[i];
      i += 1;

      // check to see if current line is completed (ends with "\r\n")
      if (client->currentLine.size() > 1 && client->currentLine[client->currentLine.size() - 2] == '\r' && client->currentLine[client->currentLine.size() - 1] == '\n')
      {
        client->currentLine.pop_back(); // remove \n
        client->currentLine.pop_back(); // remove \r

        if (client->phase == Phase::REQUEST) // the first phase of reading request
        {
          if (!client->request.setQuery(client->currentLine) || client->request.method != "GET" || client->request.protocol != "HTTP/1.1")
          {
            Response::send400(client->socket); // malformed request
            return false;
          }
          client->phase = Phase::HEADER;
        }
        else if (client->phase == Phase::HEADER) // the second phase of reading request
        {
          if (client->currentLine.empty()) // empty line after headers
          {
            client->phase = Phase::BODY;
            // try to get content length from the header
            if (client->request.hasHeader("Content-Length"))
            {
              client->request.contentLength = atoi(client->request.headers["Content-Length"].c_str());
            }
            // else content length == 0

            if (client->request.contentLength > INPUT_BUFFER_SIZE)
            {
              // Error: too large request body
              Response::send400(client->socket);
              return false;
            }
          }
          else
          {
            // current line is a header line
            if (!client->request.addHeader(client->currentLine))
            {
              Response::send400(client->socket); // malformed request
              return false;
            }
          }
        }
        client->currentLine = "";
      }
    }
  }

  // check to see if reading the request is complete
  if (client->phase == Phase::BODY && client->request.body.size() == client->request.contentLength)
  {
    return processRequest(client, client->request);
  }

  //cout << "Keep-alive Connection" << endl;

  return true;
}

int main(int argc, char *argv[])
{
  FDSet inputSet(MAX_CLIENTS + 1);
  ServerSocket theServer;
  Client *clients[MAX_CLIENTS];
  int numClients = 0;
  int portNumber = 0;

  if (argc > 1)
  {
    portNumber = atoi(argv[1]);
  }
  if (portNumber == 0)
  {
    portNumber = PORT_NUMBER;
  }

  /* Bind the server listening socket to the given port. */
  theServer.bind(portNumber);
  cout << "Server bound to port " << portNumber << endl;

  /* Add the listening socket. */
  inputSet.addFD(theServer.fd());

  /* The server repeats the same process over and over. */
  while (true)
  {
    /* Wait for input on an input stream. */
    int *activeSet = inputSet.select();
    int i = 0;
    while (activeSet[i] >= 0)
    {
      int fd = activeSet[i];
      if (fd == theServer.fd())
      {
        // accept a connection
        Client *client = new Client(theServer.accept());
        if (addClient(clients, numClients, client))
        {
          inputSet.addFD(client->socket->fd());
        }
      }
      else
      {
        // process input on a client socket.
        Client *client = findClient(fd, clients, numClients);
        // process input
        if (!processClient(client, clients, numClients))
        {
          removeClient(clients, numClients, client, inputSet);
        }
      }
      i++;
    }
    removeTimedOutClients(clients, numClients, inputSet);
  }
}