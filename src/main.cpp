#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "util/log.hpp"
#include "looper/looper.hpp"

// デフォルト値
static const char *g_hostname = "localhost";
static int g_port = 5000;

// コマンドライン引数のパース
static void parse_args(int argc, char *argv[])
{
    for (int i = 1; i < argc; i++)
    {
        if ((strcmp(argv[i], "--hostname") == 0 || strcmp(argv[i], "-h") == 0) && i + 1 < argc)
        {
            g_hostname = argv[++i];
        }
        else if ((strcmp(argv[i], "--port") == 0 || strcmp(argv[i], "-p") == 0) && i + 1 < argc)
        {
            g_port = atoi(argv[++i]);
        }
        else if (strcmp(argv[i], "--debug-log") == 0 || strcmp(argv[i], "-d") == 0)
        {
            g_debug_log_enabled = true;
        }
        else if (strcmp(argv[i], "--help") == 0)
        {
            printf("Usage: %s [options]\n", argv[0]);
            printf("Options:\n");
            printf("  --hostname, -h <host>  Server hostname (default: localhost)\n");
            printf("  --port, -p <port>      Server port (default: 5000)\n");
            printf("  --debug-log, -d        Enable debug logging\n");
            printf("  --help                 Show this help\n");
            exit(0);
        }
    }
}

// グローバル設定を取得する関数
const char *get_network_hostname() { return g_hostname; }
int get_network_port() { return g_port; }

int main(int argc, char *argv[])
{
    parse_args(argc, argv);

    printf("Connecting to %s:%d\n", g_hostname, g_port);

    Looper looper;
    if (!looper_init(&looper, SCENE_TITLE))
    {
        SDL_Quit();

        return -1;
    }

    loop(&looper);
    looper_fini(&looper);

    return 0;
}