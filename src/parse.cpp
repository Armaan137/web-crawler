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

// This returns the links of the HTML into a vector.
std::vector<std::string> extractLinks(const std::string& html) {
    lxb_status_t status;
    lxb_dom_element_t *element;
    lxb_html_document_t *document;
    lxb_dom_collection_t *collection;

    document = lxb_html_document_create();
    if (document == nullptr) {
        std::cerr << "Failed to create HTML Document" << "\n";
    }

    status = lxb_html_document_parse(document, (const lxb_char_t *)html.c_str(), sizeof(html) - 1);
    if (status != LXB_STATUS_OK) {
        std::cerr << "Failed to parse HTML." << "\n";
    }

    lxb_dom_element_t *title_el = lxb_dom_interface_element(lxb_html_document_head_element(document));

    lxb_html_document_destroy(document);

    return {};
}
