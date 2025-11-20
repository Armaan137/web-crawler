#include "csv_writer.hpp"

#include <iostream>
#include <sstream>

CsvWriter::CsvWriter(const std::string& filename)
    : m_filename(filename) {
    m_file.open(filename, std::ios::out | std::ios::trunc);
    if (!m_file.is_open()) {
        std::cerr << "Error: could not open CSV file for writing: " << filename << "\n";
    }
}

CsvWriter::~CsvWriter() {
    if (m_file.is_open()) {
        m_file.close();
    }
}

bool CsvWriter::writeHeader() {
    if (!m_file.is_open()) {
        return false;
    }
    
    m_file << "URL,Title,Status Code,Link Count,Error\n";
    return m_file.good();
}

std::string CsvWriter::escapeCsvField(const std::string& field) {
    // Check if field needs escaping (contains comma, quote, or newline)
    bool needsEscaping = field.find(',') != std::string::npos ||
                         field.find('"') != std::string::npos ||
                         field.find('\n') != std::string::npos ||
                         field.find('\r') != std::string::npos;
    
    if (!needsEscaping) {
        return field;
    }
    
    // Escape by wrapping in quotes and doubling any quotes
    std::string escaped = "\"";
    for (char c : field) {
        if (c == '"') {
            escaped += "\"\"";
        } else {
            escaped += c;
        }
    }
    escaped += "\"";
    
    return escaped;
}

bool CsvWriter::writeResult(const CrawlResult& result) {
    if (!m_file.is_open()) {
        return false;
    }
    
    m_file << escapeCsvField(result.url) << ","
           << escapeCsvField(result.title) << ","
           << result.status << ","
           << result.linkCount << ","
           << escapeCsvField(result.error) << "\n";
    
    return m_file.good();
}

bool CsvWriter::flush() {
    if (!m_file.is_open()) {
        return false;
    }
    m_file.flush();
    return m_file.good();
}

