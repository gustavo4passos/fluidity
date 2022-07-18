#include "input/game_controller.hpp"
#include "utils/logger.h"
#include <cmath>
#include <iostream>

GameController::GameController()
    : m_gameController(nullptr)
{ }

bool GameController::Init()
{
    if (SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) < 0)
    {
        LOG_ERROR("Unable to initialize SDL GameController subsystem: " + 
            std::string(SDL_GetError()));
            return false;
    }
    return true;
}

AxisStatus GameController::GetAxisStatus()
{
    if (m_gameController == nullptr) return {};
    
    return {
        vec2{
            ConvertAxisValue(SDL_GameControllerGetAxis(m_gameController, 
                SDL_CONTROLLER_AXIS_LEFTX)),
            ConvertAxisValue(SDL_GameControllerGetAxis(m_gameController, 
                SDL_CONTROLLER_AXIS_LEFTY))
        },
        vec2{
            ConvertAxisValue(SDL_GameControllerGetAxis(m_gameController, 
                SDL_CONTROLLER_AXIS_RIGHTX)),
            ConvertAxisValue(SDL_GameControllerGetAxis(m_gameController, 
                SDL_CONTROLLER_AXIS_RIGHTY))
        },
        vec2{
            ConvertAxisValue(SDL_GameControllerGetAxis(m_gameController,
                SDL_CONTROLLER_AXIS_TRIGGERLEFT)),
            ConvertAxisValue(SDL_GameControllerGetAxis(m_gameController,
                SDL_CONTROLLER_AXIS_TRIGGERRIGHT))
        }
    };
}

bool GameController::ProcessEvent(const SDL_Event& e)
{
    switch(e.type)
    {
        case SDL_CONTROLLERDEVICEADDED:
        {
            if (e.cdevice.which == 0)
            {
                m_gameController = SDL_GameControllerOpen(0);
                m_joystick = SDL_GameControllerGetJoystick(m_gameController);
            }
            break;
        }

        case SDL_CONTROLLERDEVICEREMOVED:
        {
            if (e.cdevice.which == 0)
            {
                SDL_GameControllerClose(0);
                m_gameController = nullptr;
                m_joystick       = nullptr;
            }
            break;
        }

        default: break;
    }

    return false;
}

float GameController::ConvertAxisValue(short rawValue)
{
    if (std::abs(rawValue) < THRESHOLD) return 0.f;
    return std::min(rawValue / 32767.f, 1.f);
}
