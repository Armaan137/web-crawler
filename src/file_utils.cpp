#include "file_utils.hpp"

// Saves results to a file in the root of the project directory.
bool saveToFile(const HttpResult& output) {
    const char* path = "../result.txt";
    std::ofstream outputFile(path);

    if (!outputFile.is_open()) {
        std::cerr << "Error: could not open a results.txt file to write to." << "\n";
        return false;
    }

    outputFile << output.body;

    if (outputFile.bad()) {
        std::cerr << "Error: failed to write to a results.txt file." << "\n";
        return false;
    }

    outputFile.close();

    return true;
}
