#ifndef DATABASE_H
#define DATABASE_H

#include "Models.h"
#include <string>
#include <vector>
#include <sqlite3.h>

/**
 * @brief SQLite adatbazis muveleteket tokoz RAII osztaly.
 */
class Database {
public:
    explicit Database(std::string path);
    ~Database();

    Database(const Database&) = delete;
    Database& operator=(const Database&) = delete;
    Database(Database&&) noexcept;
    Database& operator=(Database&&) noexcept;

    bool open();
    void close() noexcept;
    bool initialize_schema();
    bool save_records(const std::vector<FileRecord>& records);

    [[nodiscard]] const std::string& last_error() const noexcept { return last_error_; }

private:
    std::string db_path_;
    sqlite3* db_handle_{nullptr};
    std::string last_error_;
};

#endif // DATABASE_H
