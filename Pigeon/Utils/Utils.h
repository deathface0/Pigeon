#pragma once
//UBUNTU
#ifdef __linux__

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_sdl2.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_stdlib.h"

#include "../stb_image/stb_image.h"


#include <GL/glew.h>

#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>

//MSVC
#else


#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_stdlib.h>
#include <GLEW/GL/glew.h>

#include "stb_image/stb_image.h"

#include <Windows.h>
#include <ShlObj.h>
#include <tchar.h>
#include <filesystem>

#endif


#include <iostream>
#include <fstream>
#include <iterator>
#include <sstream>
#include <vector>
#include <iomanip>
#include <random>
#include <sstream>

#include <SDL2/SDL.h>
#include <GLEW/GL/glew.h>


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

#ifdef WIN32
    static std::string selectFile()
    {
        OPENFILENAMEW ofn;
        wchar_t szFileName[MAX_PATH] = L"";

        ZeroMemory(&ofn, sizeof(OPENFILENAMEW));
        ofn.lStructSize = sizeof(OPENFILENAMEW);
        ofn.hwndOwner = NULL; // Ventana principal (o NULL si no es relevante)
        ofn.lpstrFilter = L"All Files (*.*)\0*.*\0"; // Filtros de archivos
        ofn.lpstrFile = szFileName; // Almacena la ruta del archivo seleccionado
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;

        // Abre el cuadro de diálogo de selección de archivos
        if (GetOpenFileNameW(&ofn) == TRUE) {
            // szFileName ahora contiene la ruta del archivo seleccionado
            // Convertir wchar_t a char
            int size_needed = WideCharToMultiByte(CP_UTF8, 0, szFileName, -1, NULL, 0, NULL, NULL);
            std::string filePath(size_needed, 0);
            WideCharToMultiByte(CP_UTF8, 0, szFileName, -1, &filePath[0], size_needed, NULL, NULL);

            std::replace_if(filePath.begin(), filePath.end(), [](char c) { return c == '\\'; }, '/');

            return filePath;
        }
        return "";
    }

    static std::string selectDirectory() {
        BROWSEINFO browseInfo;
        ZeroMemory(&browseInfo, sizeof(BROWSEINFO));

        TCHAR szDir[MAX_PATH];
        ZeroMemory(szDir, sizeof(szDir));

        browseInfo.hwndOwner = NULL; // Ventana principal (o NULL si no es relevante)
        browseInfo.pidlRoot = NULL;
        browseInfo.pszDisplayName = szDir; // Almacena la ruta del directorio seleccionado
        browseInfo.lpszTitle = _T("Seleccione un directorio");
        browseInfo.ulFlags = BIF_RETURNONLYFSDIRS;

        // Abre el cuadro de diálogo de selección de directorios
        LPITEMIDLIST pidl = SHBrowseForFolder(&browseInfo);
        if (pidl != NULL) {
            // Obtiene la ruta del directorio seleccionado
            if (SHGetPathFromIDList(pidl, szDir) == TRUE) {
                // Convierte TCHAR a std::string
                std::wstring ws(szDir);
                std::string directory(ws.begin(), ws.end());

                std::replace_if(directory.begin(), directory.end(), [](char c) { return c == '\\'; }, '/');

                return directory;
            }
            // Liberar memoria
            CoTaskMemFree(pidl);
        }
        return "";
    }

    static std::string getHomeDirectory() {
        WCHAR wszPath[MAX_PATH];
        if (SUCCEEDED(SHGetFolderPathW(NULL, CSIDL_PROFILE, NULL, 0, wszPath))) {
            char szPath[MAX_PATH];
            WideCharToMultiByte(CP_UTF8, 0, wszPath, -1, szPath, MAX_PATH, NULL, NULL);

            std::string path(szPath);
            std::replace_if(path.begin(), path.end(), [](char c) { return c == '\\'; }, '/');

            return std::string(path);
        }
        return "";
    }
#endif

