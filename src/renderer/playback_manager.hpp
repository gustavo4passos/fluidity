#pragma once

class PlaybackManager
{
public:
  PlaybackManager() = default;
  PlaybackManager(int nFrames) 
    : m_nFrames(nFrames)
  { /* */ } 

  void AdvanceFrame();

  void Play();
  void Pause();
  void Stop();

  bool IsPlaying() const { return m_playing; }
  void SetIsPlaying(bool playing) { m_playing = playing; }

private:
  int m_currentFrame = 0;
  int m_nFrames;
  bool m_playing;
};
