#ifndef DB_COMPARER_H
#define DB_COMPARER_H

#include "Models.h"
#include <string>
#include <sqlite3.h>

/**
 * @brief Ket adatbazis allapotat veti ossze SQL hivasokkal.
 */
class DbComparer {
public:
    static int execute(const std::string& old_db_path, const std::string& new_db_path, 
                       bool verbose, bool full_check, bool plain_output, bool json_output);

private:
    static ComparisonResult run_diff_queries(sqlite3* db, bool full_check);
    static std::vector<FileRecord> fetch_file_records(sqlite3* db, const std::string& query);
    static std::vector<ModifiedFileRecord> fetch_modified_records(sqlite3* db, const std::string& query);
    static void print_results(const ComparisonResult& res, bool verbose, bool plain_output, bool json_output);
};

#endif // DB_COMPARER_H
