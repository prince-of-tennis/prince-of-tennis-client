#pragma once

#include <iostream>

#define ANSI_RESET "\033[0m"
#define ANSI_RED "\033[31m"
#define ANSI_GREEN "\033[32m"
#define ANSI_YELLOW "\033[33m"
#define ANSI_BOLD "\033[1m"

#define LOG_ERROR(message) \
    std::cerr << ANSI_BOLD ANSI_RED << "[x] " << message << ANSI_RESET << std::endl

#define LOG_WARN(message) \
    std::cout << ANSI_YELLOW "[!] " << message << ANSI_RESET << std::endl

#define LOG_SUCCESS(message) \
    std::cout << ANSI_GREEN "[o] " << message << ANSI_RESET << std::endl

#ifdef DEBUG
#define LOG_DEBUG(message) std::cout << "[-] " << message << std::endl
#else
#define LOG_DEBUG(message)
#endif