#ifdef __linux__
    static std::string selectDirectory()
    {
        char filepath_cstr[1024];

        FILE* file = popen("zenity  --file-selection --directory", "r");

        fgets(filepath_cstr, 1024, file);

        std::string path(filepath_cstr);
        path.pop_back();

        return path;
    }

    static std::string selectFile()
    {
        char filepath_cstr[1024];

        FILE* file = popen("zenity --file-selection", "r");

        fgets(filepath_cstr, 1024, file);

        std::string filepath(filepath_cstr);
        filepath.pop_back();

        return filepath;
    }

    static std::string getHomeDirectory() {
        struct passwd* pw = getpwuid(getuid());
        if (pw != NULL) {
            return std::string(pw->pw_dir);
        }
        return "";
    }
#endif
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

namespace Time {
    //"%Y-%m-%d %H:%M:%S" (24h)  /  "%Y-%m-%d %I:%M:%S %p" (12h)
    static std::string timestampToDateTime(time_t timestamp, std::string params)
    {
        time_t m_timestamp = timestamp;

        std::tm time_info;
#ifdef _WIN32
        localtime_s(&time_info, &m_timestamp);
#else
        localtime_r(&m_timestamp, &time_info);
#endif
        std::stringstream ss;
        ss << std::put_time(&time_info, params.c_str());
        return ss.str();
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
    inline bool generateTexture(std::string imagePath, Texture::Image* image)
    {
        int channels;
        stbi_uc* img = stbi_load(imagePath.c_str(), &image->width, &image->height, &channels, 4);
        assert(img != NULL);

        if (img)
        {
            // Turn the RGBA pixel data into an OpenGL texture:
            glGenTextures(1, &image->texture);
            glBindTexture(GL_TEXTURE_2D, image->texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image->width, image->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, img);

            stbi_image_free(img);

            return true;
        }
        else 
        {
            return false;
        }
        
    }

    inline bool generateTexture(const std::vector<unsigned char>& buffer, Texture::Image* image)
    {
        int channels;
        stbi_uc* img = stbi_load_from_memory(buffer.data(), buffer.size(), &image->width, &image->height, &channels, 0);

        if (img) {
            // Image loaded successfully
            glGenTextures(1, &image->texture);
            glBindTexture(GL_TEXTURE_2D, image->texture);

            // Determine the format based on the number of channels
            GLenum format;
            if (channels == 1)
                format = GL_RED;
            else if (channels == 3)
                format = GL_RGB;
            else if (channels == 4)
                format = GL_RGBA;
            else {
                std::cout << "Unsupported number of channels: " << channels << std::endl;
                stbi_image_free(img);  // Corrected to free the right pointer
                return true;
            }

            glTexImage2D(GL_TEXTURE_2D, 0, format, image->width, image->height, 0, format, GL_UNSIGNED_BYTE, img);  // Corrected height parameter

            // Set texture parameters
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

            // Free the image data after usage
            stbi_image_free(img);
        }
        else {
            std::cout << "FAILED TO LOAD IMAGE FROM BUFFER: " << stbi_failure_reason() << std::endl;

            return false;
        }
    }


    inline void textureLoader(std::string path)
    {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            // Check if the entry is a file
            if (entry.is_regular_file()) {
                Texture::Image* image = new Texture::Image;
                std::string filename = entry.path().filename().string();

                generateTexture(path + filename, image);

                //Erasing file extension
                size_t pos = filename.find_last_of('.');
                filename = filename.substr(0, pos);
                
                Texture::textures.insert({ filename, image });
            }
        }
    }

    inline void fontLoader(std::string path)
    {
        for (const auto& entry : std::filesystem::directory_iterator(path)) {
            // Check if the entry is a file
            if (entry.is_regular_file()) {
                std::string filename = entry.path().filename().string();
                std::string filepath = path + filename;

                Font::Font* font = new Font::Font;
                font->px10 = ImGui::GetIO().Fonts->AddFontFromFileTTF(filepath.c_str(), 10);
                font->px20 = ImGui::GetIO().Fonts->AddFontFromFileTTF(filepath.c_str(), 20);
                font->px30 = ImGui::GetIO().Fonts->AddFontFromFileTTF(filepath.c_str(), 30);
                font->px40 = ImGui::GetIO().Fonts->AddFontFromFileTTF(filepath.c_str(), 40);
                font->px50 = ImGui::GetIO().Fonts->AddFontFromFileTTF(filepath.c_str(), 50);

                //Erasing file extension
                size_t pos = filename.find_last_of('.');
                filename = filename.substr(0, pos);

                Font::fonts.insert({ filename, font });

            }
        }
    }

