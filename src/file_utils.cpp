#include "file_utils.hpp"

// Saves results to a file in the root of the project directory.
bool saveToFile(const HttpResult& output) {
    const char* path = "../page.html";
    std::ofstream outputFile(path, std::ios::binary);

    if (!outputFile.is_open()) {
        std::cerr << "Error: could not open file to write to." << "\n";
        return false;
    }

    outputFile << output.body;

    if (outputFile.bad()) {
        std::cerr << "Error: failed to write to a file." << "\n";
        return false;
    }

    std::cout << "Saved to file." << "\n";

    outputFile.close();

    return true;
}
