#include "main.hpp"
#include "http_client.hpp"
#include "file_utils.hpp"
#include "parse.hpp"

int main() {
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        std::cerr << "Global initializing failed." << "\n";
        return 1;
    }

    HttpResult result;
    std::string error;
    std::string url = "https://example.com";

    if (getHttp(url, result, error)) {
        std::cout << "Status: " << result.status << "\n";
        std::cout << "Final URL: " << result.url << "\n";
        if (!saveToFile(result)) return 1;
        std::string title = extractTitle(result.body);
        std::cout << "Extracted title: " << title << "\n";
    } else {
        std::cerr << "Request failed: " << error << "\n";
        curl_global_cleanup();
        return 1;
    }

    curl_global_cleanup();

    return 0;
}
