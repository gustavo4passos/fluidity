#pragma once
#include "vec.hpp"
#include <SDL2/SDL.h>

struct AxisStatus
{
    vec2 leftAxis;
    vec2 rightAxis;
    vec2 triggers;
};

// TODO:
// - Multiple controllers awareness
// - Configurable threshold
// - Close controllers when disconnected

class GameController
{
public:
    GameController();
    bool Init();
    bool ProcessEvent(const SDL_Event& e);
    AxisStatus GetAxisStatus();

    bool IsControllerConnected() { return m_gameController != nullptr; }

private:
    float ConvertAxisValue(short rawValue);

    SDL_GameController* m_gameController;
    SDL_Joystick* m_joystick;

    static constexpr int THRESHOLD = 6000; // TODO: This should be configurable
};