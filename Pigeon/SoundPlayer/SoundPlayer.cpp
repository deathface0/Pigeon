#include "SoundPlayer.h"

SoundPlayer::SoundPlayer() {
    // Inicializar SDL
    if (SDL_Init(SDL_INIT_AUDIO) != 0) {
        std::cout << "Error al inicializar SDL: " << SDL_GetError() << std::endl;
        exit(1);
    }

    // Inicializar SDL_mixer
    if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 1024) < 0) {
        std::cout << "Error al inicializar SDL_mixer: " << Mix_GetError() << std::endl;
        exit(1);
    }
}

SoundPlayer::~SoundPlayer()
{
    // Limpiar SDL_mixer
    Mix_Quit();

    // Limpiar SDL
    SDL_Quit();
}

void SoundPlayer::play(const std::string& filename) {
    Mix_Music* music = Mix_LoadMUS(filename.c_str());
    if (!music) {
        std::cout << "Error al cargar el archivo de audio: " << Mix_GetError() << std::endl;
        return;
    }

    std::cout << "Archivo de audio cargado correctamente" << std::endl;

    // Reproducir el audio
    if (Mix_PlayMusic(music, 1) == -1) {
        std::cout << "Error al reproducir el archivo de audio: " << Mix_GetError() << std::endl;
        Mix_FreeMusic(music);
        return;
    }

    std::cout << "START PLAY" << std::endl;

    // Esperar hasta que el audio termine de reproducirse o haya pasado 5 segundos
    int timeout = 5000; // 5 segundos
    while (Mix_PlayingMusic() && timeout > 0) {
        SDL_Delay(100);
        timeout -= 100;
    }

    std::cout << "END PLAY" << std::endl;

    Mix_FreeMusic(music);
}