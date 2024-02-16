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

    if (m_videoCodecContext != nullptr) {
        avcodec_free_context(&m_videoCodecContext);
        m_videoCodecContext = nullptr;
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
            m_videoCodecParams = m_formatContext->streams[i]->codecpar;
            m_videoCodec = avcodec_find_decoder(m_videoCodecParams->codec_id);
        }
        else if (m_formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            AUDIO_INDEX = i;
            m_audioCodecParams = m_formatContext->streams[i]->codecpar;
            m_audioCodec = avcodec_find_decoder(m_audioCodecParams->codec_id);
        }
    }

    m_videoCodecContext = avcodec_alloc_context3(m_videoCodec);
    m_audioCodecContext = avcodec_alloc_context3(m_audioCodec);

    if (avcodec_parameters_to_context(m_videoCodecContext, m_videoCodecParams) < 0) {
        return -1;
    }

    if (avcodec_parameters_to_context(m_audioCodecContext, m_audioCodecParams) < 0) {
        return -1;
    }

    if (!m_videoCodecContext) {
        return -1;
    }

    if (!m_audioCodecContext) {
        return -1;
    }

    if (avcodec_open2(m_videoCodecContext, m_videoCodec, nullptr) < 0) {
        return -1;
    }

    if (avcodec_open2(m_audioCodecContext, m_audioCodec, nullptr) < 0) {
        return -1;
    }

    return 1;
}

void MediaPlayer::CreateWindow()
{
    m_window = SDL_CreateWindow(m_filePath.c_str(), SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, m_videoCodecContext->width, m_videoCodecContext->height, SDL_WINDOW_SHOWN);
    m_renderer = SDL_CreateRenderer(m_window, 1, 0);
    m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STREAMING, m_videoCodecContext->width, m_videoCodecContext->height);
}

void MediaPlayer::RenderVideoFrame(const AVFrame* frame)
{
    dstRect.x = 0;
    dstRect.y = 0;
    dstRect.w = m_videoCodecContext->width;
    dstRect.h = m_videoCodecContext->height;

    SDL_UpdateYUVTexture(m_texture, &dstRect, frame->data[0], frame->linesize[0], frame->data[1], frame->linesize[1], frame->data[2], frame->linesize[2]);

    SDL_RenderClear(m_renderer);
    SDL_RenderCopy(m_renderer, m_texture, nullptr, &dstRect);
    SDL_RenderPresent(m_renderer);

    SDL_GL_SwapWindow(m_window);
}

void MediaPlayer::PlayAudioFrame(const AVFrame* audioFrame)
{
    // Verificar si el frame de audio es válido
    if (!audioFrame || !audioFrame->data[0]) {
        std::cerr << "Error: Frame de audio no válido." << std::endl;
        return;
    }

    // Comprueba si SDL_mixer está inicializado
    if (Mix_OpenAudio(44100, AUDIO_S16SYS, 1, 4096) < 0) {
        std::cerr << "No se pudo inicializar SDL_mixer: " << Mix_GetError() << std::endl;
        return;
    }

    // Crear un chunk de SDL_mixer con los datos de audio del frame
    Mix_Chunk* chunk = Mix_QuickLoad_RAW((Uint8*)audioFrame->data[0], audioFrame->nb_samples * av_get_bytes_per_sample((AVSampleFormat)audioFrame->format));

    // Reproducir el chunk de audio
    Mix_PlayChannel(-1, chunk, 0);

    // Esperar hasta que se haya terminado de reproducir el chunk
    while (Mix_Playing(-1) != 0) {
        SDL_Delay(100);
    }

    // Liberar el chunk de memoria
    Mix_FreeChunk(chunk);

    // Cerrar SDL_mixer después de la reproducción
    Mix_CloseAudio();
}

