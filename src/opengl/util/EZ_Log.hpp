#pragma once

#ifdef EZ_LOG_ENABLED

#include <iostream>

#define ANSI_RESET "\033[0m"    // リセット
#define ANSI_RED "\033[31m"     // 赤
#define ANSI_GREEN "\033[32m"   // 緑
#define ANSI_YELLOW "\033[33m"  // 黄
#define ANSI_BLUE "\033[34m"    // 青
#define ANSI_BOLD "\033[1m"     // 太文字

//[ERROR] (ファイル名:行番号) メッセージ
#define EZ_LOG_ERROR(message)                                                                    \
    std::cerr << ANSI_BOLD ANSI_RED << "[x] (" << __FILE__ << ":" << __LINE__ << ") " << message \
              << ANSI_RESET << std::endl

#define EZ_LOG_WARN(message) std::cout << ANSI_YELLOW "[!] " << message << ANSI_RESET << std::endl

// ログを出力する
#define EZ_LOG_DEBUG(message) std::cout << "[-] " << message << ANSI_RESET << std::endl

#define EZ_LOG_SUCCESS(message) \
    std::cout << ANSI_GREEN << "[o] " << message << ANSI_RESET << std::endl

#else
#define EZ_LOG_ERROR(message) \
    do                        \
    {                         \
    } while (0)
#define EZ_LOG_WARN(message) \
    do                       \
    {                        \
    } while (0)
#define EZ_LOG_DEBUG(message) \
    do                        \
    {                         \
    } while (0)
#define EZ_LOG_SUCCESS(message) \
    do                          \
    {                           \
    } while (0)

#endif