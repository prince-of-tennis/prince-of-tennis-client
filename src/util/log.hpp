#pragma once

#include <iostream>

// エラーログを出力する
//[ERROR] (ファイル名:行番号) メッセージ
#define LOG_ERROR(message) \
    std::cerr << "[ERROR] (" << __FILE__ << ":" << __LINE__ << ") " << message << std::endl

#ifdef DEBUG
// ログを出力する
// (CMakeで -DDEBUG=1 を指定した時だけ有効になる)
#define LOG_DEBUG(message) std::cout << "[DEBUG] " << message << std::endl
#else
#define LOG_DEBUG(message)
#endif