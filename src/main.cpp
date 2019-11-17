#include "Utilities.h"
#include "SpectrogramBar.h"
#include "KeyPressWatcher.h"

#include "fmod.hpp"
#include "fmod_studio.hpp"
#include "fmod_errors.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <filesystem>
#include <chrono>
#include <numeric>

// Configs
constexpr int FFT_WINDOWS = 8192;
constexpr uint32_t WINDOW_WIDTH = 1024;
constexpr uint32_t WINDOW_HEIGHT = 768;
constexpr int BUCKETS = 64;
constexpr bool LOGARITHMIC = true;
constexpr int SOUND_FRAME_MEMORY = 60;

KeyPressWatcher watch(GLFW_KEY_ENTER);

void framebuffer_size_callback(GLFWwindow* window, const int width, const int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }

    watch.update(window);
}

bool fmodErrorCheck(const FMOD_RESULT result)
{
    if (result != FMOD_OK)
    {
        std::cout << "FMOD error: " << FMOD_ErrorString(result) << "\n";
        return false;
    }

    return true;
}

void printSpectrum(FMOD_DSP_PARAMETER_FFT* data)
{
    if (data->length > 0)
    {
        for (int i = 0; i < data->numchannels; ++i)
        {
            for (int j = 0; j < data->length; ++j)
            {
                std::cout << data->spectrum[i][j] << " ";
            }
            std::cout << "\n";
        }
    }
}

void fillCountsLinear(std::vector<float>& counts, FMOD_DSP_PARAMETER_FFT* fftData)
{
    const int bucketsize = ((fftData->length / 2) / BUCKETS) + 1;  // round up

    for (int channel = 0; channel < fftData->numchannels; ++channel)
    {
        for (int i = 0; i < fftData->length / 2; ++i)
        {
            const int currentBucket = i / bucketsize;
            const float val = fftData->spectrum[channel][i];

            counts[currentBucket] += val;
        }
    }

    const float max = *std::max_element(counts.begin(), counts.end());

    for (int i = 0; i < counts.size(); ++i)
    {
        counts[i] /= max;
    }
}

void fillCountsLog(std::vector<float>& counts, FMOD_DSP_PARAMETER_FFT* fftData)
{
    std::vector<float> limits;
    limits.resize(BUCKETS);

    const float mult = BUCKETS / 12.0f;  // magic

    for (int i = 0; i < BUCKETS; ++i)
    {
        const float curr = (44100.0f / 2) * pow(0.5f + 0.41f * (1 - exp(-1 * (mult - 1))), i + 1);  // magic
        limits[i] = curr;
        //std::cout << curr << "\n";
    }

    std::reverse(limits.begin(), limits.end());

    for (int channel = 0; channel < fftData->numchannels; ++channel)
    {
        for (int i = 0; i < fftData->length / 2; ++i)
        {
            const float currentFreq = (44100.0f / 2) * (float(i) / (fftData->length / 2));

            //if (currentFreq > 500)
            //{
            //	continue;
            //}

            if (currentFreq > limits[limits.size() - 1])
            {
                counts[counts.size() - 1] += fftData->spectrum[channel][i];
            }
            else
            {
                const int currentBucket = -1;
                for (size_t j = 0; j < limits.size() - 1; ++j)
                {
                    if (currentFreq < limits[j + size_t(1)])
                    {
                        counts[j] += fftData->spectrum[channel][i];
                        break;
                    }
                }
            }
        }
    }

    const float max = *std::max_element(counts.begin(), counts.end());

    for (int i = 0; i < counts.size(); ++i)
    {
        counts[i] /= max;
    }
}

float calculateSoundEnergy(FMOD_DSP_PARAMETER_FFT* fftData)
{
    float ret{ 0.0f };

    for (int channel = 0; channel < fftData->numchannels; ++channel)
    {
        for (int i = 0; i < fftData->length / 2; ++i)
        {
            ret += fftData->spectrum[channel][i] * fftData->spectrum[channel][i];
        }
    }

    return ret;
}

float calculateSoundEnergyInBands(FMOD_DSP_PARAMETER_FFT* fftData, const std::vector<std::pair<float, float>>& bands)
{
    float ret{ 0.0f };

    for (int channel = 0; channel < fftData->numchannels; ++channel)
    {
        for (int i = 0; i < fftData->length / 2; ++i)
        {
            const float currentFreq = (44100.0f / 2) * (float(i) / (fftData->length / 2));

            bool foundInBand = false;
            for (int band = 0; band < bands.size(); ++band)
            {
                if (currentFreq >= bands[band].first && currentFreq <= bands[band].second)
                {
                    foundInBand = true;
                    break;
                }
            }

            if (!foundInBand)
            {
                continue;
            }

            ret += fftData->spectrum[channel][i] * fftData->spectrum[channel][i];
        }
    }

    return ret;
}

float calculateEnergyVariance(const std::vector<float>& energies, const float average)
{
    float ret{ 0.0f };

    for (int i = 0; i < energies.size(); ++i)
    {
        ret += (energies[i] - average) * (energies[i] - average);
    }

    ret *= 1 / float(energies.size());

    return ret;
}

