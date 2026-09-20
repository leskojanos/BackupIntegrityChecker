#include "Database.h"

Database::Database(std::string path) : db_path_(std::move(path)) {}
Database::~Database() { close(); }

Database::Database(Database&& other) noexcept 
    : db_path_(std::move(other.db_path_)), db_handle_(other.db_handle_), last_error_(std::move(other.last_error_)) {
    other.db_handle_ = nullptr;
}

bool Database::open() {
    if (sqlite3_open(db_path_.c_str(), &db_handle_) != SQLITE_OK) {
        last_error_ = db_handle_ ? sqlite3_errmsg(db_handle_) : "Adatbazis-inicializalasi hiba";
        return false;
    }
    return true;
}

void Database::close() noexcept {
    if (db_handle_) {
        sqlite3_close(db_handle_);
        db_handle_ = nullptr;
    }
}

bool Database::initialize_schema() {
    const char* ddl = 
        "CREATE TABLE IF NOT EXISTS files ("
        "path TEXT PRIMARY KEY, name TEXT NOT NULL, size INTEGER NOT NULL, "
        "mod_date TEXT NOT NULL, md5_checksum TEXT NOT NULL);";
    
    char* err_msg = nullptr;
    if (sqlite3_exec(db_handle_, ddl, nullptr, nullptr, &err_msg) != SQLITE_OK) {
        last_error_ = err_msg;
        sqlite3_free(err_msg);
        return false;
    }
    return true;
}

bool Database::save_records(const std::vector<FileRecord>& records) {
    sqlite3_exec(db_handle_, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
    
    const char* sql = "INSERT INTO files (path, name, size, mod_date, md5_checksum) VALUES (?, ?, ?, ?, ?);";
    sqlite3_stmt* stmt = nullptr;
    
    if (sqlite3_prepare_v2(db_handle_, sql, -1, &stmt, nullptr) != SQLITE_OK) {
        last_error_ = sqlite3_errmsg(db_handle_);
        sqlite3_exec(db_handle_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    }
    
    for (const auto& rec : records) {
        sqlite3_bind_text(stmt, 1, rec.path.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, rec.name.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_int64(stmt, 3, static_cast<sqlite3_int64>(rec.size));
        sqlite3_bind_text(stmt, 4, rec.mod_date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 5, rec.md5_checksum.c_str(), -1, SQLITE_TRANSIENT);
        
        sqlite3_step(stmt);
        sqlite3_reset(stmt);
    }
    
    sqlite3_finalize(stmt);
    sqlite3_exec(db_handle_, "COMMIT;", nullptr, nullptr, nullptr);
    return true;
}
