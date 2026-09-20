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
                opts.error_message = "A '-o' parameter utan meg kell adni a mentesi konyvtarat!";
                return opts;
            }
        } else {
            opts.positional_args.push_back(arg);
        }
    }

    if (opts.positional_args.empty()) {
        opts.error_message = "Hianyzo konyvtar- vagy adatbazis-parameter!";
    } else if (opts.mode == AppMode::Compare && opts.positional_args.size() != 2) {
        opts.error_message = "Az osszehasonlitashoz pontosan ket adatbazisfajl (.db) szukseges!";
    }

    return opts;
}

void CliParser::print_usage() {
    std::cout << "==========================================================\n"
              << "       BACKUP INTEGRITY CHECKER - Hasznalati utmutato     \n"
              << "==========================================================\n\n"
              << "1. SZKENNELES MOD (Parhuzamositott tobbmagos feldolgozas)\n"
              << "   Hasznalat: backup_integrity_checker.exe <forras_mappa> [-o <kimeneti_mappa>]\n\n"
              << "2. OSSZEHASONLITO MOD (Ketlepcsos optimalizalt diff)\n"
              << "   Hasznalat: backup_integrity_checker.exe -c <regi.db> <uj.db> [Kimeneti_Opciok]\n"
              << "   Példa: backup_integrity_checker.exe -cv -f alap.db uj.db\n\n"
              << "OPCIOK:\n"
              << "   -o <mappa>       Kimeneti konyvtar mentesehez.\n"
              << "   -c               Osszehasonlito mod.\n"
              << "   -f, --full-check Teljes hash ellenorzes kenyszeritese (szigoru MD5 diff).\n"
              << "   -v, --verbose    Reszletes vizualis konzolkimenet.\n"
              << "   -p, --plain      Szkript-barat listaskimenet (AKCIO|PATH).\n"
              << "   -j, --json       Strukturalt JSON kimenet gepi feldolgozashoz.\n"
              << "   -h, --help       Megjeleniti ezt a leirast.\n";
}
