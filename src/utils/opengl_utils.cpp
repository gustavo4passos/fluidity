#include "opengl_utils.hpp"
#include <iostream>
#include <vector>

namespace fluidity
{

void PrintCurrentColorFramebuffer(int bufferWidth, int bufferHeight, GLenum format, GLenum type)
{
  std::vector<float> pixels;
  pixels.resize(bufferWidth * bufferHeight);
  glReadPixels(0, 0, bufferWidth, bufferHeight, format, type, &pixels[0]);
  constexpr float minusInfinity = -std::numeric_limits<float>::infinity();
  for (int i = 0; i < bufferWidth * bufferHeight; i++)
  {
    // if (i % bufferWidth == 0) std::cout << "\n";
    if (pixels[i] == minusInfinity) continue;
    if (pixels[i] == 0) continue;
    std::cout << pixels[i] << " ";
  }
  std::cin.get();
  std::cin.ignore();
}
}
