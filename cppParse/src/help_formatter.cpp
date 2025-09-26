#include "cppParse/help_formatter.hpp"
#include "cppParse/parser.hpp"
#include "cppParse/argument.hpp"
#include <sstream>
#include <iomanip>

namespace cppParse {

HelpFormatter::HelpFormatter(const Parser& parser)
    : m_parser(parser) {}

std::string HelpFormatter::format() const {
    std::stringstream ss;
    ss << "Usage: " << m_parser.m_program_name;
    if (!m_parser.m_positional_arguments.empty()) {
        for (const auto& arg : m_parser.m_positional_arguments) {
            ss << " <" << arg.m_name << ">";
        }
    }
    if (!m_parser.m_subparsers.empty()) {
        ss << " [subcommand]";
    }
    ss << " [options]" << std::endl << std::endl;

    if (!m_parser.m_description.empty()) {
        ss << m_parser.m_description << std::endl << std::endl;
    }

    auto format_args = [&](const std::vector<Argument>& args) {
        for (const auto& arg : args) {
            ss << "  ";
            std::string flags_str;
            for (size_t i = 0; i < arg.m_flags.size(); ++i) {
                flags_str += arg.m_flags[i] + (i < arg.m_flags.size() - 1 ? ", " : "");
            }
            ss << std::left << std::setw(20) << flags_str;
            ss << arg.m_help << std::endl;
        }
    };

    if (!m_parser.m_positional_arguments.empty()) {
        ss << "Positional arguments:" << std::endl;
        format_args(m_parser.m_positional_arguments);
        ss << std::endl;
    }

    if (!m_parser.m_arguments.empty()) {
        ss << "Optional arguments:" << std::endl;
        format_args(m_parser.m_arguments);
        ss << std::endl;
    }

    if (!m_parser.m_subparsers.empty()) {
        ss << "Subcommands:" << std::endl;
        for (const auto& pair : m_parser.m_subparsers) {
            ss << "  " << std::left << std::setw(20) << pair.first;
            if (pair.second->m_description.empty()) {
                 ss << "(No description)" << std::endl;
            } else {
                 ss << pair.second->m_description << std::endl;
            }
        }
        ss << std::endl;
    }
    return ss.str();
}

}