int MediaPlayer::LoadMedia()
{
    while (av_read_frame(m_formatContext, m_currentPacket) >= 0) {
        if (m_currentPacket->stream_index == VIDEO_INDEX) {
            int ret = avcodec_send_packet(m_videoCodecContext, m_currentPacket);
            if (ret < 0) 
                break;

            while (ret >= 0)
            {
                if (videoBuffer.size() >= MAX_BUFFER_SIZE) {
                    std::cout << "MAX_BUFFER_SIZE reached for video, clearing buffer" << std::endl;
                    PlayMedia(videoBuffer, audioBuffer);
                    videoBuffer.clear();
                    audioBuffer.clear();
                }

                ret = avcodec_receive_frame(m_videoCodecContext, m_currentFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    break;

                AVFrame* frame = av_frame_clone(m_currentFrame);
                videoBuffer.push_back(frame);
            }
        }
        else if (m_currentPacket->stream_index == AUDIO_INDEX) {
            int ret = avcodec_send_packet(m_audioCodecContext, m_currentPacket);
            if (ret < 0)
                break;

            while (ret >= 0)
            {
                if (audioBuffer.size() >= MAX_BUFFER_SIZE) {
                    std::cout << "MAX_BUFFER_SIZE reached for audio, clearing buffer" << std::endl;
                    PlayMedia(videoBuffer, audioBuffer);
                    videoBuffer.clear();
                    audioBuffer.clear();
                }

                ret = avcodec_receive_frame(m_audioCodecContext, m_currentFrame);
                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    break;

                AVFrame* frame = av_frame_clone(m_currentFrame);
                audioBuffer.push_back(frame);
            }
        }
        av_packet_unref(m_currentPacket);
    }
    std::cout << "loaded video " << videoBuffer.size() << std::endl;
    std::cout << "loaded audio " << audioBuffer.size() << std::endl;

    PlayMedia(videoBuffer, audioBuffer);
    videoBuffer.clear();
    audioBuffer.clear();

    delete this;

    return 1;
}

void MediaPlayer::PlayMedia(const std::vector<AVFrame*>& videoFrameBuffer, const std::vector<AVFrame*>& audioFrameBuffer)
{
    bool quitVideo = false;

    SDL_GLContext gl_context2 = SDL_GL_CreateContext(m_window);

    size_t videoIndex = 0;
    size_t audioIndex = 0;

    uint32_t startTime = SDL_GetTicks();
    uint32_t lastVideoTime = 0;
    uint32_t lastAudioTime = 0;

    while (!quitVideo && (videoIndex < videoFrameBuffer.size() || audioIndex < audioFrameBuffer.size())) {
        uint32_t currentTime = SDL_GetTicks() - startTime;

        // Reproducir video si es el momento adecuado
        if (videoIndex < videoFrameBuffer.size() && currentTime >= lastVideoTime) {
            RenderVideoFrame(videoFrameBuffer[videoIndex]);
            videoIndex++;
            lastVideoTime += 1000 / FPS; // Avanzar al siguiente fotograma de video
        }

        // Reproducir audio si es el momento adecuado
        if (audioIndex < audioFrameBuffer.size() && currentTime >= lastAudioTime) {
            PlayAudioFrame(audioFrameBuffer[audioIndex]);
            audioIndex++;
            lastAudioTime += (audioFrameBuffer[audioIndex]->nb_samples / m_audioCodecContext->sample_rate) * 1000; // Avanzar al siguiente fotograma de audio
        }

        // Esperar un breve tiempo para mantener la tasa de fotogramas deseada
        SDL_Delay(1);

        // Manejar eventos de SDL, como cerrar la ventana
        SDL_Event event;
        while (SDL_PollEvent(&event) > 0) {
            switch (event.type) {
            case SDL_QUIT:
                quitVideo = true;
                break;
            case SDL_WINDOWEVENT:
                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
                    quitVideo = true;
                break;
            }
        }
    }
}


//void MediaPlayer::PlayMedia(const std::vector<AVFrame*>& frameBuffer)
//{
//    bool quitVideo = false;
//
//    SDL_GLContext gl_context2 = SDL_GL_CreateContext(m_window);
//
//    for (const AVFrame* frame : frameBuffer) {
//        if (quitVideo)
//            break;
//
//        RenderFrame(frame);
//
//        SDL_Delay(1000 / FPS);
//
//        SDL_Event event;
//        while (SDL_PollEvent(&event) > 0) {
//            switch (event.type) {
//            case SDL_QUIT:
//                return;
//            case SDL_WINDOWEVENT:
//                if (event.window.event == SDL_WINDOWEVENT_CLOSE)
//                    return;
//                break;
//            }
//        }
//    }
//}

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