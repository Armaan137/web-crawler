#include "main.hpp"
#include "crawler.hpp"
#include "csv_writer.hpp"

#include <string>
#include <curl/curl.h>
#include <iostream>
#include <iomanip>
#include <ctime>
#include <sstream>

// Checks if the URL is valid.
static bool isValidUrl(const std::string& url) {
    CURLU* handle = curl_url();
    if (!handle) return false;
    CURLUcode rc = curl_url_set(handle, CURLUPART_URL, url.c_str(), 0);
    curl_url_cleanup(handle);
    return rc == CURLUE_OK;
}

// Generate CSV filename with timestamp
static std::string generateCsvFilename() {
    auto now = std::time(nullptr);
    auto* localTime = std::localtime(&now);
    
    std::ostringstream oss;
    oss << "crawl_results_"
        << std::put_time(localTime, "%Y%m%d_%H%M%S")
        << ".csv";
    return oss.str();
}

int main(int argc, char* argv[]) {
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << argv[0] << " <start_url> [max_pages]\n";
        std::cerr << "  start_url: The starting URL to crawl\n";
        std::cerr << "  max_pages: Maximum number of pages to crawl (default: 100)\n";
        return 1;
    }

    std::string startUrl = argv[1];
    size_t maxPages = 100;
    
    if (argc == 3) {
        try {
            maxPages = std::stoul(argv[2]);
        } catch (const std::exception& e) {
            std::cerr << "Invalid max_pages value: " << argv[2] << "\n";
            return 1;
        }
    }

    if (!isValidUrl(startUrl)) {
        std::cerr << "URL is invalid: " << startUrl << "\n";
        return 1;
    }

    const size_t numThreads = 4;
    
    std::cout << "Starting multithreaded web crawler...\n";
    std::cout << "Start URL: " << startUrl << "\n";
    std::cout << "Max pages: " << maxPages << "\n";
    std::cout << "Threads: " << numThreads << "\n\n";

    // Create crawler with specified number of threads
    WebCrawler crawler(numThreads, maxPages);
    
    // Start crawling
    crawler.start(startUrl);
    
    // Get results
    auto results = crawler.getResults();
    
    // Generate CSV filename
    std::string csvFilename = generateCsvFilename();
    
    // Write results to CSV
    CsvWriter csvWriter(csvFilename);
    if (!csvWriter.writeHeader()) {
        std::cerr << "Failed to write CSV header\n";
        return 1;
    }
    
    for (const auto& result : results) {
        if (!csvWriter.writeResult(result)) {
            std::cerr << "Failed to write result to CSV\n";
            return 1;
        }
    }
    
    csvWriter.flush();
    
    std::cout << "\nCrawling completed!\n";
    std::cout << "Total pages crawled: " << results.size() << "\n";
    std::cout << "Results saved to: " << csvFilename << "\n";

    return 0;
}
