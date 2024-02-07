#include "MediaPlayer.h"
#include <thread>

MediaPlayer::MediaPlayer(const std::string& filePath) : m_filePath(filePath), dstRect()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    m_currentFrame = av_frame_alloc();
    m_currentPacket = av_packet_alloc();
}

MediaPlayer::~MediaPlayer()
{
    if (m_formatContext != nullptr) {
        avformat_free_context(m_formatContext);
        m_formatContext = nullptr;
    }

    if (m_codecContext != nullptr) {
        avcodec_free_context(&m_codecContext);
        m_codecContext = nullptr;
    }

    if (m_currentFrame != nullptr) {
        av_frame_free(&m_currentFrame);
        m_currentFrame = nullptr;
    }

    if (m_currentPacket != nullptr) {
        av_packet_free(&m_currentPacket);
        m_currentPacket = nullptr;
    }

    if (m_texture != nullptr) {
        SDL_DestroyTexture(m_texture);
        m_texture = nullptr;
    }

    if (m_renderer != nullptr) {
        SDL_DestroyRenderer(m_renderer);
        m_renderer = nullptr;
    }

    if (m_window != nullptr) {
        SDL_DestroyWindow(m_window);
        m_window = nullptr;
    }

    std::cout << "MediaPlayer was deleted" << std::endl;
}

int MediaPlayer::ProcessMedia()
{
    if (avformat_open_input(&m_formatContext, m_filePath.c_str(), nullptr, nullptr) != 0) {
        return -1;
    }

    if (avformat_find_stream_info(m_formatContext, nullptr) < 0) {
        return -1;
    }

    for (size_t i = 0; i < m_formatContext->nb_streams; i++) {
        if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            VIDEO_INDEX = i;
            m_codecParams = m_formatContext->streams[i]->codecpar;
            m_codec = avcodec_find_decoder(m_codecParams->codec_id);
            break;
        }
    }

    m_codecContext = avcodec_alloc_context3(m_codec);

    if (avcodec_parameters_to_context(m_codecContext, m_codecParams) < 0) {
        return -1;
    }

    if (!m_codecContext) {
        return -1;
    }

    if (avcodec_open2(m_codecContext, m_codec, nullptr) < 0) {
        return -1;
    }

    return 1;
}

void MediaPlayer::CreateWindow()
{
    m_window = SDL_CreateWindow(m_filePath.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_codecContext->width, m_codecContext->height, SDL_WINDOW_SHOWN);
    m_renderer = SDL_CreateRenderer(m_window, 1, 0);
    m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, m_codecContext->width, m_codecContext->height);
}

void MediaPlayer::RenderFrame(const AVFrame* frame)
{
    dstRect.x = 0;
    dstRect.y = 0;
    dstRect.w = m_codecContext->width;
    dstRect.h = m_codecContext->height;

    SDL_UpdateYUVTexture(m_texture, &dstRect, frame->data[0], frame->linesize[0], frame->data[1], frame->linesize[1], frame->data[2], frame->linesize[2]);

    SDL_RenderClear(m_renderer);
    SDL_RenderCopy(m_renderer, m_texture, nullptr, &dstRect);
    SDL_RenderPresent(m_renderer);

    SDL_GL_SwapWindow(m_window);
}

int MediaPlayer::LoadMedia()
{
    bool quitVideo = false;

    while (av_read_frame(m_formatContext, m_currentPacket) >= 0) {
        if (m_currentPacket->stream_index == VIDEO_INDEX) {
            int ret = avcodec_send_packet(m_codecContext, m_currentPacket);
            if (ret < 0) 
                break;

            while (ret >= 0)
            {
                if (frameBuffer.size() >= MAX_BUFFER_SIZE) {
                    std::cout << "MAX_BUFFER_SIZE reached, clearing buffer" << std::endl;
                    PlayMedia(frameBuffer);
                    frameBuffer.clear();
                }

                ret = avcodec_receive_frame(m_codecContext, m_currentFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    break;

                AVFrame* frame = av_frame_clone(m_currentFrame);
                frameBuffer.push_back(frame);
            }
        }
        av_packet_unref(m_currentPacket);
    }
    std::cout << "loaded " << frameBuffer.size() * sizeof(AVFrame*) << std::endl;

    PlayMedia(frameBuffer);
    frameBuffer.clear();

    delete this;

    return 1;
}

void MediaPlayer::PlayMedia(const std::vector<AVFrame*>& frameBuffer)
{
    bool quitVideo = false;

    SDL_GLContext gl_context2 = SDL_GL_CreateContext(m_window);

    for (const AVFrame* frame : frameBuffer) {
        if (quitVideo)
            break;

        RenderFrame(frame);

        SDL_Delay(1000 / FPS);

        SDL_Event event;
        while (SDL_PollEvent(&event) > 0) {
            switch (event.type) {
            case SDL_QUIT:
                return;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                    return;
                break;
            }
        }
    }
}

void MediaPlayer::Run(bool& isDone)
{
        std::thread([&] {
            if (ProcessMedia()) {
                CreateWindow();
                LoadMedia();
                //PlayMedia();
                isDone = true;
            }
        }).detach(); 
}