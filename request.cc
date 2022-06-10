#include "request.h"
#include "utils.h"

#include <string>
#include <map>
#include <vector>
using namespace std;

void Request::clear()
{
    query = "";
    headers.clear();
    body.clear();
    contentLength = 0;
}

bool Request::setQuery(const std::string &query)
{
    this->query = query;
    vector<string> v = splitBySpace(query);
    if (v.size() != 3)
    {
        return false;
    }
    method = v[0];
    uri = v[1];
    protocol = v[2];
    if (endsWith(uri, "/"))
    {
        uri.append("index.html");
    }
    return true;
}

bool Request::addHeader(const std::string &header)
{
    vector<string> kv = splitHeaderLine(header);
    if (kv.size() == 2)
    {
        headers.insert(make_pair(kv[0], kv[1]));
        return true;
    }
    return false;
}

bool Request::hasHeader(const std::string &header)
{
    return headers.count(header) > 0;
}
