#include "CliParser.h"
#include <iostream>

CliOptions CliParser::parse(int argc, char* argv[]) {
    CliOptions opts;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];

        if (arg == "-h" || arg == "--help") {
            opts.mode = AppMode::Help;
            return opts;
        }
        
        if (arg == "-c") {
            opts.mode = AppMode::Compare;
        } else if (arg == "-v" || arg == "--verbose") {
            opts.verbose = true;
        } else if (arg == "-f" || arg == "--full-check") {
            opts.full_check = true;
        } else if (arg == "-p" || arg == "--plain") {
            opts.plain_output = true;
        } else if (arg == "-j" || arg == "--json") {
            opts.json_output = true;
        } else if (arg == "-cv" || arg == "-vc") {
            opts.mode = AppMode::Compare;
            opts.verbose = true;
        } else if (arg == "-o") {
            if (i + 1 < argc) {
                opts.output_dir = argv[++i];
            } else {
                opts.error_message = "Error: Output directory must be specified after '-o'!";
                return opts;
            }
        } else {
            opts.positional_args.push_back(arg);
        }
    }

    if (opts.positional_args.empty()) {
        opts.error_message = "Error: Missing directory or database parameter!";
    } else if (opts.mode == AppMode::Compare && opts.positional_args.size() != 2) {
        opts.error_message = "Error: Exactly two database files (.db) are required for comparison!";
    }

    return opts;
}

void CliParser::print_usage() {
    std::cout << "==========================================================\n"
              << "       BACKUP INTEGRITY CHECKER - Usage Guide             \n"
              << "==========================================================\n\n"
              << "1. SCAN MODE (Parallel multi-threaded processing)\n"
              << "   Usage: backup_integrity_checker.exe <source_folder> [-o <output_folder>]\n\n"
              << "2. COMPARE MODE (Optimized two-phase diff)\n"
              << "   Usage: backup_integrity_checker.exe -c <old.db> <new.db> [Output_Options]\n"
              << "   Example: backup_integrity_checker.exe -cv -f base.db new.db\n\n"
              << "OPTIONS:\n"
              << "   -o <folder>      Specify the output directory for scanning.\n"
              << "   -c               Enable Compare (diff) mode.\n"
              << "   -f, --full-check Force full MD5 hash check (ignores size-based pre-filtering).\n"
              << "   -v, --verbose    Detailed visual console output (dates, sizes, hashes).\n"
              << "   -p, --plain      Script-friendly, simple list output (ACTION|PATH).\n"
              << "   -j, --json       Structured JSON output for machine processing.\n"
              << "   -h, --help       Display this usage guide.\n";
}
