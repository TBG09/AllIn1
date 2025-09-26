#pragma once

#include "cppParse/argument.hpp"
#include "cppParse/help_formatter.hpp"

#include <string>
#include <vector>
#include <map>
#include <variant>
#include <stdexcept>
#include <memory>

namespace cppParse {

class Parser {
public:
    friend class HelpFormatter; // Allow HelpFormatter to access private members

    Parser(std::string program_name, std::string version = "");

    void add_description(std::string description);
    Argument& add_argument(const std::vector<std::string>& flags);
    Parser& add_subparser(const std::string& name);

    void parse_args(int argc, char* argv[]);

    bool is_subcommand_used(const std::string& name) const;
    Parser& get_subparser(const std::string& name);

    template<typename T> T get(const std::string& name) {
        auto val_it = m_parsed_values.find(name);
        if (val_it == m_parsed_values.end()) {
            return T{};
        }
        try {
            // Use std::get_if to handle cases where the type might not be T
            if (auto* value = std::get_if<T>(&val_it->second)) {
                return *value;
            }
            // This will handle cases where a default T is acceptable
            return T{};
        } catch (const std::bad_variant_access&) {
            throw std::runtime_error("Invalid type requested for argument: " + name);
        }
    }

private:
    void check_required_arguments();
    void apply_default_values();

    std::string m_program_name;
    std::string m_version;
    std::string m_description;
    
    std::vector<Argument> m_arguments;
    std::map<std::string, size_t> m_flag_map;

    std::vector<Argument> m_positional_arguments;

    std::map<std::string, std::unique_ptr<Parser>> m_subparsers;
    std::string m_used_subcommand_name;

    // Upgraded to use the new ArgValue variant
    std::map<std::string, ArgValue> m_parsed_values;
};

} // namespace cppParse
