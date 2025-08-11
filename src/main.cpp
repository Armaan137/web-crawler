#include "main.hpp"
#include "http.hpp"
#include "file_utils.hpp"

int main() {
    HttpResult result;
    std::string error;

    std::string url = "https://example.com";

    if (getHttp(url, result, error)) {
        std::cout << "Status: " << result.status << "\n";
        std::cout << "Final URL: " << result.url << "\n";
        if (!saveToFile(result)) return 1;
    } else {
        std::cerr << "Request failed: " << error << "\n";
        return 1;
    }

    return 0;
}
