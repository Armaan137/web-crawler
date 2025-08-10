#ifndef HTTP_HPP
#define HTTP_HPP

#include <string>
#include <curl/curl.h>
#include <memory>

struct HttpResult {
    long status = 0;
    std::string url;
    std::string body;
};

static size_t writeCallback(char* contents, size_t size, size_t nmemb, void* userdata);
bool getHttp(const std::string& url, HttpResult& output, std::string& error);

#endif
