#include "KeyPressWatcher.h"

#include <GLFW/glfw3.h>

#include <iostream>

KeyPressWatcher::KeyPressWatcher(const int keyToWatch)
    : watchedKey(keyToWatch)
{
}

void KeyPressWatcher::update(GLFWwindow* window)
{
    const auto currentTime = std::chrono::steady_clock::now();
    const auto msecsSinceLastUpdate = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - lastUpdateTime).count();

    msecGrace -= msecsSinceLastUpdate;
    msecPenalty -= msecsSinceLastUpdate;

    if (msecGrace < 0)
    {
        msecGrace = 0;
    }
    if (msecPenalty < 0)
    {
        msecPenalty = 0;
    }

    if (glfwGetKey(window, watchedKey) == GLFW_PRESS)
    {
        pressed = true;
    }
    else if (glfwGetKey(window, watchedKey) == GLFW_RELEASE)
    {
        pressed = false;
    }

    lastUpdateTime = currentTime;
}

bool KeyPressWatcher::isOK() const
{
    std::cout << "penalty: " << msecPenalty << "\n";
    std::cout << "grace: " << msecGrace << "\n";

    if (pressed && (msecPenalty < 1 || msecGrace > 0))
    {
        return true;
    }

    return false;
}