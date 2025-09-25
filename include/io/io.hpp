#pragma once

#include <argparse/argparse.hpp>

void register_io_commands(argparse::ArgumentParser& parser);
void handle_io_create(const argparse::ArgumentParser& create_command, bool output_enabled);
