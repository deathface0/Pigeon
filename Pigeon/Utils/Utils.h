#pragma once

#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_opengl3.h>
#include <GLEW/GL/glew.h>

#include "stb_image/stb_image.h"

#include <iostream>
#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>
#include <iomanip>
#include <functional>

namespace File {
    static bool BufferToDisk(const std::vector<unsigned char>& buffer, const std::string& filename) {
        std::ofstream outfile(filename, std::ios::out | std::ios::binary);

        if (outfile.is_open()) {
            outfile.write(reinterpret_cast<const char*>(buffer.data()), buffer.size());
            outfile.close();
        }
        else {
            return false;
        }
        return true;
    }

    static std::vector<unsigned char> DiskToBuffer(const std::string& filename) {
        std::vector<unsigned char> buffer;
        std::ifstream infile(filename, std::ios::in | std::ios::binary);

        if (infile.is_open()) {
            infile.seekg(0, std::ios::end);
            std::streampos fileSize = infile.tellg();
            infile.seekg(0, std::ios::beg);

            buffer.resize(fileSize);

            infile.read(reinterpret_cast<char*>(buffer.data()), fileSize);

            infile.close();
        }

        return buffer;
    }

    static std::string GetFileExtension(const std::string& filename) {
        size_t dotIndex = filename.find_last_of(".");
        if (dotIndex != std::string::npos) {
            return filename.substr(dotIndex + 1);
        }
        return ""; 
    }

    static std::string GetFilenameFromPath(const std::string& filepath) {
        //This handles both unix and windows like filepath styles
        size_t lastSlashIndex = filepath.find_last_of("/\\"); 
        if (lastSlashIndex != std::string::npos) {
            return filepath.substr(lastSlashIndex + 1);
        }
        return "";
    }

    static unsigned char* loadImage(const char* filename, int* width, int* height, int* channels) {
        return stbi_load(filename, width, height, channels, STBI_rgb_alpha);
    }
}

namespace String {
    static std::vector<unsigned char> StringToBytes(const std::string& str) {
        return std::vector<unsigned char>(str.begin(), str.end());
    }
    static void printBytesInHex(const std::vector<unsigned char>& bytes) {
        for (const unsigned char& byte : bytes) {
            std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte) << " " << std::dec;
        }
        std::cout << std::endl;
    }
    static std::string BytesToString(const std::vector<unsigned char>& buf) {
        return std::string(buf.begin(), buf.end());
    }

  
}

/* Not my implementation */
namespace B64 {
    static std::string base64_encode(const std::string& in) {

        std::string out;

        int val = 0, valb = -6;
        for (unsigned char c : in) {
            val = (val << 8) + c;
            valb += 8;
            while (valb >= 0) {
                out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[(val >> valb) & 0x3F]);
                valb -= 6;
            }
        }
        if (valb > -6) out.push_back("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[((val << 8) >> (valb + 8)) & 0x3F]);
        while (out.size() % 4) out.push_back('=');
        return out;
    }

    static std::string base64_decode(const std::string& in) {

        std::string out;

        std::vector<int> T(256, -1);
        for (int i = 0; i < 64; i++) T["ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/"[i]] = i;

        int val = 0, valb = -8;
        for (unsigned char c : in) {
            if (T[c] == -1) break;
            val = (val << 6) + T[c];
            valb += 6;
            if (valb >= 0) {
                out.push_back(char((val >> valb) & 0xFF));
                valb -= 8;
            }
        }
        return out;
    }
}


namespace GUIUtils
{
    inline void TextCentered(const std::string text) {
        auto windowWidth = ImGui::GetWindowSize().x;
        auto textWidth = ImGui::CalcTextSize(text.c_str()).x;

        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::Text(text.c_str());
    }

    inline void InputCentered(const char* label, char* buf, int buf_size , float width, ImGuiInputTextFlags flags)
    {
        auto windowWidth = ImGui::GetWindowSize().x;

        ImGui::SetCursorPosX((windowWidth - width) * 0.5f);
        ImGui::PushItemWidth(width);
        ImGui::InputText(label, buf, buf_size, flags);
    }

    inline void ImageCentered(const char* imagePath, GLuint& texture, int width, int height, float aspectRatio)
    {
        int channels;
        unsigned char* my_image_data = stbi_load(imagePath, &width, &height, &channels, 4);
        assert(my_image_data != NULL);

        // Turn the RGBA pixel data into an OpenGL texture:
        glGenTextures(1, &texture);
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, my_image_data);

        auto windowWidth = ImGui::GetWindowSize().x;
        
        ImGui::SetCursorPosX((windowWidth - width * aspectRatio) * 0.5f);
        ImGui::Image((void*)(intptr_t)texture, ImVec2(width * aspectRatio, height * aspectRatio));
    }

    inline bool ButtonCentered(const char* label, float width, float height)
    {
        auto windowWidth = ImGui::GetWindowSize().x;

        ImGui::SetCursorPosX((windowWidth - width) * 0.5f);
        return ImGui::Button(label, ImVec2{ width, height });
    }
}