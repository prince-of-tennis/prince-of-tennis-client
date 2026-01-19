#include "audio.hpp"
#include "util/log.hpp"

bool audio_init(Audio *audio)
{
    // SDL_mixer初期化
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
    {
        LOG_ERROR("SDL_mixer初期化失敗: " << Mix_GetError());
        return false;
    }

    audio->se_chunks.clear();
    audio->bgm = nullptr;
    audio->se_volume = MIX_MAX_VOLUME;
    audio->bgm_volume = MIX_MAX_VOLUME;
    audio->initialized = true;

    LOG_DEBUG("オーディオシステム初期化完了");
    return true;
}

void audio_fini(Audio *audio)
{
    if (!audio->initialized)
    {
        return;
    }

    // SE解放
    for (Mix_Chunk *chunk : audio->se_chunks)
    {
        if (chunk)
        {
            Mix_FreeChunk(chunk);
        }
    }
    audio->se_chunks.clear();

    // BGM解放
    if (audio->bgm)
    {
        Mix_FreeMusic(audio->bgm);
        audio->bgm = nullptr;
    }

    Mix_CloseAudio();
    audio->initialized = false;

    LOG_DEBUG("オーディオシステム終了");
}

int audio_load_se(Audio *audio, const char *path)
{
    if (!audio->initialized)
    {
        LOG_ERROR("オーディオシステムが未初期化");
        return -1;
    }

    Mix_Chunk *chunk = Mix_LoadWAV(path);
    if (!chunk)
    {
        LOG_ERROR("SE読み込み失敗: " << path << " - " << Mix_GetError());
        return -1;
    }

    int se_id = static_cast<int>(audio->se_chunks.size());
    audio->se_chunks.push_back(chunk);

    LOG_DEBUG("SE読み込み完了: " << path << " (ID: " << se_id << ")");
    return se_id;
}

void audio_play_se(Audio *audio, int se_id, bool loop)
{
    if (!audio->initialized)
    {
        return;
    }

    if (se_id < 0 || se_id >= static_cast<int>(audio->se_chunks.size()))
    {
        LOG_ERROR("無効なSE ID: " << se_id);
        return;
    }

    Mix_Chunk *chunk = audio->se_chunks[se_id];
    if (chunk)
    {
        Mix_PlayChannel(-1, chunk, loop ? -1 : 0);
    }
}

void audio_set_se_volume(Audio *audio, int volume)
{
    if (!audio->initialized)
    {
        return;
    }

    audio->se_volume = (volume < 0) ? 0 : (volume > MIX_MAX_VOLUME) ? MIX_MAX_VOLUME : volume;

    // 全チャンネルの音量を設定
    Mix_Volume(-1, audio->se_volume);
}

bool audio_load_bgm(Audio *audio, const char *path)
{
    if (!audio->initialized)
    {
        LOG_ERROR("オーディオシステムが未初期化");
        return false;
    }

    // 既存のBGMを解放
    if (audio->bgm)
    {
        Mix_FreeMusic(audio->bgm);
        audio->bgm = nullptr;
    }

    audio->bgm = Mix_LoadMUS(path);
    if (!audio->bgm)
    {
        LOG_ERROR("BGM読み込み失敗: " << path << " - " << Mix_GetError());
        return false;
    }

    LOG_DEBUG("BGM読み込み完了: " << path);
    return true;
}

void audio_play_bgm(Audio *audio, bool loop)
{
    if (!audio->initialized || !audio->bgm)
    {
        return;
    }

    if (Mix_PlayMusic(audio->bgm, loop ? -1 : 0) < 0)
    {
        LOG_ERROR("BGM再生失敗: " << Mix_GetError());
    }
}

void audio_stop_bgm(Audio *audio)
{
    if (!audio->initialized)
    {
        return;
    }

    Mix_HaltMusic();
}

void audio_pause_bgm(Audio *audio)
{
    if (!audio->initialized)
    {
        return;
    }

    Mix_PauseMusic();
}

void audio_resume_bgm(Audio *audio)
{
    if (!audio->initialized)
    {
        return;
    }

    Mix_ResumeMusic();
}

void audio_set_bgm_volume(Audio *audio, int volume)
{
    if (!audio->initialized)
    {
        return;
    }

    audio->bgm_volume = (volume < 0) ? 0 : (volume > MIX_MAX_VOLUME) ? MIX_MAX_VOLUME : volume;
    Mix_VolumeMusic(audio->bgm_volume);
}
