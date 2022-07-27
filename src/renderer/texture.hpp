#ifndef TEXTURE_H
#define TEXTURE_H

#include <string>
#include <GL/glew.h>

enum class TextureFilteringMode
{
    Linear,
    Nearest,
};

class Texture {
public:
    Texture(const void* data, int width, int height, int nChannels, TextureFilteringMode filteringMode = TextureFilteringMode::Nearest);
    Texture(const Texture&);

    void Bind();
    void Unbind();
    void CleanUp();

    inline int GetWidth() const { return mWidth; }
    inline int GetHeight() const { return mHeight; }

    void SetFilteringMode(TextureFilteringMode mode);
    TextureFilteringMode GetFilteringMode() { return mFilteringMode; }

	inline GLuint GetID() const { return mTextureId; }
private:
    typedef unsigned char BYTE;

    // Texture information
    int mWidth, mHeight, mNChannels;
    TextureFilteringMode mFilteringMode;

    // Texture buffer OpenGL name
    GLuint mTextureId;

    // Set mag and min filtering
    void SetTexFiltering(TextureFilteringMode magLinearFiltering, TextureFilteringMode minLinearFiltering);

    // Specifies what happens when drawing out of the bounds 
    // of the texture 
    void SetTexRepeatBehavior(bool repeat = true);

    // Uploads texture data to the GPU
    void UploadTexture(BYTE* data);

};

#endif
