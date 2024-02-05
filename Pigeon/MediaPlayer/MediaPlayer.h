#pragma once
#include <iostream>
#include <string>
#include <SDL2/SDL.h>

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

class MediaPlayer
{
private:
	std::string filePath;

	const AVCodec* codec = nullptr;
	AVCodecParameters* codecParams = nullptr;
	AVCodecContext* codecContext = nullptr;

	AVFrame* currentFrame = nullptr;

	int VIDEO_INDEX = -1;

public:
	MediaPlayer(const std::string& filePath);

};

