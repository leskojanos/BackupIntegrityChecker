#include "DbComparer.h"
#include <iostream>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

std::vector<FileRecord> DbComparer::fetch_file_records(sqlite3* db, const std::string& query) {
    std::vector<FileRecord> records;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            FileRecord rec;
            rec.path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            rec.size = static_cast<std::uintmax_t>(sqlite3_column_int64(stmt, 1));
            rec.mod_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 2));
            rec.md5_checksum = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            records.push_back(std::move(rec));
        }
        sqlite3_finalize(stmt);
    }
    return records;
}

std::vector<ModifiedFileRecord> DbComparer::fetch_modified_records(sqlite3* db, const std::string& query) {
    std::vector<ModifiedFileRecord> records;
    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, nullptr) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            ModifiedFileRecord rec;
            rec.path = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            rec.old_size = static_cast<std::uintmax_t>(sqlite3_column_int64(stmt, 1));
            rec.new_size = static_cast<std::uintmax_t>(sqlite3_column_int64(stmt, 2));
            rec.old_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 3));
            rec.new_date = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 4));
            rec.old_md5 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 5));
            rec.new_md5 = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 6));

            if (rec.old_size != rec.new_size) rec.reason = "Meret valtozott";
            else if (rec.old_md5 != rec.new_md5) rec.reason = "Tartalom valtozott (MD5)";
            else if (rec.old_date != rec.new_date) rec.reason = "Datum valtozott";
            else rec.reason = "Ismeretlen elteres";

            records.push_back(std::move(rec));
        }
        sqlite3_finalize(stmt);
    }
    return records;
}

ComparisonResult DbComparer::run_diff_queries(sqlite3* db, bool full_check) {
    ComparisonResult result;
    
    const std::string q_missing = 
        "SELECT main.files.path, main.files.size, main.files.mod_date, main.files.md5_checksum "
        "FROM main.files LEFT JOIN new_db.files ON main.files.path = new_db.files.path "
        "WHERE new_db.files.path IS NULL;";

    const std::string q_new = 
        "SELECT new_db.files.path, new_db.files.size, new_db.files.mod_date, new_db.files.md5_checksum "
        "FROM new_db.files LEFT JOIN main.files ON new_db.files.path = main.files.path "
        "WHERE main.files.path IS NULL;";

    std::string q_modified;
    if (full_check) {
        q_modified = 
            "SELECT main.files.path, main.files.size, new_db.files.size, "
            "main.files.mod_date, new_db.files.mod_date, main.files.md5_checksum, new_db.files.md5_checksum "
            "FROM main.files JOIN new_db.files ON main.files.path = new_db.files.path "
            "WHERE main.files.md5_checksum != new_db.files.md5_checksum;";
    } else {
        q_modified = 
            "SELECT main.files.path, main.files.size, new_db.files.size, "
            "main.files.mod_date, new_db.files.mod_date, main.files.md5_checksum, new_db.files.md5_checksum "
            "FROM main.files JOIN new_db.files ON main.files.path = new_db.files.path "
            "WHERE main.files.size != new_db.files.size "
            "   OR main.files.md5_checksum != new_db.files.md5_checksum;";
    }

    result.missing = fetch_file_records(db, q_missing);
    result.modified = fetch_modified_records(db, q_modified);
    result.added = fetch_file_records(db, q_new);
    return result;
}

