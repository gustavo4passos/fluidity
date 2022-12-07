#pragma once
#include "renderer/generic_fluid.hpp"
#define FLUIDITY_ENABLE_SIMULATOR 1
#if FLUIDITY_ENABLE_SIMULATOR
#include <particleSystem.h>
#include <assert.h>
#endif

struct FluidSimulationParameters
{
    int   m_iterations        = 1;
    float m_damping           = 1.f;
    float m_gravity           = 0.0003f;
    float m_ballRadius        = 11;
    float m_collideSpring     = 0.5f;
    float m_collideDamping    = 0.02f;
    float m_collideShear      = 0.1f;
    float m_collideAttraction = 0.f;
    float m_timestep          = 0.5f;
};


#if FLUIDITY_ENABLE_SIMULATOR
class SimulatedFluid  {
public: 
    explicit SimulatedFluid()
    : m_particleSystem(nullptr),
      m_paused(false) 
    { /* */ }

    bool Init(int argc = 0, char* args[] = nullptr);
    void AddSphere();
    void Update();
    void Pause() { m_paused = true;  }
    void Resume() { m_paused = false; }
    void Reset();
    
    virtual int GetNumberOfParticles() { return m_numParticles; }
    virtual GLuint GetFrameVao() { return GetParticleSystem()->getCurrentPosVao(); }

    ParticleSystem* GetParticleSystem() const
    { 
        assert(m_particleSystem != nullptr);
        return m_particleSystem;
    }

    bool HasBenInit() { return m_hasBenInit; }

public:
    FluidSimulationParameters m_simulationParameters;

private:
    const int m_numParticles = 96384;
    // const int m_numParticles = 16384;
    const uint3 m_gridSize   = { 64, 64, 64 };

    ParticleSystem* m_particleSystem;
    bool m_paused;
    bool m_hasBenInit = false;
};

#endif // If option FLUDITY_ENABLE_SIMULATOR is enabled (end)
