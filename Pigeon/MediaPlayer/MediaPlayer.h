#pragma once
#include <iostream>
#include <string>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer_ext.h>

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
#include "GLEW/GL/glew.h"

class MediaPlayer
{
public:
	MediaPlayer(const std::string& filePath);
	~MediaPlayer();

public:
	void Run(bool& isDone);

	inline int& getFPS() { return this->FPS; }

	inline void setfilePath(const std::string& filePath) { this->m_filePath = filePath; }

private:
	int ProcessMedia();
	void CreateWindow();
	void RenderVideoFrame(const AVFrame* frame);
	void PlayAudioFrame(const AVFrame* audioFrame);

	int LoadMedia();
	void PlayMedia(const std::vector<AVFrame*>& frameBuffer, const std::vector<AVFrame*>& audioFrameBuffer);

private:
	std::string m_filePath = "";

	
	const static size_t MAX_BUFFER_SIZE = 100 * 1024 * 1024;
	std::vector<AVFrame*> videoBuffer;
	std::vector<AVFrame*> audioBuffer;


	const AVCodec* m_videoCodec = nullptr;
	const AVCodec* m_audioCodec = nullptr;

	AVFormatContext* m_formatContext = nullptr;
	AVCodecParameters* m_videoCodecParams = nullptr;
	AVCodecParameters* m_audioCodecParams = nullptr;
	AVCodecContext* m_videoCodecContext = nullptr;
	AVCodecContext* m_audioCodecContext = nullptr;

	AVFrame* m_currentFrame = nullptr;
	AVPacket* m_currentPacket = nullptr;

	SDL_Window* m_window = nullptr;
	SDL_Renderer* m_renderer = nullptr;
	SDL_Texture* m_texture = nullptr;

	SDL_Rect dstRect;

	int VIDEO_INDEX = -1;
	int AUDIO_INDEX = -1; //TODO: Stereo
	int FPS = 60;
};

