#pragma once

#include <iostream>

#define ANSI_RESET "\033[0m"    // リセット (必須)
#define ANSI_RED "\033[31m"     // 赤
#define ANSI_GREEN "\033[32m"   // 緑
#define ANSI_YELLOW "\033[33m"  // 黄
#define ANSI_BLUE "\033[34m"    // 青
#define ANSI_BOLD "\033[1m"     // 太文字
// エラーログを出力する
//[ERROR] (ファイル名:行番号) メッセージ
#define LOG_ERROR(message)                                                                       \
    std::cerr << ANSI_BOLD ANSI_RED << "[x] (" << __FILE__ << ":" << __LINE__ << ") " << message \
              << ANSI_RESET << std::endl

#define LOG_WARN(message) std::cout << ANSI_YELLOW "[!] " << message << ANSI_RESET << std::endl

#ifdef DEBUG
// ログを出力する
// (CMakeで -DDEBUG=1 を指定した時だけ有効になる)
#define LOG_DEBUG(message) std::cout << "[-] " << message << ANSI_RESET << std::endl

#define LOG_SUCCESS(message) std::cout << ANSI_GREEN << "[o] " << message << ANSI_RESET << std::endl

#else
#define LOG_DEBUG(message)

#endif