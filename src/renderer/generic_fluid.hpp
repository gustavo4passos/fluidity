#pragma once
#include <GL/glew.h>

enum class FluidType
{
  SPHSimulation,
  NPZPreSimulated
};

class GenericFluid
{
public:

  virtual void CleanUp() = 0;

  virtual int GetNumberOfParticles(int frame) = 0;
  virtual int GetNumberOfFrames() const = 0;

  virtual GLuint GetFrameVao(int frame) = 0;
  virtual FluidType GetFluidType() = 0;
};
