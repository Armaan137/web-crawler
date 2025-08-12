#ifndef PARSE_H
#define PARSE_H

#include <string>
#include <regex>
#include <vector>
#include "http_client.hpp"
#include <lexbor/dom/dom.h>
#include <lexbor/html/html.h>

std::string extractTitle(const std::string& html);
std::vector<std::string> extractLinks(const std::string& html);

#endif
