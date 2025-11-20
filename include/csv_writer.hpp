#ifndef CSV_WRITER_HPP
#define CSV_WRITER_HPP

#include "crawler.hpp"

#include <string>
#include <vector>
#include <fstream>

class CsvWriter {
public:
    CsvWriter(const std::string& filename);
    ~CsvWriter();
    
    bool writeHeader();
    bool writeResult(const CrawlResult& result);
    bool flush();
    
private:
    std::string escapeCsvField(const std::string& field);
    
    std::ofstream m_file;
    std::string m_filename;
};

#endif