int main()
{
    // Initialize GLFW and GLAD
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Spectrum", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    FMOD_RESULT result;
    FMOD::Studio::System* studioSystem = nullptr;
    result = FMOD::Studio::System::create(&studioSystem);
    if (!fmodErrorCheck(result))
    {
        system("pause");
        return -1;
    }

    result = studioSystem->initialize(512, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, nullptr);
    if (!fmodErrorCheck(result))
    {
        system("pause");
        return -1;
    }

    FMOD::System* lowLevel;
    studioSystem->getCoreSystem(&lowLevel);

    const FMOD_MODE mode = FMOD_DEFAULT | FMOD_2D | FMOD_LOOP_NORMAL | FMOD_CREATESTREAM;

    FMOD::Sound* testSound;
    const std::string soundStr = getSoundPath("test2.mp3").string();

    result = lowLevel->createSound(soundStr.c_str(), mode, nullptr, &testSound);
    if (!fmodErrorCheck(result))
    {
        std::cout << soundStr << "\n";
        system("pause");
        return -1;
    }

    FMOD::Channel* testChannel;
    result = lowLevel->playSound(testSound, nullptr, false, &testChannel);
    if (!fmodErrorCheck(result))
    {
        system("pause");
        return -1;
    }

    FMOD::DSP* testDSP;
    result = lowLevel->createDSPByType(FMOD_DSP_TYPE_FFT, &testDSP);
    if (!fmodErrorCheck(result))
    {
        system("pause");
        return -1;
    }
    testDSP->setParameterInt(FMOD_DSP_FFT_WINDOWSIZE, FFT_WINDOWS);

    result = testChannel->addDSP(1, testDSP);
    if (!fmodErrorCheck(result))
    {
        system("pause");
        return -1;
    }


    std::vector<SpectrogramBar> bars;

    std::chrono::steady_clock::time_point earlier = std::chrono::steady_clock::now();

    std::vector<float> memory;
    memory.resize(SOUND_FRAME_MEMORY);
    uint32_t memoryPtr = 0;

    glEnable(GL_DEPTH_TEST);
    while (!glfwWindowShouldClose(window))
    {
        result = studioSystem->update();
        if (!fmodErrorCheck(result))
        {
            system("pause");
            return -1;
        }

        FMOD_DSP_PARAMETER_FFT* data = nullptr;
        result = testDSP->getParameterData(FMOD_DSP_FFT_SPECTRUMDATA, (void**)(&data), nullptr, nullptr, 0);
        if (!fmodErrorCheck(result))
        {
            system("pause");
            return -1;
        }

        std::vector<float> counts;
        counts.resize(BUCKETS);

        if constexpr (LOGARITHMIC)
        {
            fillCountsLog(counts, data);
        }
        else
        {
            fillCountsLinear(counts, data);
        }

        for (int i = 0; i < BUCKETS; ++i)
        {
            bars.emplace_back(float(i) / BUCKETS + 0.025f * (10.0f / BUCKETS), 0.1f, 0.05f * (10.0f / BUCKETS), 0.6f * counts[i]);
        }

        std::vector<std::pair<float, float>> bands;
        bands.push_back(std::make_pair<float, float>(60, 250));     // Kick
        bands.push_back(std::make_pair<float, float>(60, 210));     // Toms
        bands.push_back(std::make_pair<float, float>(120, 250));    // Snare
        bands.push_back(std::make_pair<float, float>(3000, 5000));  // hi-hat

        const float previousEnergies = std::accumulate(memory.begin(), memory.end(), 0.0f);
        const float averageEnergy = (1 / float(memory.size())) * previousEnergies;

        const float currentEnergy = calculateSoundEnergy(data);
        //const float currentEnergy = calculateSoundEnergyInBands(data, bands);
        const float variance = calculateEnergyVariance(memory, averageEnergy);

        memory[memoryPtr++] = currentEnergy;

        if (memoryPtr > memory.size() - 1)
        {
            memoryPtr = 0;
        }

        const float multiplier = -25.714f * variance + 1.5142857f;
        //const float multiplier = 1.3f;

        std::cout << multiplier << "\n";

        SpectrogramBar* beatBar = nullptr;
        SpectrogramBar* hitBar = nullptr;
        if (currentEnergy > multiplier * averageEnergy)
        {
            beatBar = new SpectrogramBar(0.05f, 0.9f, 0.3f, 0.1f);
        }

        if (currentEnergy > multiplier * averageEnergy && watch.isOK())
        {
            hitBar = new SpectrogramBar(0.5f, 0.9f, 0.3f, 0.1f);
            watch.setGraceTime(200);
        }
        else if (watch.isPressed())
        {
            watch.setPenaltyTime(200);
        }

        // Draw
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        for (const SpectrogramBar& bar : bars)
        {
            bar.draw();
        }
        if (beatBar)
        {
            beatBar->draw();
            delete beatBar;
            beatBar = nullptr;
        }
        if (hitBar)
        {
            hitBar->draw();
            delete hitBar;
            hitBar = nullptr;
        }

        // Upkeep
        processInput(window);
        glfwSwapBuffers(window);
        glfwPollEvents();

        bars.clear();

        // Time measurement
        const std::chrono::steady_clock::time_point later{ std::chrono::steady_clock::now() };
        std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::milliseconds>(later - earlier).count() << " ms\n";
        earlier = later;
    }

    result = studioSystem->release();
    if (!fmodErrorCheck(result))
    {
        system("pause");
        return -1;
    }

    glfwTerminate();

    system("pause");
    return 0;
}