    inline void TextCentered(const std::string text) {
        auto windowWidth = ImGui::GetWindowSize().x;
        auto textWidth = ImGui::CalcTextSize(text.c_str()).x;

        ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
        ImGui::Text("%s",text.c_str());
    }

    inline void InputCentered(const char* label, std::string& buf , float width, ImGuiInputTextFlags flags)
    {
        auto windowWidth = ImGui::GetWindowSize().x;

        ImGui::SetCursorPosX((windowWidth - width) * 0.5f);
        ImGui::PushItemWidth(width);
        ImGui::InputText(label, &buf, flags);
    }

    inline bool RoundButton2(std::string label, GLuint& texture, int size) {
        // Obtener el tama�o deseado para el bot�n circular
        const float buttonSize = size;

        // Calcular el centro del bot�n circular
        ImVec2 center = ImGui::GetCursorScreenPos();
        center.x += buttonSize / 2.0f;
        center.y += buttonSize / 2.0f;

        // Dibujar un c�rculo invisible para actuar como �rea de clic
        bool val = ImGui::InvisibleButton(label.c_str(), ImVec2(buttonSize, buttonSize));

        // Dibujar la imagen
        ImGui::GetWindowDrawList()->AddImage((void*)(intptr_t)texture, ImVec2(center.x - buttonSize / 2, center.y - buttonSize / 2), ImVec2(center.x + buttonSize / 2, center.y + buttonSize / 2));

        return val;
    }

    inline bool RoundButton(std::string label, GLuint& texture, int size) {
        // Obtener el tama�o deseado para el bot�n circular
        const float buttonSize = size;

        // Calcular el centro del bot�n circular
        ImVec2 center = ImGui::GetCursorScreenPos();
        center.x += buttonSize / 2.0f;
        center.y += buttonSize / 2.0f;

        // Dibujar un c�rculo invisible para actuar como �rea de clic
        bool val = ImGui::InvisibleButton(label.c_str(), ImVec2(buttonSize, buttonSize));

        // Dibujar la imagen
        ImGui::GetWindowDrawList()->AddImage((void*)(intptr_t)texture, ImVec2(center.x - buttonSize / 2, center.y - buttonSize / 2), ImVec2(center.x + buttonSize / 2, center.y + buttonSize / 2));

        return val;
    }

    inline bool ButtonCentered(const char* label, float width, float height)
    {
        auto windowWidth = ImGui::GetWindowSize().x;

        ImGui::SetCursorPosX((windowWidth - width) * 0.5f);
        return ImGui::Button(label, ImVec2{ width, height });
    }

    inline void ImageCentered(std::string id, GLuint& texture, int width, int height)
    {
        auto windowWidth = ImGui::GetWindowSize().x;
        ImGui::SetCursorPosX((windowWidth - width) * 0.5f);

        ImGui::PushID(id.c_str());
        ImGui::Image((void*)(intptr_t)texture, ImVec2(width, height));
        ImGui::PopID();
    }

    inline void Image(std::string id, GLuint& texture, int width, int height)
    {
        ImGui::PushID(id.c_str());
        ImGui::Image((void*)(intptr_t)texture, ImVec2(width, height));
        ImGui::PopID();
    }

    inline bool ImageButton(std::string id, GLuint texture, const ImVec2& size, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1))
    {
        ImGui::PushID(id.c_str());
        bool pressed = ImGui::ImageButton((void*)(intptr_t)texture, size, uv0, uv1, frame_padding, bg_col, tint_col);
        ImGui::PopID();

        return pressed;
    }
}