void DbComparer::print_results(const ComparisonResult& res, bool verbose, bool plain_output, bool json_output) {
    if (json_output) {
        json j;
        j["status"] = res.empty() ? "ok" : "changes_detected";
        j["summary"] = {
            {"missing", res.missing.size()},
            {"modified", res.modified.size()},
            {"added", res.added.size()}
        };

        j["details"]["missing"] = json::array();
        for (const auto& f : res.missing) {
            j["details"]["missing"].push_back({
                {"path", f.path}, {"size", f.size}, {"date", f.mod_date}, {"md5", f.md5_checksum}
            });
        }

        j["details"]["modified"] = json::array();
        for (const auto& f : res.modified) {
            j["details"]["modified"].push_back({
                {"path", f.path}, {"reason", f.reason},
                {"old_size", f.old_size}, {"new_size", f.new_size},
                {"old_date", f.old_date}, {"new_date", f.new_date},
                {"old_md5", f.old_md5}, {"new_md5", f.new_md5}
            });
        }

        j["details"]["added"] = json::array();
        for (const auto& f : res.added) {
            j["details"]["added"].push_back({
                {"path", f.path}, {"size", f.size}, {"date", f.mod_date}, {"md5", f.md5_checksum}
            });
        }

        std::cout << j.dump(4) << "\n";
        return;
    }

    if (plain_output) {
        for (const auto& f : res.missing)  std::cout << "DEL|" << f.path << "\n";
        for (const auto& f : res.modified) std::cout << "MOD|" << f.path << "\n";
        for (const auto& f : res.added)    std::cout << "ADD|" << f.path << "\n";
        return; 
    }

    std::cout << std::string(60, '-') << "\n"
              << "OSSZEHASONLITAS EREDMENYE (SQLite Engine)\n"
              << std::string(60, '-') << "\n";
              
    if (res.empty()) {
        std::cout << "\xE2\x9C\x85 A ket allapot teljesen megegyezik! Nincs valtozas.\n";
        return;
    }
    if (!res.missing.empty()) {
        std::cout << "\xE2\x9D\x8C HIANYZO FAJLOK (" << res.missing.size() << " db):\n";
        for (const auto& f : res.missing) {
            std::cout << "   - " << f.path << "\n";
            if (verbose) std::cout << "       [Meret: " << f.size << " byte | MD5: " << f.md5_checksum << "]\n";
        }
        std::cout << "\n";
    }
    if (!res.modified.empty()) {
        std::cout << "\xE2\x9A\xA0\xEF\xB8\x8F MODOSULT FAJLOK (" << res.modified.size() << " db):\n";
        for (const auto& f : res.modified) {
            std::cout << "   - " << f.path << " (" << f.reason << ")\n";
            if (verbose) {
                std::cout << "       [Meret: " << f.old_size << " -> " << f.new_size << " byte"
                          << " | MD5: " << f.old_md5 << " -> " << f.new_md5 << "]\n";
            }
        }
        std::cout << "\n";
    }
    if (!res.added.empty()) {
        std::cout << "\xE2\x9E\x95 UJ FAJLOK (" << res.added.size() << " db):\n";
        for (const auto& f : res.added) {
            std::cout << "   - " << f.path << "\n";
            if (verbose) std::cout << "       [Meret: " << f.size << " byte | MD5: " << f.md5_checksum << "]\n";
        }
        std::cout << "\n";
    }
}

int DbComparer::execute(const std::string& old_db_path, const std::string& new_db_path, 
                        bool verbose, bool full_check, bool plain_output, bool json_output) {
                            
    if (!plain_output && !json_output) {
        std::cout << "Adatbazisok csatolasa...\n Regi: " << old_db_path << "\n Uj:   " << new_db_path << "\n";
        if (full_check) std::cout << "Mod: Teljes MD5 ellenorzes (--full-check)\n\n";
        else std::cout << "Mod: Gyors ketlepcsos integritas-ellenorzes\n\n";
    }

    sqlite3* db = nullptr;
    if (sqlite3_open(old_db_path.c_str(), &db) != SQLITE_OK) {
        if (!plain_output && !json_output) std::cerr << "Hiba a regi adatbazis megnyitasakor: " << sqlite3_errmsg(db) << "\n";
        return 1;
    }
    
    const std::string attach_sql = "ATTACH DATABASE '" + new_db_path + "' AS new_db;";
    char* err_msg = nullptr;
    if (sqlite3_exec(db, attach_sql.c_str(), nullptr, nullptr, &err_msg) != SQLITE_OK) {
        if (!plain_output && !json_output) std::cerr << "Hiba az uj adatbazis csatolasakor: " << err_msg << "\n";
        sqlite3_free(err_msg);
        sqlite3_close(db);
        return 1;
    }
    
    ComparisonResult result = run_diff_queries(db, full_check);
    sqlite3_close(db);
    
    print_results(result, verbose, plain_output, json_output);
    
    return result.empty() ? 0 : 2;
}
