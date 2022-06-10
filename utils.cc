#include "utils.h"

#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <ctime>
using namespace std;

static const int FILE_BUFFER_SIZE = 4096;

std::vector<std::string> splitHeaderLine(const std::string &s)
{
  std::vector<std::string> kv;
  size_t colon = s.find(':');
  if (colon != s.npos)
  {
    kv.push_back(s.substr(0, colon));
    kv.push_back(s.substr(colon + 2));
  }
  return kv;
}

std::vector<std::string> splitBySpace(const std::string &s)
{
  std::vector<std::string> v;
  std::stringstream ss(s);
  std::string line;
  while (ss >> line)
  {
    v.push_back(line);
  }
  return v;
}

bool endsWith(const std::string &str, const std::string &suffix)
{
  return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool startsWith(const std::string &str, const std::string &prefix)
{
  return str.size() >= prefix.size() && str.compare(0, prefix.size(), prefix) == 0;
}

std::string contentTypeFromFilename(const std::string &filename)
{
  if (endsWith(filename, ".html"))
    return "text/html";
  if (endsWith(filename, ".gif"))
    return "image/gif";
  if (endsWith(filename, ".jpeg"))
    return "image/jpeg";
  if (endsWith(filename, ".jpg"))
    return "image/jpeg";
  return "text/plain";
}

std::vector<char> readFileContent(const std::string &fname)
{
  std::vector<char> content;
  std::ifstream in(fname, std::ios::binary);
  if (in)
  {
    char buffer[FILE_BUFFER_SIZE];
    while (in)
    {
      in.read(buffer, FILE_BUFFER_SIZE);
      if (in.gcount() > 0)
      {
        content.insert(content.end(), buffer, buffer + in.gcount());
      }
    }
  }

  return content;
}

bool isFileExist(const std::string &fname)
{
  return access(fname.c_str(), R_OK) == 0;
}

std::string makeFilename(const std::string &uri)
{
  char cwd[FILENAME_MAX];
  getcwd(cwd, FILENAME_MAX);
  string fname = cwd;  
  if (!startsWith(uri, "/"))
  {
    fname += "/";
  }
  fname += uri;
  return fname;
}

time_t getLastModified(const std::string &filename)
{  
  struct stat statbuf;
  stat(filename.c_str(), &statbuf);
  time_t t = statbuf.st_mtim.tv_sec;  
  return t;
}

std::string formatDate(time_t t)
{
  char outstr[200];  
  struct tm *tmp;  
  tmp = localtime(&t);  
  strftime(outstr, sizeof(outstr), "%a, %d %b %Y %H:%M:%S %z", tmp);
  return outstr;
}

time_t parseDate(const std::string &date)
{
  struct tm tmp;
  strptime(date.c_str(), "%a, %d %b %Y %H:%M:%S %z", &tmp);
  time_t t =  mktime(&tmp);  
  return t;
}
