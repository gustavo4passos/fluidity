#pragma once

namespace fluidity
{

struct FilteringParameters
{
  int nIterations;
  int filterSize;
  int maxFilterSize;
  bool gammaCorrection; // TODO: This should not be here. More like "post processing parameters"
};

struct FluidRenderingParameters
{
  float attenuation;
};

struct ShadowMapParameters
{
  float minShadowBias;
  float maxShadowBias;
  float shadowIntensity;
  bool  usePcf;
};

}