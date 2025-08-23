#ifndef HTTP_HPP
#define HTTP_HPP

#include <string>
#include <curl/curl.h>
#include <memory>
#include <vector>

struct HttpResult {
    long status = 0;
    std::string url;
    std::string body;
    std::vector<std::string> headers;
};

// RAII deleters.
struct CurlHandleDeleter {
    void operator()(CURL* curl) const {
        if (curl) curl_easy_cleanup(curl);
    }
};

struct SlistDeleter {
    void operator()(curl_slist* s) const {
        if (s) curl_slist_free_all(s);
    }
};

bool getHttp(const std::string& url, HttpResult& output, std::string& error);

#endif
