#ifndef FILE_SCANNER_H
#define FILE_SCANNER_H

#include "Models.h"
#include <filesystem>
#include <vector>
#include <string>

/**
 * @brief Fajlrendszer muveletert es MD5 hashelesert felelos osztaly.
 */
class FileScanner {
public:
    static std::vector<FileRecord> scan_directory_multithreaded(const std::filesystem::path& root_path);
    static std::string calculate_md5(const std::filesystem::path& file_path);
    static std::string generate_timestamped_filename(const std::string& prefix, const std::string& ext);

private:
    static std::string format_timestamp(const std::filesystem::file_time_type& ftime);
};

#endif // FILE_SCANNER_H
