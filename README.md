# C++ Multithreaded Web Crawler

A high-performance, multithreaded web crawler written in modern C++ (C++20).  
It efficiently crawls websites, extracts links, collects metadata, and saves results to CSV files. Perfect for learning systems programming, networking, concurrent programming, and search engine design.

---

## Features

- **Multithreaded Crawling**: Uses a thread pool (default: 4 threads) for concurrent page fetching
- **Frontier Queue Management**: Maintains a queue of URLs to crawl with referrer tracking
- **Visited URL Tracking**: Prevents revisiting pages using thread-safe URL deduplication
- **Link Extraction**: Parses HTML using Lexbor to extract all `<a href="">` links
- **URL Resolution**: Automatically resolves relative URLs to absolute URLs
- **Same-Domain Crawling**: Configurable to crawl only within the starting domain
- **CSV Output**: Saves crawl results to timestamped CSV files with proper escaping
- **Configurable Limits**: Set maximum number of pages to crawl
- **Robust Error Handling**: Handles network errors, timeouts, and malformed HTML gracefully

---

## Prerequisites

- C++20 or later (`g++` or `clang++`)
- [CMake](https://cmake.org/) (version 3.28 or later)
- [libcurl](https://curl.se/libcurl/) (for HTTP requests)
- [Lexbor](https://github.com/lexbor/lexbor) (for HTML parsing)
- Threading support (pthreads)

---

## Installation

### 1. Install System Dependencies

**On Ubuntu/Debian:**
```bash
sudo apt install g++ cmake libcurl4-openssl-dev
```

### 2. Build Lexbor

```bash
git clone https://github.com/lexbor/lexbor.git
cd lexbor
cmake .
make
sudo make install
```

### 3. Build the Crawler

```bash
git clone https://github.com/Armaan137/web-crawler.git
cd web-crawler
mkdir build && cd build
cmake ..
cmake --build .
```

---

## Usage

### Basic Usage

Crawl a website with default settings (100 pages max, 4 threads):

```bash
./build/crawler https://example.com
```

### Advanced Usage

Specify the maximum number of pages to crawl:

```bash
./build/crawler https://example.com 50
```

### Output

The crawler generates a CSV file with a timestamped filename:
- Format: `crawl_results_YYYYMMDD_HHMMSS.csv`
- Columns: `URL, Title, Status Code, Link Count, Error`

Example output:
```
Starting multithreaded web crawler...
Start URL: https://example.com
Max pages: 100
Threads: 4

Crawling completed!
Total pages crawled: 42
Results saved to: crawl_results_20240115_143022.csv
```

---

## How It Works

1. **Initialization**: The crawler starts with a seed URL and initializes a thread pool
2. **Frontier Queue**: URLs to crawl are added to a thread-safe frontier queue
3. **Worker Threads**: Multiple threads concurrently fetch pages from the queue
4. **Link Extraction**: Each page is parsed to extract links using Lexbor HTML parser
5. **URL Processing**: Links are resolved (relative → absolute), normalized, and validated
6. **Deduplication**: Visited URLs are tracked to prevent revisiting pages
7. **Domain Filtering**: Only URLs from the same domain are added to the frontier
8. **Result Collection**: All crawled page data is collected and saved to CSV
9. **Termination**: Crawling stops when max pages is reached or frontier is empty

---

## Architecture

### Key Components

- **`WebCrawler`**: Main crawler class managing threads and frontier queue
- **`CsvWriter`**: Handles CSV file writing with proper field escaping
- **`extractLinks()`**: Parses HTML and extracts all anchor tag links
- **`extractTitle()`**: Extracts page title from HTML
- **`resolveUrl()`**: Resolves relative URLs to absolute URLs
- **`normalizeUrl()`**: Normalizes URLs (removes fragments, trailing slashes)

### Thread Safety

- Mutex-protected frontier queue and visited URL set
- Atomic counters for page count and active workers
- Condition variables for efficient thread synchronization
- Lock-free operations where possible for better performance

---

## CSV Output Format

The CSV file contains the following columns:

| Column | Description |
|--------|-------------|
| URL | The crawled page URL |
| Title | Page title extracted from `<title>` tag |
| Status Code | HTTP response status code (200, 404, etc.) |
| Link Count | Number of links found on the page |
| Error | Error message if the page failed to load |

---

## Configuration

You can modify the crawler behavior by editing `src/main.cpp`:

- **Thread Count**: Change the `numThreads` variable (default: 4)
- **Domain Filtering**: Modify `shouldCrawl()` in `src/crawler.cpp` to allow external links
- **Timeout Settings**: Adjust timeouts in `src/http_client.cpp`

---

## Limitations

- Currently crawls only same-domain links (can be modified)
- No robots.txt parsing (function exists but not integrated)
- No rate limiting (be respectful when crawling)
- No cookie/session management
- No JavaScript execution (static HTML only)
