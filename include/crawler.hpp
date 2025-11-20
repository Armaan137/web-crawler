#ifndef CRAWLER_HPP
#define CRAWLER_HPP

#include "http_client.hpp"
#include "parse.hpp"

#include <string>
#include <vector>
#include <queue>
#include <unordered_set>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <memory>

struct CrawlResult {
    std::string url;
    std::string title;
    long status;
    size_t linkCount;
    std::string error;
};

// Frontier entry: stores URL and metadata about the page that linked to it
struct FrontierEntry {
    std::string url;
    std::string referrerUrl;  // URL of the page that contained this link
    std::string referrerTitle; // Title of the referring page
};

class WebCrawler {
public:
    WebCrawler(size_t numThreads = 4, size_t maxPages = 100);
    ~WebCrawler();
    
    void start(const std::string& startUrl);
    void stop();
    std::vector<CrawlResult> getResults() const;
    
private:
    void workerThread();
    bool shouldCrawl(const std::string& url);
    std::string resolveUrl(const std::string& baseUrl, const std::string& relativeUrl);
    std::string normalizeUrl(const std::string& url);
    void processUrl(const std::string& url);
    bool isFrontierEmpty() const;
    void markWorkerActive();
    void markWorkerIdle();
    
    size_t m_numThreads;
    size_t m_maxPages;
    std::atomic<size_t> m_pagesCrawled{0};
    std::atomic<size_t> m_activeWorkers{0};
    std::atomic<bool> m_shouldStop{false};
    
    // Frontier queue: URLs waiting to be crawled
    std::queue<FrontierEntry> m_frontier;
    // Visited URLs: tracks all URLs we've already crawled or added to frontier
    std::unordered_set<std::string> m_visitedUrls;
    // Results: stores all crawled page information
    std::vector<CrawlResult> m_results;
    
    mutable std::mutex m_frontierMutex;
    mutable std::mutex m_resultsMutex;
    std::condition_variable m_frontierCondition;
    
    std::vector<std::thread> m_threads;
    std::string m_baseDomain;
};

#endif

