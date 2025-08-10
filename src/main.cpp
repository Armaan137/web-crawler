#include "main.hpp"
#include "http.hpp"

int main() {
    std::cout << "Crawler Test." << '\n';
    fetch_url("test.com");

    return 0;
}
