#pragma once

#include <chrono>

struct GLFWwindow;

class KeyPressWatcher
{
public:
    KeyPressWatcher(const int keyToWatch);

    void update(GLFWwindow* window);

    bool isOK() const;
    bool isPressed() const
    {
        return pressed;
    }

    void setGraceTime(const long long msecs)
    {
        msecGrace = msecs;
    }
    void setPenaltyTime(const long long msecs)
    {
        msecPenalty = msecs;
    }

private:
    const int watchedKey;

    std::chrono::steady_clock::time_point lastUpdateTime{ std::chrono::steady_clock::now() };

    long long msecPenalty{ 0 };
    long long msecGrace{ 0 };

    bool pressed{ false };
};