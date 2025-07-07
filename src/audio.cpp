// g++ src/audio.cpp src/globals.cpp -o src/audio.exe -std=c++20 -Ilib -Ilib/fftw -Llib/fftw -lfftw3 -lm
#include <atomic>
#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <algorithm>
#include <thread>
#include <mutex>

#define MINIAUDIO_IMPLEMENTATION
#include "../lib/miniaudio.h"
#include "../lib/fftw/fftw3.h"
#include "globals.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

const int SAMPLE_RATE = 44100;
const int FFT_SIZE = 4096;

float g_buffer[FFT_SIZE];
int g_index = 0;
bool g_ready = false;
std::mutex bufferMutex;
float processingBuffer[FFT_SIZE];
bool dataReady = false;

ma_device device;

using namespace std;

string frequency_to_note(double freq) {
    if (freq <= 0) return "???";
    int A4 = 440;
    int noteIndex = round(12 * log2(freq / A4)) + 69;
    vector<string> notes = {};
    if (SHARPS) {
        notes = {"C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"};
    } else {
        notes = {"C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"};
    }

    int note = noteIndex % 12;
    int octave = noteIndex / 12 - 1;

    return notes[note] + to_string(octave);
}

void mic_callback(ma_device* device, void* output, const void* input, ma_uint32 frameCount) {
    const float* fInput = (const float*)input;
    for (ma_uint32 i = 0; i < frameCount; ++i) {
        g_buffer[g_index++] = fInput[i];
        if (g_index >= FFT_SIZE) {
            g_index = 0;
            std::lock_guard<std::mutex> lock(bufferMutex);
            std::copy(std::begin(g_buffer), std::end(g_buffer), std::begin(processingBuffer));
            dataReady = true;
        }
    }
}

int get_fundamental_hps(const std::vector<fftw_complex>& spectrum, int fftSize, int maxHarmonics = 5) {
    int len = fftSize / 2 + 1;
    std::vector<double> hps(len, 0.0);

    for (int i = 1; i < len; ++i) {
        hps[i] = sqrt(spectrum[i][0] * spectrum[i][0] + spectrum[i][1] * spectrum[i][1]);
    }

    for (int h = 2; h <= maxHarmonics; ++h) {
        for (int i = 1; i < len / h; ++i) {
            hps[i] *= sqrt(spectrum[i * h][0] * spectrum[i * h][0] + spectrum[i * h][1] * spectrum[i * h][1]);
        }
    }

    int peakIndex = 1;
    double maxVal = hps[1];
    for (int i = 2; i < len / maxHarmonics; ++i) {
        if (hps[i] > maxVal) {
            maxVal = hps[i];
            peakIndex = i;
        }
    }

    return peakIndex;
}

void InitAudio() {
    RUNNINGPROGRAM = true;
    try {
        ma_device_config config = ma_device_config_init(ma_device_type_capture);
        config.capture.format = ma_format_f32;
        config.capture.channels = 1;
        config.sampleRate = SAMPLE_RATE;
        config.dataCallback = mic_callback;

        if (ma_device_init(NULL, &config, &device) != MA_SUCCESS) {
            cerr << "Failed to initialize audio device\n";
            return;
        }

        if (ma_device_start(&device) != MA_SUCCESS) {
            cerr << "Failed to start audio device\n";
            return;
        }

        cout << "Listening for notes...\n";

        thread audioWorker([] {
            vector<double> input(FFT_SIZE);
            vector<fftw_complex> output(FFT_SIZE / 2 + 1);
            fftw_plan plan = fftw_plan_dft_r2c_1d(FFT_SIZE, input.data(), output.data(), FFTW_ESTIMATE);

            while (RUNNINGPROGRAM) {
                bool doWork = false;

                {
                    lock_guard<mutex> lock(bufferMutex);
                    if (dataReady) {
                        for (int i = 0; i < FFT_SIZE; ++i) {
                            input[i] = processingBuffer[i] * (0.5 - 0.5 * cos(2.0 * M_PI * i / (FFT_SIZE - 1)));
                        }
                        dataReady = false;
                        doWork = true;
                    }
                }

                if (doWork) {
                    fftw_execute(plan);
                    int peakIndex = get_fundamental_hps(output, FFT_SIZE, 3);
                    if (peakIndex <= 0) peakIndex = 1;
                    if (peakIndex >= (FFT_SIZE / 2)) peakIndex = (FFT_SIZE / 2) - 1;

                    auto mag = [&](int i) {
                        return sqrt(output[i][0] * output[i][0] + output[i][1] * output[i][1]);
                    };

                    double alpha = mag(peakIndex - 1);
                    double beta = mag(peakIndex);
                    double gamma = mag(peakIndex + 1);

                    double p = 0.0;
                    double denominator = (alpha - 2 * beta + gamma);
                    if (denominator == 0 || isnan(denominator) || isinf(denominator)) {
                        p = 0;
                    } else {
                        p = 0.5 * (alpha - gamma) / denominator;
                        if (p < -0.5 || p > 0.5) p = 0;
                    }

                    double trueIndex = peakIndex + p;
                    double freq = trueIndex * (double)SAMPLE_RATE / FFT_SIZE;

                    if (freq >= 20 && freq <= 1500) {
                        CURRENTNOTE = frequency_to_note(freq);
                    }
                }
                this_thread::sleep_for(chrono::milliseconds(1));
            }
            fftw_destroy_plan(plan);
        });
        audioWorker.detach();

        while (RUNNINGPROGRAM) {
            this_thread::sleep_for(chrono::milliseconds(50));
        }

        ma_device_uninit(&device);

    } catch (const exception& e) {
        cerr << "Audio exception: " << e.what() << "\n";
    } catch (...) {
        cerr << "Audio threw unknown exception!\n";
    }
}
