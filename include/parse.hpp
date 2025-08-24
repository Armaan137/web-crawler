#ifndef PARSE_H
#define PARSE_H

#include "http_client.hpp"

#include <string>
#include <vector>

std::string extractTitle(const std::string& html);
std::vector<std::string> extractLinks(const std::string& html);

#endif
