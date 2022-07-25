#include <iostream>
#include <stdio.h>
#include <cnpy.h>
#include "Fluid.hpp"
#include "renderer/fluid_renderer.hpp"
#include "renderer/window.h"
#include "utils/logger.h"
#include "utils/gui_layer.hpp"

struct CommandLineArgs 
{
    std::string scenePath;
    std::string npzPath;
    int frameCount;
    int firstFrame;
};

CommandLineArgs parseCommandArgs(int argc, char* args[])
{
    CommandLineArgs output = { "", "", 0, 0 };

    if (argc > 1) output.scenePath = args[1];
    if (argc > 2) output.npzPath = args[2];
    if (argc > 3) output.frameCount = std::stoi(args[3]);
    if (argc > 4) output.firstFrame = std::stoi(args[4]);

    return output;
}

void printUsage()
{
    std::cout << "Usage: $ npz-rendering scene_path npz_path frame_count [first_frame]\n";
}

int validateCommandLineArguments(const CommandLineArgs& cmdArgs)
{
    if (cmdArgs.scenePath.empty())
    {
        std::cerr << "Error: Missing scene path.\n";
        printUsage();
        return 1;
    }
    if (cmdArgs.npzPath.empty())
    {
        std::cerr << "Error: Missing npz folder.\n";
        printUsage();
        return 2;
    }
    if (cmdArgs.frameCount == 0)
    {
        std::cout << "Error: frame count missing or 0.\n";
        printUsage();
        return 3;
    }

    return 0;
}

int main(int argc, char* args[])
{
    auto cmdLineArgs = parseCommandArgs(argc, args);
    auto validationStatus = validateCommandLineArguments(cmdLineArgs);
    if (validationStatus != 0) return validationStatus;

    const unsigned int WINDOW_WIDTH = 1366;
    const unsigned int WINDOW_HEIGHT = 768;

    Window window = Window("Fluidity", WINDOW_WIDTH, WINDOW_HEIGHT, 4, 5, true, false);

    if(!window.Init()) 
    {
        LOG_ERROR("Unable to create window..");
        return 0;
    }
    else LOG_WARNING("Window successfully created.");

    fluidity::FluidRenderer* renderer;
    renderer = new fluidity::FluidRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0.06250);

    fluidity::SceneSerializer ss(cmdLineArgs.scenePath);
    ss.Deserialize();
    std::cout << ss.GetScene().skyboxPath;
    renderer->SetScene(ss.GetScene());

    if(!renderer->Init())
    {
        LOG_ERROR("Unable to initialize fluid renderer.");
        return 0;
    }

    Fluid f;
    f.Load(cmdLineArgs.npzPath, "fluid_", cmdLineArgs.firstFrame, cmdLineArgs.frameCount);

    bool running = true;
    bool playing = true;
    bool showGui = true;
    int currentFrame = 0;

    SDL_CaptureMouse(SDL_TRUE);
    fluidity::GuiLayer gui = fluidity::GuiLayer(window.GetSDLWindow(), window.GetSDLGLContext(), renderer);
    gui.Init();

    while(running) 
    {
        SDL_Event e;

        while(SDL_PollEvent(&e)) 
        {
            if(e.type == SDL_QUIT) running = false;
            
            if (gui.ProcessEvent(e)) continue;

            if(e.type == SDL_KEYUP)
            {
                switch(e.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                    {
                        running = false;
                    } break;

                    case SDLK_SPACE:
                    {
                        playing = !playing;
                    } break;

                    case SDLK_i:
                    {
                        showGui = !showGui;
                        break;
                    }

                    default: break;
                }
            }
            renderer->ProcessInput(e);
        }

        renderer->SetNumberOfParticles(f.GetNumberOfParticles(currentFrame));
        renderer->SetVAO(f.GetFrameVao(currentFrame));
        renderer->Update();
        renderer->Render();
        if (showGui) gui.Render();
        if (playing) currentFrame = (currentFrame + 1) % f.GetNumberOfFrames();

        window.Swap();
    }

    return 0;
}
