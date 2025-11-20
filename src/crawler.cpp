#include "crawler.hpp"

#include <iostream>
#include <algorithm>
#include <sstream>
#include <curl/curl.h>

WebCrawler::WebCrawler(size_t numThreads, size_t maxPages)
    : m_numThreads(numThreads), m_maxPages(maxPages) {
}

WebCrawler::~WebCrawler() {
    stop();
}

void WebCrawler::start(const std::string& startUrl) {
    // Initialize curl for the main thread
    if (curl_global_init(CURL_GLOBAL_DEFAULT) != CURLE_OK) {
        std::cerr << "Global initializing failed." << "\n";
        return;
    }
    
    // Extract base domain
    CURLU* handle = curl_url();
    if (handle) {
        if (curl_url_set(handle, CURLUPART_URL, startUrl.c_str(), 0) == CURLUE_OK) {
            char* host = nullptr;
            if (curl_url_get(handle, CURLUPART_HOST, &host, 0) == CURLUE_OK) {
                m_baseDomain = host ? std::string(host) : "";
                curl_free(host);
            }
        }
        curl_url_cleanup(handle);
    }
    
    // Normalize and add starting URL to frontier
    std::string normalized = normalizeUrl(startUrl);
    {
        std::lock_guard<std::mutex> lock(m_frontierMutex);
        FrontierEntry entry;
        entry.url = normalized;
        entry.referrerUrl = "";  // Starting URL has no referrer
        entry.referrerTitle = "";
        m_frontier.push(entry);
        m_visitedUrls.insert(normalized);
    }
    
    // Start worker threads
    m_shouldStop = false;
    for (size_t i = 0; i < m_numThreads; ++i) {
        m_threads.emplace_back(&WebCrawler::workerThread, this);
    }
    
    // Wait for all threads to finish
    for (auto& thread : m_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    
    curl_global_cleanup();
}

void WebCrawler::stop() {
    m_shouldStop = true;
    m_frontierCondition.notify_all();
    
    for (auto& thread : m_threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    m_threads.clear();
}

std::vector<CrawlResult> WebCrawler::getResults() const {
    std::lock_guard<std::mutex> lock(m_resultsMutex);
    return m_results;
}

bool WebCrawler::isFrontierEmpty() const {
    std::lock_guard<std::mutex> lock(m_frontierMutex);
    return m_frontier.empty() && m_activeWorkers == 0;
}

void WebCrawler::markWorkerActive() {
    // Called while holding m_frontierMutex
    m_activeWorkers++;
}

void WebCrawler::markWorkerIdle() {
    // Called while holding m_frontierMutex (or can be called without lock since it's atomic)
    m_activeWorkers--;
}

void WebCrawler::workerThread() {
    // curl_global_init is already called in start(), and it's thread-safe
    // No need to call it again here
    
    while (!m_shouldStop) {
        FrontierEntry entry;
        bool hasWork = false;
        
        // Get URL from frontier queue
        {
            std::unique_lock<std::mutex> lock(m_frontierMutex);
            // Wait until frontier has work, we should stop, or max pages reached
            m_frontierCondition.wait(lock, [this] {
                return !m_frontier.empty() || m_shouldStop || 
                       (m_pagesCrawled >= m_maxPages && m_activeWorkers == 0);
            });
            
            // Check stop conditions: explicit stop, max pages reached, or frontier empty
            if (m_shouldStop) {
                break;
            }
            
            // If we've reached max pages and no active workers, stop
            if (m_pagesCrawled >= m_maxPages && m_activeWorkers == 0) {
                break;
            }
            
            // If frontier is empty, continue waiting (or exit if all done)
            if (m_frontier.empty()) {
                // If no active workers and frontier is empty, we're done
                if (m_activeWorkers == 0) {
                    break;
                }
                continue;
            }
            
            // Get entry from frontier
            entry = m_frontier.front();
            m_frontier.pop();
            hasWork = true;
            markWorkerActive();  // Mark active while holding lock
        }
        
        if (hasWork) {
            // Process the URL (outside the lock for better concurrency)
            processUrl(entry.url);
            
            // Mark idle after processing
            {
                std::lock_guard<std::mutex> lock(m_frontierMutex);
                markWorkerIdle();
                // Notify other threads that a worker is now idle
                m_frontierCondition.notify_all();
            }
        }
        
        // Check if we should stop after processing
        if (m_pagesCrawled >= m_maxPages) {
            // Notify other threads that we're done
            m_frontierCondition.notify_all();
            break;
        }
    }
}

bool WebCrawler::shouldCrawl(const std::string& url) {
    // Check if URL is valid
    CURLU* handle = curl_url();
    if (!handle) return false;
    
    bool isValid = curl_url_set(handle, CURLUPART_URL, url.c_str(), 0) == CURLUE_OK;
    if (!isValid) {
        curl_url_cleanup(handle);
        return false;
    }
    
    // Extract host from URL to compare with base domain
    std::string urlHost;
    char* host = nullptr;
    if (curl_url_get(handle, CURLUPART_HOST, &host, 0) == CURLUE_OK) {
        if (host) {
            urlHost = host;
            curl_free(host);
        }
    }
    curl_url_cleanup(handle);
    
    // Check if URL is from the same domain (optional: can be modified to crawl external links)
    // For now, we'll only crawl same-domain links
    if (!m_baseDomain.empty() && urlHost != m_baseDomain) {
        return false;
    }
    
    // Note: We don't check visitedUrls here to avoid deadlock
    // The caller will do a final check while holding the lock
    
    return true;
}

std::string WebCrawler::normalizeUrl(const std::string& url) {
    std::string normalized = url;
    
    // Remove fragment (everything after #)
    size_t pos = normalized.find('#');
    if (pos != std::string::npos) {
        normalized.resize(pos);
    }
    
    // Remove trailing slash for consistency
    // This normalizes "https://example.com/" to "https://example.com"
    // and "https://example.com/page/" to "https://example.com/page"
    if (normalized.length() > 1 && normalized.back() == '/') {
        normalized.pop_back();
    }
    
    return normalized;
}

std::string WebCrawler::resolveUrl(const std::string& baseUrl, const std::string& relativeUrl) {
    // If relativeUrl is already absolute, validate and return it
    if (relativeUrl.find("://") != std::string::npos) {
        CURLU* handle = curl_url();
        if (!handle) return "";
        CURLUcode rc = curl_url_set(handle, CURLUPART_URL, relativeUrl.c_str(), 0);
        if (rc != CURLUE_OK) {
            curl_url_cleanup(handle);
            return "";
        }
        char* resolved = nullptr;
        std::string result;
        if (curl_url_get(handle, CURLUPART_URL, &resolved, 0) == CURLUE_OK) {
            if (resolved) {
                result = resolved;
                curl_free(resolved);
            }
        }
        curl_url_cleanup(handle);
        return result;
    }
    
    // Use curl's URL API to resolve relative URLs
    CURLU* handle = curl_url();
    if (!handle) return "";
    
    // Set base URL first
    if (curl_url_set(handle, CURLUPART_URL, baseUrl.c_str(), 0) != CURLUE_OK) {
        curl_url_cleanup(handle);
        return "";
    }
    
    // For relative URLs, we need to manually resolve the path
    // Get the current path from base URL
    char* basePath = nullptr;
    curl_url_get(handle, CURLUPART_PATH, &basePath, 0);
    
    std::string resolvedPath;
    if (relativeUrl.empty()) {
        resolvedPath = basePath ? basePath : "/";
    } else if (relativeUrl[0] == '/') {
        // Absolute path - use it directly
        resolvedPath = relativeUrl;
    } else {
        // Relative path - resolve against base path
        std::string currentPath = basePath ? basePath : "/";
        // Remove filename from current path if it exists (keep directory)
        size_t lastSlash = currentPath.find_last_of('/');
        if (lastSlash != std::string::npos) {
            currentPath = currentPath.substr(0, lastSlash + 1);
        }
        resolvedPath = currentPath + relativeUrl;
    }
    
    if (basePath) curl_free(basePath);
    
    // Set the resolved path
    if (curl_url_set(handle, CURLUPART_PATH, resolvedPath.c_str(), 0) != CURLUE_OK) {
        curl_url_cleanup(handle);
        return "";
    }
    
    // Get the final resolved URL
    char* resolved = nullptr;
    std::string result;
    if (curl_url_get(handle, CURLUPART_URL, &resolved, 0) == CURLUE_OK) {
        if (resolved) {
            result = resolved;
            curl_free(resolved);
        }
    }
    
    curl_url_cleanup(handle);
    return result;
}

void WebCrawler::processUrl(const std::string& url) {
    CrawlResult result;
    result.url = url;
    
    HttpResult httpResult;
    std::string error;
    
    if (getHttp(url, httpResult, error)) {
        result.status = httpResult.status;
        result.title = extractTitle(httpResult.body);
        
        // Extract links from the crawled web page
        std::vector<std::string> links = extractLinks(httpResult.body);
        result.linkCount = links.size();
        
        // Prepare new frontier entries with web page info
        std::vector<FrontierEntry> entriesToAdd;
        for (const auto& link : links) {
            // Skip bad schemes (javascript:, mailto:, tel:, data:)
            if (link.find("javascript:") == 0 || 
                link.find("mailto:") == 0 || 
                link.find("tel:") == 0 ||
                link.find("data:") == 0) {
                continue;
            }
            
            // Resolve relative URLs to absolute URLs
            std::string resolved = resolveUrl(url, link);
            if (resolved.empty()) continue;
            
            // Normalize the URL (remove fragments, trailing slashes, etc.)
            std::string normalized = normalizeUrl(resolved);
            
            // Check if we should crawl this URL (domain validation, etc.)
            if (shouldCrawl(normalized)) {
                FrontierEntry entry;
                entry.url = normalized;
                entry.referrerUrl = url;  // Record which page linked to this URL
                entry.referrerTitle = result.title;  // Record the title of the referring page
                entriesToAdd.push_back(entry);
            }
        }
        
        // Add new entries to frontier queue (thread-safe)
        {
            std::lock_guard<std::mutex> lock(m_frontierMutex);
            for (const auto& entry : entriesToAdd) {
                // Double-check visited status while holding lock to prevent duplicates
                if (m_visitedUrls.find(entry.url) == m_visitedUrls.end()) {
                    // Mark as visited immediately to prevent other threads from adding it
                    m_visitedUrls.insert(entry.url);
                    
                    // Only add to frontier if we haven't reached max pages
                    if (m_pagesCrawled < m_maxPages) {
                        m_frontier.push(entry);
                        m_frontierCondition.notify_one();
                    }
                }
            }
        }
    } else {
        result.status = 0;
        result.error = error;
        result.linkCount = 0;
    }
    
    // Record the crawled page result
    {
        std::lock_guard<std::mutex> lock(m_resultsMutex);
        m_results.push_back(result);
    }
    
    m_pagesCrawled++;
    
    // Notify waiting threads that new work may be available
    m_frontierCondition.notify_all();
}

