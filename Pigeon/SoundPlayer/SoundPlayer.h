#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <iostream>

class SoundPlayer {
public:
    SoundPlayer();
    ~SoundPlayer();

    void play(const std::string& filename);

private:
    bool initialized = false;  // Static member to track whether SDL Mixer has been initialized
    Mix_Music* m_sound = nullptr;
};
