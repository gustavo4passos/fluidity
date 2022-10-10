#pragma once

namespace fluidity
{

struct FilteringParameters
{
  int nIterations;
  int filterSize;
  int maxFilterSize;
  bool gammaCorrection; // TODO: This should not be here. More like "post processing parameters"
  bool useRefractionMask = true;
  bool filter1D          = false;
};

struct FluidParameters
{
  float attenuation;
  bool  transparentFluid;
  float pointRadius;
  float refractionModifier = 1.0;
  bool  twoSidedRefractions = false;
  float reflectionConstant = 0.0;
};

struct LightingParameters
{
  float minShadowBias;
  float maxShadowBias;
  float shadowIntensity;
  float fluidShadowIntensity = 0.002;
  bool  usePcf;
  bool  renderShadows;
  bool  renderFluidShadows = false;
  bool  showLightsOnScene;
};

}
