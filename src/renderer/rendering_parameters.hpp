#pragma once

namespace fluidity
{

struct FilteringParameters
{
  int nIterations;
  int filterSize;
  int maxFilterSize;
  bool gammaCorrection; // TODO: This should not be here. More like "post processing parameters"
  bool useRefractionMask;
};

struct FluidParameters
{
  float attenuation;
  bool  transparentFluid;
  float pointRadius;
  float refractionModifier = 1.0;
};

struct LightingParameters
{
  float minShadowBias;
  float maxShadowBias;
  float shadowIntensity;
  bool  usePcf;
  bool  renderShadows;
  bool  showLightsOnScene;
};

}