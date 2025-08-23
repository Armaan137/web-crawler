#include "http_client.hpp"

#include <string_view>
#include <iostream>

// Write callback to collect the response chunks into a string.
static size_t writeCallback(char* contents, size_t size, size_t nmemb, void* userdata) {
    const size_t totalSize {size * nmemb};
    auto* out {static_cast<HttpResult*>(userdata)};
    out->body.append(contents, totalSize);

    return totalSize;
}

bool isStatusLine(std::string_view line) {
    return line.rfind("HTTP/", 0) == 0;
}

// Write callback to collect the last response header into a vector.
static size_t headerCallback(char* contents, size_t size, size_t nmemb, void* userdata) {
    const size_t totalSize {size * nmemb};
    auto* out {static_cast<HttpResult*>(userdata)};
    std::string line(contents, totalSize);
    if (isStatusLine(line)) out->headers.clear();
    out->headers.emplace_back(std::move(line));

    return totalSize;
}

// Performs an HTTP GET request.
bool getHttp(const std::string& url, HttpResult& output, std::string& error) {
    output.status = 0;
    output.url.clear();
    output.body.clear();
    output.headers.clear();
    error.clear();

    // Ensures that curl_easy_cleanup() runs automatically on scope exit.
    std::unique_ptr<CURL, CurlHandleDeleter> curl(curl_easy_init());

    if (!curl) {
        std::cerr << "Easy initializing failed." << "\n";
        return false;
    }

    char errbuf[CURL_ERROR_SIZE] = {};

    const char* userAgent = "CrawlerWIP (+https://example.local)";

    curl_easy_setopt(curl.get(), CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(curl.get(), CURLOPT_NOSIGNAL, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, userAgent);
    curl_easy_setopt(curl.get(), CURLOPT_HEADERFUNCTION, headerCallback);
    curl_easy_setopt(curl.get(), CURLOPT_HEADERDATA, &output);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &output);
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 20L);        
    curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 10L);

    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 2L);

    CURLcode rc {curl_easy_perform(curl.get())};
    if (rc != CURLE_OK) {
        if (errbuf[0] != '\0') {
            error = std::string(curl_easy_strerror(rc)) + ": " + errbuf;
        } else {
            error = curl_easy_strerror(rc);
        }

        return false;
    }

    long status = 0;
    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &status);

    char* eff = nullptr;
    curl_easy_getinfo(curl.get(), CURLINFO_EFFECTIVE_URL, &eff);

    output.status = status;
    output.url = eff ? std::string(eff) : url;

    return true;
}
