#include <iostream>
#include <stdio.h>
#include <cnpy.h>
#include "Fluid.hpp"
#include "Vec.hpp"
#include "renderer/fluid_renderer.h"
#include "renderer/window.h"
#include "utils/logger.h"


int main(int argc, char* args[])
{
    // cnpy::npz_t arr = cnpy::npz_load("C:\\dev\\npy_reading\\frame51to110\\fluid_0051.npz");

    // auto& pos = arr["pos"];
    // auto& vel = arr["vel"];

    // vec3* pos_data = pos.data<vec3>();
    // vec3* vel_data = vel.data<vec3>();
    
    // int numParticles = GetNumOfParticles(pos.num_bytes());
    // std::cout << "Number of particles: " << numParticles << std::endl;
    // std::cout << (pos.num_bytes() / 4) / 3 << "size: " << pos.shape[0] << std::endl;
    
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

    renderer = new fluidity::FluidRenderer(WINDOW_WIDTH, WINDOW_HEIGHT, 6.f);

    if(!renderer->Init())
    {
        LOG_ERROR("Unable to initialized fluid surfaces.");
        return 0;
    }

    Fluid f;
    f.Load("C:\\dev\\npy_reading\\frame51to110", "fluid_", 0, 110);

    bool running = true;
    int currentFrame = 0;

    while(running) 
    {
        SDL_Event e;
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
                   
                    default: break;
                }
            }
        }

        renderer->SetClearColor(.3f, .3f, .5f, 1.f);
        renderer->Clear();

        // // ps.Update();

        renderer->SetNumberOfParticles(f.GetNumberOfParticles(currentFrame));
        renderer->SetVAO(f.GetFrameVao(currentFrame));
        renderer->Render();

        currentFrame = (currentFrame + 1) % f.GetNumberOfFrames();

        window.Swap();
    }

 

    // for (int i = 0; i < f.GetNumberOfParticles(); i++)
    // {
    //     printf("(%f, %f, %f)\n", f.GetFramePosData(0)[i].x, f.GetFramePosData(0)[i].y, f.GetFramePosData(0)[i].z);
    // }

    return 0;
}