#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>
#include <cstdint>

/**
 * @brief Egy vizsgalt fajl pillanatnyi allapotat leiro adatmodell.
 */
struct FileRecord {
    std::string path;           // Relativ vagy abszolut utvonal
    std::string name;           // Fajl neve
    std::uintmax_t size{0};     // Fajl merete bajtokban
    std::string mod_date;       // Utolso modositas ideje formázva
    std::string md5_checksum;   // MD5 hash a tartalomrol
};

/**
 * @brief Egy modosult fajl valtozasait tartalmazo adatmodell.
 */
struct ModifiedFileRecord {
    std::string path;
    std::uintmax_t old_size{0};
    std::uintmax_t new_size{0};
    std::string old_date;
    std::string new_date;
    std::string old_md5;
    std::string new_md5;
    std::string reason;         // A valtozas oka (pl. "Meret valtozott")
};

/**
 * @brief Az osszehasonlitas vegeredmenyet osszefogo struktura.
 */
struct ComparisonResult {
    std::vector<FileRecord> missing;
    std::vector<ModifiedFileRecord> modified;
    std::vector<FileRecord> added;

    // Ellenorzi, hogy tortent-e barmilyen valtozas
    [[nodiscard]] bool empty() const noexcept {
        return missing.empty() && modified.empty() && added.empty();
    }
};

#endif // MODELS_H
