#pragma once

#include <QString>

// TTS 配置。从 data.hpp 抽出：voicevox 插件需要它，但不能拖入 data.hpp
// （data.hpp 会连带 llamaclient.h 等重依赖）。
struct TTSConfig
{
    int provider = 0;
    QString speaker_openai_edge_tts = "zh-CN-XiaoxiaoNeural";
    double speed_openai_edge_tts = 1.0;
    QString openai_endpoint;
    QString openai_apiKey;
    QString openai_model = "tts-1";
    QString openai_voice = "alloy";
    double openai_speed = 1.0;
    QString iFlytek_APPID;
    QString iFlytek_APISecret;
    QString iFlytek_APIKey;
    QString iFlytek_speaker = "x4_yezi";
    QString voicevox_dict_dir;
    QString voicevox_model;
    int voicevox_style_id;
    double voicevox_speed = 1.0;
    int tr_point;
    QString tr_provider;
    QString tr_lang_translators;
    QString tr_lang_libretranslate;
    QString tr_libretranslate_port = "5000";
    QString tr_tx_secret_id;
    QString tr_tx_secret_key;
    QString tr_tx_region;
    int tr_tx_project_id = 0;
    QString tr_tx_source_lang = "auto";
    QString tr_tx_target_lang;
};
