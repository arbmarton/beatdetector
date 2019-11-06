#include "Utilities.h"
#include "SpectrogramBar.h"

#include "fmod.hpp"
#include "fmod_studio.hpp"
#include "fmod_errors.h"

#include "fftw3.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <filesystem>

constexpr int FFT_WINDOWS = 2048;
constexpr uint32_t WINDOW_WIDTH = 1024;
constexpr uint32_t WINDOW_HEIGHT = 768;

constexpr int buckets = 10;

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

	const FMOD_MODE mode = FMOD_DEFAULT | FMOD_2D | FMOD_LOOP_OFF | FMOD_CREATESTREAM;

	FMOD::Sound* testSound;
	const std::string soundStr = getSoundPath("lost_time.mp3").string();

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

	constexpr float vertices[] = {
	 0.5f,  0.5f, 0.0f,  // top right
	 0.5f, -0.5f, 0.0f,  // bottom right
	-0.5f, -0.5f, 0.0f,  // bottom left
	-0.5f,  0.5f, 0.0f   // top left 
	};
	constexpr unsigned int indices[] = {  // note that we start from 0!
		0, 1, 3,   // first triangle
		1, 2, 3    // second triangle
	};

	//SpectrogramBar bar(0, 0, 0.2, 0.8);

	std::vector<SpectrogramBar> bars;

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

		processInput(window);

		// Draw
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		

		for (int i = 0; i < buckets; ++i)
		{
			bars.push_back(SpectrogramBar(float(i) / buckets, 0, 0.2f, 0.8f));
		}

		for (const SpectrogramBar& bar : bars)
		{
			bar.draw();
		}

		// Upkeep
		glfwSwapBuffers(window);
		glfwPollEvents();

		bars.clear();
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