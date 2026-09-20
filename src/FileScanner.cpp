#include "FileScanner.h"
#include "MD5.h"
#include "ThreadPool.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <ctime>
#include <atomic>

namespace fs = std::filesystem;

std::string FileScanner::calculate_md5(const fs::path& file_path) {
    std::ifstream file(file_path, std::ios::binary);
    if (!file) {
        return "";
    }

    MD5 md5;
    std::vector<char> buffer(65536);
    while (file.read(buffer.data(), buffer.size())) {
        md5.update(reinterpret_cast<const unsigned char*>(buffer.data()), file.gcount());
    }
    if (file.gcount() > 0) {
        md5.update(reinterpret_cast<const unsigned char*>(buffer.data()), file.gcount());
    }

    md5.finalize();
    return md5.hexdigest();
}

std::string FileScanner::format_timestamp(const fs::file_time_type& ftime) {
    const auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
        ftime - fs::file_time_type::clock::now() + std::chrono::system_clock::now()
    );
    const std::time_t cftime = std::chrono::system_clock::to_time_t(sctp);
    char buffer[80];
    std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", std::localtime(&cftime));
    return std::string(buffer);
}

std::string FileScanner::generate_timestamped_filename(const std::string& prefix, const std::string& ext) {
    const auto now = std::chrono::system_clock::now();
    const std::time_t now_c = std::chrono::system_clock::to_time_t(now);
    const std::tm* now_tm = std::localtime(&now_c);
    char time_str[32];
    std::strftime(time_str, sizeof(time_str), "%y%m%d_%H%M%S", now_tm);
    return prefix + "_" + std::string(time_str) + ext;
}

std::vector<FileRecord> FileScanner::scan_directory_multithreaded(const fs::path& root_path) {
    std::cout << "[1/2] Scanning file system metadata...\n";
    
    struct PendingItem {
        fs::path path;
        std::string name;
        std::uintmax_t size{0};
        std::string mod_date;
    };

    std::vector<PendingItem> pending_items;

    for (const auto& entry : fs::recursive_directory_iterator(root_path, fs::directory_options::skip_permission_denied)) {
        if (!entry.is_regular_file()) continue;

        try {
            PendingItem item;
            item.path = entry.path();
            item.name = entry.path().filename().string();
            item.size = entry.file_size();
            item.mod_date = format_timestamp(entry.last_write_time());
            pending_items.push_back(std::move(item));
        } catch (const fs::filesystem_error& e) {
            std::cerr << "\n[Warning] Metadata read error: " << e.what() << "\n";
        }
    }

    const size_t total_files = pending_items.size();
    const size_t num_threads = std::max(1u, std::thread::hardware_concurrency());
    
    std::cout << "[2/2] Parallel MD5 calculation on " << num_threads << " threads (" 
              << total_files << " files)...\n";

    ThreadPool pool(num_threads);
    std::vector<std::future<FileRecord>> futures;
    futures.reserve(total_files);

    std::atomic<size_t> processed_count{0};

    for (auto&& item : pending_items) {
        futures.push_back(pool.enqueue([item = std::move(item), &processed_count, total_files]() -> FileRecord {
            FileRecord rec;
            rec.path = item.path.lexically_normal().string();
            rec.name = std::move(item.name);
            rec.size = item.size;
            rec.mod_date = std::move(item.mod_date);
            rec.md5_checksum = FileScanner::calculate_md5(item.path);

            const size_t current = ++processed_count;
            if (current % 1000 == 0 || current == total_files) {
                std::cout << "\rProcessed: " << current << " / " << total_files 
                          << " (" << (current * 100 / total_files) << "%)" << std::flush;
            }
            return rec;
        }));
    }

    std::vector<FileRecord> records;
    records.reserve(total_files);
    for (auto& fut : futures) {
        records.push_back(fut.get());
    }
    std::cout << "\n";

    return records;
}
