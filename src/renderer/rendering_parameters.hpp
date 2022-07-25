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

struct FluidParameters
{
  float attenuation;
  bool  transparentFluid;
  float pointRadius;
};

struct LightingParameters
{
  float minShadowBias;
  float maxShadowBias;
  float shadowIntensity;
  bool  usePcf;
  bool  renderShadows;
};

}