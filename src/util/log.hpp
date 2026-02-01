#pragma once

#include <iostream>

inline bool g_debug_log_enabled = false;

#define ANSI_RESET "\033[0m"
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BOLD "\033[1m"

#define LOG_ERROR(message) \
    std::cerr << ANSI_BOLD ANSI_RED << "[x] " << message << ANSI_RESET << std::endl

#define LOG_WARN(message) \
    do { if (g_debug_log_enabled) std::cout << ANSI_YELLOW "[!] " << message << ANSI_RESET << std::endl; } while(0)

#define LOG_SUCCESS(message) \
    do { if (g_debug_log_enabled) std::cout << ANSI_GREEN "[o] " << message << ANSI_RESET << std::endl; } while(0)

// NOLINT: single-line to avoid backslash-newline-at-EOF warning
#define LOG_DEBUG(message) do { if (g_debug_log_enabled) std::cout << "[-] " << message << std::endl; } while(0)
