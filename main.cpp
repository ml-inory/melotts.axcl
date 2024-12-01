/**************************************************************************************************
 *
 * Copyright (c) 2019-2023 Axera Semiconductor (Ningbo) Co., Ltd. All Rights Reserved.
 *
 * This source file is the property of Axera Semiconductor (Ningbo) Co., Ltd. and
 * may not be copied or distributed in any isomorphic form without the prior
 * written consent of Axera Semiconductor (Ningbo) Co., Ltd.
 *
 **************************************************************************************************/
#include <stdio.h>
#include <string>
#include <unordered_map>
#include <fstream>
#include <iostream>
#include <cmath>
#include <axcl.h>

#include "cmdline.h"
#include "OnnxWrapper.hpp"
#include <ax_sys_api.h>
#include "middleware/axcl_runtime_runner.hpp"
#include "AudioFile.h"
#include "Lexicon.hpp"
#include "utilities/timer.hpp"

const char* CONFIG_FILE_DEFAULT = "/usr/local/axcl/axcl.json";

static std::vector<int> intersperse(const std::vector<int>& lst, int item) {
    std::vector<int> result(lst.size() * 2 + 1, item);
    for (size_t i = 1; i < result.size(); i+=2) {
        result[i] = lst[i / 2];
    }
    return result;
}

static int calc_product(const std::vector<int64_t>& dims) {
    int64_t result = 1;
    for (auto i : dims)
        result *= i;
    return result;
}

std::unique_ptr<middleware::runner> load_runner(const std::string& model_path) {
    std::unique_ptr<middleware::runner> runner = std::make_unique<middleware::runtime_runner>();

    if (!runner->init(CONFIG_FILE_DEFAULT, 0, 0)) {
        fprintf(stderr, "[ERROR] Init failed.\n");
        return nullptr;
    }

    if (!runner->load(model_path)) {
        fprintf(stderr, "[ERROR] Loading model {%s} failed.\n", model_path.c_str());
        return nullptr;
    }

    if (!runner->prepare(true, true, 0, 0)) {
        fprintf(stderr, "[ERROR] Prepare for model {%s} failed.\n", model_path.c_str());
        return nullptr;
    }

    return std::move(runner);
}

