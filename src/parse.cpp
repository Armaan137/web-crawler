#include "parse.hpp"

// This will eventually parse the HTML fully.
std::string extractTitle(const std::string& html) {

    std::string openingTag = "<title>";
    std::string closingTag = "</title>";
    
    size_t start = html.find(openingTag);
    size_t end = html.find(closingTag);

    if (start == std::string::npos || end == std::string::npos) {
        std::cerr << "Could not find a title." << "\n";
        return "";
    }

    start += openingTag.size();

    return html.substr(start, end - start);
}
