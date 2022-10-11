#include "simulated_fluid.hpp"
#include <random>

#if FLUIDITY_ENABLE_SIMULATOR
extern "C" void cudaInit(int argc, char **args);

auto SimulatedFluid::Init(int argc, char* args[]) -> bool
{
    cudaInit(argc, args);
    
    m_particleSystem = new ParticleSystem(
        m_numParticles,
        m_gridSize,
        true);
    m_particleSystem->reset(ParticleSystem::CONFIG_GRID);
    m_hasBenInit = true;

    return true;
}

auto SimulatedFluid::AddSphere() -> void
{
    int ballr = m_simulationParameters.m_ballRadius;
    float pr = m_particleSystem->getParticleRadius();
    float tr = pr + (pr * 2.f) * ballr;
    float pos[4], vel[4];
    pos[0] = -2.f + tr + (rand() / (float)RAND_MAX) * (2.f - tr * 2.f);
    pos[1] = 2.f - tr;
    pos[2] = -1.f + tr + (rand() / (float)RAND_MAX) * (2.f - tr * 2.f);
    pos[3] = 0.f;

    vel[0] = vel[1] = vel[2] = vel[3] = 0.f;

    m_particleSystem->addSphere(0, pos, vel, ballr, pr * 2.f);
}

auto SimulatedFluid::Update() -> void
{
    if(!m_paused)
    {
        m_particleSystem->setIterations(m_simulationParameters.m_iterations);
        m_particleSystem->setDamping(m_simulationParameters.m_damping);
        m_particleSystem->setGravity(-m_simulationParameters.m_gravity);
        m_particleSystem->setCollideSpring(m_simulationParameters.m_collideSpring);
        m_particleSystem->setCollideDamping(m_simulationParameters.m_collideDamping);
        m_particleSystem->setCollideShear(m_simulationParameters.m_collideShear);
        m_particleSystem->setCollideAttraction(m_simulationParameters.m_collideAttraction);

        m_particleSystem->update(m_simulationParameters.m_timestep);
    }
}

auto SimulatedFluid::Reset() -> void
{
  GetParticleSystem()->reset(ParticleSystem::CONFIG_GRID);
}
#endif
