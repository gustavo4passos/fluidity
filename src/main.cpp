#include <iostream>
#include <stdio.h>
#include <cnpy.h>
#include "Fluid.hpp"
#include "Vec.hpp"
#include "renderer/fluid_renderer.h"
#include "renderer/window.h"
#include "utils/logger.h"

struct CommandLineArgs 
{
    std::string npzPath;
    int frameCount;
    int firstFrame;
};

CommandLineArgs parseCommandArgs(int argc, char* args[])
{
    CommandLineArgs output = { "", 0, 0 };

    if (argc > 1) output.npzPath = args[1];
    if (argc > 2) output.frameCount = std::stoi(args[2]);
    if (argc > 3) output.firstFrame = std::stoi(args[3]);

    return output;
}

void printUsage()
{
    std::cout << "Usage: $ npz-rendering npz_path frame_count [first_frame]\n";
}

int validateCommandLineArguments(const CommandLineArgs& cmdArgs)
{
    if (cmdArgs.npzPath.empty())
    {
        std::cerr << "Error: Missing npz folder.\n";
        printUsage();
        return 1;
    }
    if (cmdArgs.frameCount == 0)
    {
        std::cout << "Error: frame count missing or 0.\n";
        printUsage();
        return 2;
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
    fluidity::FluidRenderer* renderer;

    if(!window.Init()) 
    {
        LOG_ERROR("Unable to create window..");
        return 0;
    }
    else LOG_WARNING("Window successfully created.");

    renderer = new fluidity::FluidRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 0.06250);

    if(!renderer->Init())
    {
        LOG_ERROR("Unable to initialized fluid surfaces.");
        return 0;
    }

    Fluid f;
    f.Load(cmdLineArgs.npzPath, "fluid_", cmdLineArgs.firstFrame, cmdLineArgs.frameCount);

    bool running = true;
    bool playing = true;
    int currentFrame = 0;

    while(running) 
    {
        SDL_Event e;
        vec3 movement = { 0.f, 0.f, 0.f };

        while(SDL_PollEvent(&e)) 
        {
            if(e.type == SDL_QUIT) running = false;
            if(e.type == SDL_KEYUP)
            {
                switch(e.key.keysym.sym)
                {
                    case SDLK_ESCAPE:
                    {
                        running = false;
                    } break;

                    case SDLK_f: 
                    {
                        renderer->SetFiltering(!renderer->GetFiltering());
                    } break;

                    case SDLK_SPACE:
                    {
                        playing = !playing;
                    } break;

                    // Camera movement
                    case SDLK_UP:
                    {
                      currentCameraPosition.z = 1;
                    }
                    default: break;
                }
            }
        }

        renderer->SetCameraPosition(currentCameraPosition);
        renderer->SetClearColor(.3f, .3f, .5f, 1.f);
        renderer->Clear();

        renderer->SetNumberOfParticles(f.GetNumberOfParticles(currentFrame));
        renderer->SetVAO(f.GetFrameVao(currentFrame));
        renderer->Render();

        if (playing) currentFrame = (currentFrame + 1) % f.GetNumberOfFrames();

        window.Swap();
    }

    return 0;
}
