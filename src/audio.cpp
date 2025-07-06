// g++ src/audio.cpp src/globals.cpp -o src/audio.exe -std=c++20 -Ilib -Ilib/fftw -Llib/fftw -lfftw3 -lm
#define MINIAUDIO_IMPLEMENTATION
#include "../lib/miniaudio.h"
#include "../lib/fftw/fftw3.h"
#include "globals.h"

#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const int SAMPLE_RATE = 44100;
const int FFT_SIZE = 2048;

ma_device_config config = {};
ma_device device = {};

float g_buffer[FFT_SIZE];
int g_index = 0;
bool g_ready = false;

std::string frequency_to_note(double freq) {
    if (freq <= 0) return "???";
    int A4 = 440;
    int noteIndex = round(12 * log2(freq / A4)) + 69;

    std::vector<std::string> notes = {"C", "C#", "D", "D#", "E", "F",
                                      "F#", "G", "G#", "A", "A#", "B"};
    int note = noteIndex % 12;
    int octave = noteIndex / 12 - 1;

    return notes[note] + std::to_string(octave);
}

// Audio input callback from mic
void mic_callback(ma_device* device, void* output, const void* input, ma_uint32 frameCount) {
    const float* fInput = (const float*)input;
    for (ma_uint32 i = 0; i < frameCount; ++i) {
        g_buffer[g_index++] = fInput[i];
        if (g_index >= FFT_SIZE) {
            g_index = 0;
            g_ready = true;
        }
    }
}

void InitAudio() {
    ma_device_config config = ma_device_config_init(ma_device_type_capture);
    config.capture.format = ma_format_f32;
    config.capture.channels = 1;
    config.sampleRate = SAMPLE_RATE;
    config.dataCallback = mic_callback;
    ma_device device;
    if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
        std::cerr << "Failed to initialize audio device\n";
        return;
    }
    
    // Start Microphone
    ma_device_start(&device);
}

void RunAudio() {
    ma_device_config config = ma_device_config_init(ma_device_type_capture);
    config.capture.format = ma_format_f32;
    config.capture.channels = 1;
    config.sampleRate = SAMPLE_RATE;
    config.dataCallback = mic_callback;
    ma_device device;
    if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
        std::cerr << "Failed to initialize audio device\n";
    }
    // Setup miniaudio capture
    ma_device_start(&device);
    std::cout << "Listening for notes...\n";

    // FFTW setup
    std::vector<double> input(FFT_SIZE);
    std::vector<fftw_complex> output(FFT_SIZE / 2 + 1);
    fftw_plan plan = fftw_plan_dft_r2c_1d(
        FFT_SIZE, input.data(), output.data(), FFTW_ESTIMATE);

    while (true) {
        if (g_ready) {
            // Copy mic buffer into FFT input (convert to double)
            for (int i = 0; i < FFT_SIZE; ++i) {
                // Optional: apply Hann window for better results
                input[i] = g_buffer[i] * (0.5 - 0.5 * cos(2.0 * M_PI * i / (FFT_SIZE - 1)));
            }

            // Run FFT
            fftw_execute(plan);

            // Analyze magnitudes
            int peakIndex = 0;
            double peakMag = 0.0;
            for (int i = 1; i < FFT_SIZE / 2 + 1; ++i) {
                double real = output[i][0];
                double imag = output[i][1];
                double mag = sqrt(real * real + imag * imag);
                if (mag > peakMag) {
                    peakMag = mag;
                    peakIndex = i;
                }
            }

            // Convert index to frequency
            double freq = peakIndex * (double)SAMPLE_RATE / FFT_SIZE;
            std::string note = frequency_to_note(freq);
            currentNote = note;
            g_ready = false;
        }
    }

    fftw_destroy_plan(plan);
    ma_device_uninit(&device);
}
