#include "main.hpp"
#include "http_client.hpp"
#include "file_utils.hpp"
#include "parse.hpp"

#include <string>
#include <curl/curl.h>

// Checks if the URL is valid.
static bool isValidUrl (const std::string& url) {
    CURLU* handle {curl_url()};
    CURLUcode rc {curl_url_set(handle, CURLUPART_URL, url.c_str(), 0)};
    curl_url_cleanup(handle);
    return rc == CURLUE_OK;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Must pass in one URL.\n";
        return 1;
    }

    std::string url {argv[1]};

    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        std::cerr << "Global initializing failed." << "\n";
        return 1;
    }

    if (!isValidUrl(url)) {
        std::cerr << "URL is invalid.\n";
        return 1;
    }

    HttpResult result;
    HttpResult robots;
    std::string error;
    std::vector<std::string> parsedURLS;

    if (getHttp(url, result, error)) {
        std::vector headers = result.headers;
        for (const auto& element : headers) {
            std::cout << element;
        }
        std::cout << "Status: " << result.status << "\n";
        std::cout << "Final URL: " << result.url << "\n";
        // if (!saveToFile(result)) return 1;
        std::string title = extractTitle(result.body);
        parsedURLS = extractLinks(result.body);
        // std::cout << "Extracted title: " << title << "\n";
        std::string body = result.body;
        // std::cout << "Body:" << body << "\n"; 

        for (auto& url : parsedURLS) {
            std::cout << "Parsed Url: " << url << "\n";
        }
        
        /*
        if (getRobots(url, robots, error)) {   
            std::cout << "Robots.txt: " << robots.body;
        }
        */
    } else {
        std::cerr << "Request failed: " << error << "\n";
        curl_global_cleanup();
        return 1;
    }

    curl_global_cleanup();

    return 0;
}
