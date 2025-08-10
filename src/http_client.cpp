#include "http.hpp"

// Write callback to collect the respone into a string.
size_t writeCallback(char* contents, size_t size, size_t nmemb, void* userdata) {
    
    size_t totalSize = size * nmemb;
    std::string* response = static_cast<std::string*>(userdata);
    respone->append(static_cast<char*>(contents), totalSize)

    return totalSize;
}
