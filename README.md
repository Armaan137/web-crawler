# C++ Web Crawler

A lightweight, extensible web crawler written in modern C++ (C++20).  
It fetches web pages, extracts links, and collects metadata — useful for learning systems programming, networking, and basic search engine design.

---

## Features
- Fetches web pages using **libcurl**
- Can fetch and save HTML
- Parses HTML with **Lexbor** (fast & lightweight)
- Extracts links (`<a href="">`)
- Collects response headers and status codes

---

### Prerequisites
- C++20 or later (`g++` or `clang++`)
- [CMake](https://cmake.org/)
- [libcurl](https://curl.se/libcurl/)
- [Lexbor](https://github.com/lexbor/lexbor) (for HTML parsing)

### Build
```
git clone https://github.com/Armaan137/web-crawler.git
cd web-crawler
mkdir build && cmake -S . -B build/
cmake --build build/
```

## Installation and Usage
1. Install/Build Dependencies
```
sudo apt install g++ cmake libcurl4-openssl-dev
git clone https://github.com/lexbor/lexbor.git
cd lexbor
cmake .
make
sudo make install
```
2. Clone the Repository
```
git clone https://github.com/Armaan137/web-crawler.git
cd web-crawler
```
3. Build the Project
```
mkdir build
cmake -S . -B build/
cmake --build build/
```
4. Run the Crawler
```
./build/crawler https://example.com
```
