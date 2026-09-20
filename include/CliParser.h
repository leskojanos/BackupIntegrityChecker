#ifndef CLI_PARSER_H
#define CLI_PARSER_H

#include <string>
#include <vector>

enum class AppMode {
    Scan,
    Compare,
    Help
};

/**
 * @brief A parancssori argumentumok feldolgozott eredmenyeit tarolja.
 */
struct CliOptions {
    AppMode mode{AppMode::Scan};
    bool verbose{false};
    bool full_check{false};
    bool plain_output{false};
    bool json_output{false};
    std::string output_dir{"."};
    std::vector<std::string> positional_args;
    std::string error_message;
};

class CliParser {
public:
    static CliOptions parse(int argc, char* argv[]);
    static void print_usage();
};

#endif // CLI_PARSER_H
