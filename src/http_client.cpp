#include "http.hpp"

// Write callback to collect the response chunks into a string.
size_t writeCallback(char* contents, size_t size, size_t nmemb, void* userdata) {
    
    const size_t totalSize = size * nmemb;
    auto* response = static_cast<std::string*>(userdata);
    response->append(contents, totalSize);

    return totalSize;
}

// Performs an HTTP GET request.
bool getHttp(const std::string& url, HttpResult& output, std::string& error) {
    static bool initialized = (curl_global_init(CURL_GLOBAL_DEFAULT) == 0);

    if (!initialized) {
        std::cout << "Global initializing failed.";
        return false;
    }

    // Ensures that curl_easy_cleanup() runs automatically on scope exit.
    std::unique_ptr<CURL, CurlHandleDeleter> curl(curl_easy_init());

    if (!curl) {
        std::cout << "Easy initializing failed.";
        return false;
    }

    std::string body;
    char errbuf[CURL_ERROR_SIZE] = {};

    const char* userAgent = "CrawlerWIP (+https://example.local)";

    curl_easy_setopt(curl.get(), CURLOPT_ERRORBUFFER, errbuf);
    curl_easy_setopt(curl.get(), CURLOPT_ACCEPT_ENCODING, "");
    curl_easy_setopt(curl.get(), CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl.get(), CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_MAXREDIRS, 5L);
    curl_easy_setopt(curl.get(), CURLOPT_USERAGENT, userAgent);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl.get(), CURLOPT_WRITEDATA, &body);
    curl_easy_setopt(curl.get(), CURLOPT_TIMEOUT, 20L);        
    curl_easy_setopt(curl.get(), CURLOPT_CONNECTTIMEOUT, 10L);

    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYPEER, 1L);
    curl_easy_setopt(curl.get(), CURLOPT_SSL_VERIFYHOST, 2L);

    CURLcode rc = curl_easy_perform(curl.get());
    if (rc != CURLE_OK) {
        error = curl_easy_strerror(rc);
        return false;
    }

    long status = 0;
    curl_easy_getinfo(curl.get(), CURLINFO_RESPONSE_CODE, &status);

    char* eff = nullptr;
    curl_easy_getinfo(curl.get(), CURLINFO_EFFECTIVE_URL, &eff);

    output.status = status;
    output.url = eff ? std::string(eff) : url;
    output.body = std::move(body);
    return true;
}
