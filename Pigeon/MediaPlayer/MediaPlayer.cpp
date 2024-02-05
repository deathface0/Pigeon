#include "MediaPlayer.h"

MediaPlayer::MediaPlayer(const std::string& filePath):	filePath(filePath)
{
	SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
}
