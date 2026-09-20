#include "CliParser.h"
#include "FileScanner.h"
#include "Database.h"
#include "DbComparer.h"
#include <iostream>
#include <filesystem>

#ifdef _WIN32
#include <windows.h>
#endif

namespace fs = std::filesystem;

static void initialize_console_environment() {
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif
}

static int handle_scan_mode(const std::string& source_dir, const std::string& output_dir) {
    const fs::path source_path(source_dir);
    const fs::path target_path(output_dir);

    if (!fs::exists(source_path) || !fs::is_directory(source_path)) {
        std::cerr << "Hiba: A forras konyvtar ('" << source_dir << "') nem letezik!\n";
        return 1;
    }
    if (!fs::exists(target_path) || !fs::is_directory(target_path)) {
        std::cerr << "Hiba: A kimeneti konyvtar ('" << output_dir << "') nem letezik!\n";
        return 1;
    }

    const auto records = FileScanner::scan_directory_multithreaded(source_path);
    const std::string db_filename = FileScanner::generate_timestamped_filename("backup_integrity_check", ".db");
    const fs::path full_db_path = target_path / db_filename;

    std::cout << "[3/3] Rekordok mentese SQLite adatbazisba...\n";
    Database db(full_db_path.string());
    if (!db.open() || !db.initialize_schema() || !db.save_records(records)) {
        std::cerr << "Adatbazis hiba: " << db.last_error() << "\n";
        return 1;
    }

    std::cout << "\nSikeresen elmentve:\n-> " << full_db_path.string() << " (" << records.size() << " fajl)\n";
    return 0;
}

int main(int argc, char* argv[]) {
    initialize_console_environment();
    const CliOptions opts = CliParser::parse(argc, argv);

    if (opts.mode == AppMode::Help) {
        CliParser::print_usage();
        return 0;
    }
    
    if (!opts.error_message.empty()) {
        std::cerr << "Hiba: " << opts.error_message << "\n";
        std::cerr << "Tipp: Hasznald a 'backup_integrity_checker.exe -h' parancsot a sugohoz.\n";
        return 1; 
    }
    
    if (opts.mode == AppMode::Compare) {
        return DbComparer::execute(
            opts.positional_args[0], 
            opts.positional_args[1], 
            opts.verbose, 
            opts.full_check, 
            opts.plain_output, 
            opts.json_output
        );
    }

    return handle_scan_mode(opts.positional_args[0], opts.output_dir);
}
