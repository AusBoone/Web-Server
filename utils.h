#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <vector>

std::vector<std::string> splitHeaderLine(const std::string &s);

std::vector<std::string> splitBySpace(const std::string &s);

bool startsWith(const std::string &str, const std::string &prefix);
bool endsWith(const std::string &str, const std::string &suffix);

std::string contentTypeFromFilename(const std::string &filename);

std::vector<char> readFileContent(const std::string &fname);

bool isFileExist(const std::string &fname);

std::string makeFilename(const std::string &uri);

std::string formatDate(time_t t);
time_t parseDate(const std::string &date);

time_t getLastModified(const std::string &filename);

#endif
