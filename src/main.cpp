#include "main.hpp"
#include "http.hpp"

int main() {
    HttpResult result;
    std::string error;

    std::string url = "https://example.com";

    if (getHttp(url, result, error)) {
        std::cout << "Status: " << result.status << "\n";
        std::cout << "Final URL: " << result.effective_url << "\n";
        std::cout << "Body (first 500 chars):\n";
        std::cout << result.body.substr(0, 500) << "\n";
    } else {
        std::cerr << "Request failed: " << error << "\n";
    }

    return 0;
}