int main(int argc, char** argv) {
    cmdline::parser cmd;
    cmd.add<std::string>("encoder", 'e', "encoder onnx", false, "./models/encoder.onnx");
    cmd.add<std::string>("decoder", 'd', "decoder axmodel", false, "./models/decoder.axmodel");
    cmd.add<std::string>("lexicon", 'l', "lexicon.txt", false, "./models/lexicon.txt");
    cmd.add<std::string>("token", 't', "tokens.txt", false, "./models/tokens.txt");
    cmd.add<std::string>("g", 0, "g.bin", false, "./models/g.bin");

    cmd.add<std::string>("sentence", 's', "input sentence", false, "爱芯元智半导体股份有限公司，致力于打造世界领先的人工智能感知与边缘计算芯片。服务智慧城市、智能驾驶、机器人的海量普惠的应用");
    cmd.add<std::string>("wav", 'w', "wav file", false, "output.wav");

    cmd.add<float>("speed", 0, "speak speed", false, 0.8f);
    cmd.add<int>("sample_rate", 0, "sample rate", false, 44100);
    cmd.parse_check(argc, argv);

    auto encoder_file   = cmd.get<std::string>("encoder");
    auto decoder_file   = cmd.get<std::string>("decoder");
    auto lexicon_file   = cmd.get<std::string>("lexicon");
    auto token_file     = cmd.get<std::string>("token");
    auto g_file         = cmd.get<std::string>("g");

    auto sentence       = cmd.get<std::string>("sentence");
    auto wav_file       = cmd.get<std::string>("wav");

    auto speed          = cmd.get<float>("speed");
    auto sample_rate    = cmd.get<int>("sample_rate");

    printf("encoder: %s\n", encoder_file.c_str());
    printf("decoder: %s\n", decoder_file.c_str());
    printf("lexicon: %s\n", lexicon_file.c_str());
    printf("token: %s\n", token_file.c_str());
    printf("sentence: %s\n", sentence.c_str());
    printf("wav: %s\n", wav_file.c_str());
    printf("speed: %f\n", speed);
    printf("sample_rate: %d\n", sample_rate);

    utilities::timer timer;

    // Load lexicon
    Lexicon lexicon(lexicon_file, token_file);

    // Convert sentence to phones and tones
    std::vector<int> phones_bef, tones_bef;
    lexicon.convert(sentence, phones_bef, tones_bef);

    // for (auto p : phones_bef) {
    //     printf("%d ", p);
    // }
    // printf("\n");

    // Add blank between words
    auto phones = intersperse(phones_bef, 0);
    auto tones = intersperse(tones_bef, 0);
    int phone_len = phones.size();

    std::vector<int> langids(phone_len, 3);

    // Read g.bin
    std::vector<float> g(256, 0);
    FILE* fp = fopen(g_file.c_str(), "rb");
    if (!fp) {
        printf("Open %s failed!\n", g_file.c_str());
        return -1;
    }
    fread(g.data(), sizeof(float), g.size(), fp);
    fclose(fp);

    printf("Load encoder\n");
    OnnxWrapper encoder;
    if (0 != encoder.Init(encoder_file)) {
        printf("encoder init failed!\n");
        return -1;
    }

    printf("Load decoder model\n");
    auto decoder_model = load_runner(decoder_file);
    if (!decoder_model) {
        printf("Init decoder model failed!\n");
        return -1;
    }

    float noise_scale   = 0.3f;
    float length_scale  = 1.0 / speed;
    float noise_scale_w = 0.6f;
    float sdp_ratio     = 0.2f;
    timer.start();
    auto encoder_output = encoder.Run(phones, tones, langids, g, noise_scale, noise_scale_w, length_scale, sdp_ratio);
    timer.stop();
    printf("Encoder run take %.2fms\n", timer.elapsed<utilities::timer::milliseconds>());

    float* zp_data = encoder_output.at(0).GetTensorMutableData<float>();
    int audio_len = encoder_output.at(2).GetTensorMutableData<int>()[0];
    auto zp_info = encoder_output.at(0).GetTensorTypeAndShapeInfo();
    auto zp_shape = zp_info.GetShape();

    int zp_size = decoder_model->get_input_size(0) / sizeof(float);
    int dec_len = zp_size / zp_shape[1];
    int audio_slice_len = decoder_model->get_output_size(0) / sizeof(float);
    std::vector<float> decoder_output(audio_slice_len);

    int dec_slice_num = int(std::ceil(zp_shape[2] * 1.0 / dec_len));
    printf("decoder slice num: %d\n", dec_slice_num);
    std::vector<float> wavlist;
    for (int i = 0; i < dec_slice_num; i++) {
        timer.start();

        std::vector<float> zp(zp_size, 0);
        int actual_size = (i + 1) * dec_len < zp_shape[2] ? dec_len : zp_shape[2] - i * dec_len;
        for (int n = 0; n < zp_shape[1]; n++) {
            memcpy(zp.data() + n * dec_len, zp_data + n * zp_shape[2] + i * dec_len, sizeof(float) * actual_size);
        }

        axclrtMemcpy(decoder_model->get_input_pointer(0), zp.data(), sizeof(float) * zp_size, AXCL_MEMCPY_HOST_TO_DEVICE);
        axclrtMemcpy(decoder_model->get_input_pointer(1), g.data(), sizeof(float) * g.size(), AXCL_MEMCPY_HOST_TO_DEVICE);
        if (!decoder_model->run(false)) {
            printf("Run decoder model failed!\n");
            return -1;
        }
        axclrtMemcpy(decoder_output.data(), decoder_model->get_output_pointer(0), sizeof(float) * decoder_output.size(), AXCL_MEMCPY_DEVICE_TO_HOST);
        
        actual_size = (i + 1) * audio_slice_len < audio_len ? audio_slice_len : audio_len - i * audio_slice_len;
        wavlist.insert(wavlist.end(), decoder_output.begin(), decoder_output.begin() + actual_size);

        timer.stop();
        printf("Decode slice(%d/%d) take %.2fms\n", i + 1, dec_slice_num, timer.elapsed<utilities::timer::milliseconds>());
    }

    AudioFile<float> audio_file;
    std::vector<std::vector<float> > audio_samples{wavlist};
    audio_file.setAudioBuffer(audio_samples);
    audio_file.setSampleRate(sample_rate);
    if (!audio_file.save(wav_file)) {
        printf("Save audio file failed!\n");
        return -1;
    }

    printf("Saved audio to %s\n", wav_file.c_str());

    return 0;
}