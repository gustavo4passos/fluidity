#include "renderer/texture.hpp"
#include <cassert>

Texture::Texture(const void* data, int width, int height, int nChannels, TextureFilteringMode filteringMode) :
    mWidth(width),
    mHeight(height),
    mNChannels(nChannels),
    mTextureId(0),
    mFilteringMode(filteringMode)
{  
    assert(nChannels >= 3 && nChannels <= 4);
    
    glGenTextures(1, &mTextureId);
    glBindTexture(GL_TEXTURE_2D, mTextureId);

    UploadTexture((BYTE*)data);

    SetTexFiltering(mFilteringMode, mFilteringMode);
    SetTexRepeatBehavior(true);
}

Texture::Texture(const Texture& t)
    : mTextureId(t.mTextureId), 
    mWidth(t.mWidth),
    mHeight(t.mHeight),
    mNChannels(t.mNChannels),
    mFilteringMode(t.mFilteringMode)     
{ }

void Texture::Bind() {
    glBindTexture(GL_TEXTURE_2D, mTextureId);
}

void Texture::Unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::CleanUp()
{
    glDeleteTextures(1, &mTextureId);
}

void Texture::SetFilteringMode(TextureFilteringMode mode)
{
    if(mode != mFilteringMode)
    {
        mFilteringMode = mode;
        SetTexFiltering(mFilteringMode, mFilteringMode);
    }
}


void Texture::SetTexFiltering(TextureFilteringMode magLinearFiltering, TextureFilteringMode minLinearFiltering) {
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, magLinearFiltering == TextureFilteringMode::Linear ? GL_LINEAR : GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, minLinearFiltering == TextureFilteringMode::Linear ? GL_LINEAR : GL_NEAREST);
}

void Texture::SetTexRepeatBehavior(bool repeat){
    Bind();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat ? GL_REPEAT : GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat ? GL_REPEAT : GL_CLAMP_TO_BORDER);
}

void Texture::UploadTexture(BYTE* data) {
    Bind();

    GLenum format;
    switch(mNChannels) {
        case 3:
            format = GL_RGB;
            break;
        case 4:
            format = GL_RGBA;
            break;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, mWidth, mHeight, 0, format, GL_UNSIGNED_BYTE, (const void*)data);
}
