#include <iostream>
#include <argparse/argparse.hpp>
#include "io/io.hpp"

int main(int argc, char *argv[]) {
    argparse::ArgumentParser program("AllIn1");

    program.add_argument("--output")
        .help("Enable output messages")
        .default_value(false)
        .implicit_value(true);

    // Create a subcommand for 'io' and register its commands
    argparse::ArgumentParser io_command("io");
    register_io_commands(io_command);
    program.add_subparser(io_command);

    try {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err) {
        std::cerr << err.what() << std::endl;
        std::cerr << program;
        return 1;
    }

    auto output_enabled = program.get<bool>("--output");

    if (program.is_subcommand_used(io_command)) {
        if (io_command.is_subcommand_used("create")) {
            handle_io_create(io_command.at<argparse::ArgumentParser>("create"), output_enabled);
        }
    } else if (!output_enabled) {
        // No command used, and output is disabled, do nothing.
    } else {
        std::cout << "Welcome to AllIn1. Use --help to see commands." << std::endl;
    }

    return 0;
}
