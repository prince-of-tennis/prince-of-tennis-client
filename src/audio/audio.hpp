#pragma once

#include <SDL2/SDL_mixer.h>
#include <vector>

// SE管理用の最大数
constexpr int AUDIO_MAX_SE = 64;

struct Audio
{
    std::vector<Mix_Chunk *> se_chunks;  // SE（効果音）リスト
    Mix_Music *bgm;                       // 現在のBGM
    int se_volume;                        // SE音量 (0-128)
    int bgm_volume;                       // BGM音量 (0-128)
    bool initialized;                     // 初期化済みフラグ
};

// 基本関数
bool audio_init(Audio *audio);
void audio_fini(Audio *audio);

// SE関数
int audio_load_se(Audio *audio, const char *path);             // SE読み込み → ID返却 (-1で失敗)
void audio_play_se(Audio *audio, int se_id, bool loop=false);  // SE再生 (loop=trueで無限ループ)
void audio_set_se_volume(Audio *audio, int volume);            // SE音量設定 (0-128)

// BGM関数
bool audio_load_bgm(Audio *audio, const char *path);    // BGM読み込み
void audio_play_bgm(Audio *audio, bool loop=true);      // BGM再生 (loop=trueで無限ループ)
void audio_stop_bgm(Audio *audio);                     // BGM停止
void audio_pause_bgm(Audio *audio);                    // BGM一時停止
void audio_resume_bgm(Audio *audio);                   // BGM再開
void audio_set_bgm_volume(Audio *audio, int volume);   // BGM音量設定 (0-128)
