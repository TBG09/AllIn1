#include "cppParse/parser.hpp"
#include <stdexcept>
#include <iostream>
#include <iomanip>

namespace cppParse {

// --- Parser Class Implementation ---

Parser::Parser(std::string program_name, std::string version)
    : m_program_name(std::move(program_name)), m_version(std::move(version)) {
    add_argument({"-h", "--help"})
        .help("shows help message and exits")
        .store_true();
}

void Parser::add_description(std::string description) {
    m_description = std::move(description);
}

Argument& Parser::add_argument(const std::vector<std::string>& flags) {
    Argument new_arg(flags);
    if (new_arg.m_is_positional) {
        m_positional_arguments.push_back(new_arg);
        return m_positional_arguments.back();
    } else {
        m_arguments.push_back(new_arg);
        size_t argument_index = m_arguments.size() - 1;
        for (const auto& flag : flags) {
            m_flag_map[flag] = argument_index;
        }
        return m_arguments.back();
    }
}

Parser& Parser::add_subparser(const std::string& name) {
    auto new_parser = std::make_unique<Parser>(name);
    auto& parser_ref = *new_parser;
    m_subparsers[name] = std::move(new_parser);
    return parser_ref;
}

bool Parser::is_subcommand_used(const std::string& name) const {
    return m_used_subcommand_name == name;
}

Parser& Parser::get_subparser(const std::string& name) {
    auto it = m_subparsers.find(name);
    if (it == m_subparsers.end()) {
        throw std::invalid_argument("Subparser '" + name + "' not found.");
    }
    return *it->second;
}

void Parser::apply_default_values() {
    for (const auto& arg : m_arguments) {
        if (arg.m_default_value.has_value() && m_parsed_values.find(arg.m_name) == m_parsed_values.end()) {
            m_parsed_values[arg.m_name] = *arg.m_default_value;
        }
    }
    for (const auto& arg : m_positional_arguments) {
        if (arg.m_default_value.has_value() && m_parsed_values.find(arg.m_name) == m_parsed_values.end()) {
            m_parsed_values[arg.m_name] = *arg.m_default_value;
        }
    }
}

void Parser::check_required_arguments() {
    for (const auto& arg : m_arguments) {
        if (arg.m_is_required && m_parsed_values.find(arg.m_name) == m_parsed_values.end()) {
            throw std::runtime_error("Required argument missing: " + arg.m_flags[0]);
        }
    }
    for (const auto& arg : m_positional_arguments) {
        if (arg.m_is_required && m_parsed_values.find(arg.m_name) == m_parsed_values.end()) {
            throw std::runtime_error("Required positional argument missing: " + arg.m_name);
        }
    }
}

void Parser::parse_args(int argc, char* argv[]) {
    std::vector<std::string> raw_args(argv + 1, argv + argc);

    if (argc > 1 && (raw_args[0] == "-h" || raw_args[0] == "--help")) {
        HelpFormatter formatter(*this);
        std::cout << formatter.format() << std::endl;
        exit(0);
    }

    std::vector<std::string> positional_candidates;

    for (size_t i = 0; i < raw_args.size(); ++i) {
        const std::string& arg = raw_args[i];

        auto subparser_it = m_subparsers.find(arg);
        if (subparser_it != m_subparsers.end()) {
            m_used_subcommand_name = arg;
            Parser& subparser = *subparser_it->second;
            std::vector<char*> sub_argv;
            sub_argv.push_back(const_cast<char*>(arg.c_str()));
            for (size_t j = i + 1; j < raw_args.size(); ++j) {
                sub_argv.push_back(const_cast<char*>(raw_args[j].c_str()));
            }
            subparser.parse_args(sub_argv.size(), sub_argv.data());
            return;
        }

        auto flag_it = m_flag_map.find(arg);
        if (flag_it != m_flag_map.end()) {
            Argument& argument = m_arguments[flag_it->second];
            if (argument.m_is_store_true) {
                m_parsed_values[argument.m_name] = true;
            } else if (argument.m_nargs == '*') {
                std::vector<std::string> values;
                while (i + 1 < raw_args.size() && m_flag_map.find(raw_args[i + 1]) == m_flag_map.end()) {
                    values.push_back(raw_args[++i]);
                }
                m_parsed_values[argument.m_name] = values;
            } else if (argument.m_takes_value) {
                if (++i < raw_args.size()) {
                    m_parsed_values[argument.m_name] = raw_args[i];
                } else {
                    throw std::runtime_error("Argument " + arg + " requires a value.");
                }
            }
            continue;
        }
        positional_candidates.push_back(arg);
    }

    if (positional_candidates.size() > m_positional_arguments.size()) {
        throw std::runtime_error("Unexpected positional argument: " + positional_candidates[m_positional_arguments.size()]);
    }

    for (size_t i = 0; i < positional_candidates.size(); ++i) {
        Argument& pos_arg = m_positional_arguments[i];
        m_parsed_values[pos_arg.m_name] = positional_candidates[i];
    }

    apply_default_values();
    check_required_arguments();
}

} // namespace cppParse
