#pragma once
#include <iostream>
#include <string>
#include <SDL.h>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/imgutils.h>
#include <libswscale/swscale.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
}


#ifdef _WIN32
	#pragma comment(lib, "ws2_32.lib")
	#pragma comment(lib, "Secur32.lib")
	#pragma comment(lib, "Bcrypt.lib")
	#pragma comment(lib, "Mfuuid.lib")
	#pragma comment(lib, "Mfplat.lib")
	#pragma comment(lib, "strmiids.lib")
#endif

#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_opengl3.h>
#include "GL/glew.h"

class MediaPlayer
{
private:
	std::string m_filePath = "";

	const AVCodec* m_codec = nullptr;

	AVFormatContext* m_formatContext = nullptr;
	AVCodecParameters* m_codecParams = nullptr;
	AVCodecContext* m_codecContext = nullptr;

	AVFrame* m_currentFrame = nullptr;
	AVPacket* m_currentPacket = nullptr;

	SDL_Window* m_window = nullptr;
	SDL_Renderer* m_renderer = nullptr;
	SDL_Texture* m_texture = nullptr;

	SDL_Rect dstRect;

	int VIDEO_INDEX = -1;
	int FPS = 60;

public:
	int ProcessMedia();
	void CreateWindow();
	void RenderWindow();

	void PlayMedia();

	void Run(bool& isDone);

	int& getFPS() {
		return this->FPS;
	}

	void setfilePath(const std::string& filePath) {
		this->m_filePath = filePath;
	}

public:
	MediaPlayer(const std::string& filePath);
	~MediaPlayer();
};

