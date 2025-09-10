#include "parse.hpp"

#include <iostream>

extern "C" {
#include <lexbor/dom/interfaces/element.h>
#include <lexbor/dom/interfaces/attr.h>
#include <lexbor/dom/interfaces/document.h>
#include <lexbor/dom/interfaces/node.h>
#include <lexbor/html/html.h>
#include <lexbor/html/parser.h>
}

// Extracts titles.
std::string extractTitle(const std::string& html) {

    std::string openingTag = "<title>";
    std::string closingTag = "</title>";
    
    size_t start = html.find(openingTag);
    size_t end = html.find(closingTag);

    if (start == std::string::npos || end == std::string::npos) {
        std::cerr << "Could not find a title.\n";
        return "";
    }

    start += openingTag.size();

    return html.substr(start, end - start);
}

// Helper for normalizing URLs.
static bool badScheme(std::string_view url) {
    // checks if the URL starts with a certain prefix.
    auto starts = [&](std::string_view prefix) {
        return url.size() >= prefix.size() && url.substr(0, prefix.size()) == prefix;
    };

    return starts("javascript:") || starts("mailto:") || starts("tel:") || starts("data");
}

// Strips the fragment off a URl if it has one.
static std::string removeFragment(std::string url) {
    std::size_t pos = url.find('#');
    if (pos != std::string::npos) {
        url.resize(pos);
    }

    return url;
}

// Performs a DFS to extract links into a vector.
static void collectLinksDfs(lxb_dom_node_t* node, std::vector<std::string>& links) {
    // Traverse until the current node is null; until there are no more sibling nodes.
    for (lxb_dom_node_t* curr {node}; curr; curr = lxb_dom_node_next(curr)) {
        if (curr->type == LXB_DOM_NODE_TYPE_ELEMENT) {
            auto* element {lxb_dom_interface_element(curr)};
            
            // Check if current element is an <a>.
            if (lxb_dom_element_tag_id(element) == LXB_TAG_A) {
                const lxb_char_t* name {(const lxb_char_t*)"href"};
                lxb_dom_attr_t* attribute {lxb_dom_element_attr_by_name(element, name, 4)};
                // Check if there is an "href" attribute.
                if (attribute) {
                    size_t length {};
                    const lxb_char_t* data {lxb_dom_attr_value(attribute, &length)};
                    if (data) {
                        links.emplace_back(reinterpret_cast<const char*>(data), length);
                    }
                }
            }
        }
        
        // Recursively call DFS for the children of the current node.
        auto* child {lxb_dom_node_first_child(curr)};
        if (child) {
            collectLinksDfs(child, links);
        }
    }
}

// This returns the links of the HTML into a vector.
std::vector<std::string> extractLinks(const std::string& html) {
    std::vector<std::string> links;

    // Lexbor does not rely on a null terminator. It treats HTML as a raw buffer of bytes. 
    lxb_html_document_t* document = lxb_html_document_create();
    size_t htmlLength = html.size();

    if (document == nullptr) {
        std::cerr << "Failed to create HTML Document.\n";
        return links;
    }

    // Parse the html doc.
    lxb_status_t status = lxb_html_document_parse(document, reinterpret_cast<const lxb_char_t*>(html.data()), htmlLength);
    if (status != LXB_STATUS_OK) {
        std::cerr << "Failed to parse HTML.\n";
        lxb_html_document_destroy(document);
        return links;
    }

    // Traverse starting from the body if available. Otherwise, traverse from the docs root.
    lxb_dom_node_t* start = nullptr;
    if (document->body) {
        auto* bodyElement = lxb_dom_interface_element(document->body);
        start = lxb_dom_interface_node(bodyElement);
    } else {
        auto* node = lxb_dom_interface_node(lxb_dom_interface_document(document));
        start = lxb_dom_node_first_child(node);
    }

    // Only do a traversal if we have a place to start from.
    if (start) collectLinksDfs(start, links);

    lxb_html_document_destroy(document);

    return links;
}
