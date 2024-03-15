#pragma once

#include "PigeonClient.h"
#include "ImGuiLogger.h"
#include "Utils/Utils.h"

#include <SDL2/SDL.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_stdlib.h>
#include <GLEW/GL/glew.h>
#include <iostream>
#include <chrono>
#include <ctime>
#include <vector>
#include <map>

using namespace PigeonClientGUIInfo;

namespace PigeonClientGUI
{
	PigeonClient* client = nullptr;

    SDL_Window* sdlWindow = nullptr;
    SDL_GLContext glContext = NULL;
    SDL_Event currentEvent;

    ImGuiLog* log = nullptr;

	namespace Welcome
	{
		void WelcomePage()
		{
			//Title
			ImGui::PushFont(largeFont);
			GUIUtils::TextCentered("Welcome to Pigeon");

			//Logo
			GUIUtils::ImageCentered("Images\\logo2.png", welcome_texture, my_image_width, my_image_height, 0.5, welcome_loaded);

			//Form
			ImGui::NewLine();
			ImGui::PushFont(smallFont);
			GUIUtils::TextCentered("Server Address");
			GUIUtils::InputCentered("##addressField", Address, 400, fetchingData ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_CharsNoBlank);

			GUIUtils::TextCentered("Port");
			GUIUtils::InputCentered("##portField", Port, 400, fetchingData ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_CharsNoBlank);

			GUIUtils::TextCentered("Username");
			GUIUtils::InputCentered("##usernameField", Username, 400, fetchingData ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_CharsNoBlank);

			ImGui::NewLine();
			if (GUIUtils::ButtonCentered("Connect", 200, 50))
			{
				/*if (Address.empty() || Port.empty() || Username.empty())
					return;*/

				Address = "192.168.100.16";
				Port = "4444";
				/*Username = "Ahuesag";*/

				client = new PigeonClient(Address, stoi(Port), Username);
				client->Run();
			}
		}
	}

	namespace Chat
	{
		std::string getStatusImage(int status)
		{
			switch (status)
			{
			case 0:
				return "Images\\online_status.png";
			case 1:
				return "Images\\idle_status.png";
			case 2:
				return "Images\\dnd_status.png";
			default:
				return "Images\\error_status.png";
			}
		}

		void RenderTabBar() {
			ImGui::PushFont(mediumFont);
			//ImGui::ShowDemoWindow();

			ImGui::BeginChild("Connected clients", ImVec2(220, windowHeight), true);
			
			std::string statusIcon = "";
			for (const auto& pair : Users) {
				statusIcon = getStatusImage(stoi(pair.second));
				if (GUIUtils::RoundButton(statusIcon.c_str(), user_icon_texture, 30, user_icon_loaded))
				{
					std::cout << "User: " << pair.first << std::endl;
				}
				ImGui::SameLine();
				ImGui::Text("%s", pair.first);
			}

			/*
			for (int i = 0; i < 50; i++)
			{
				std::string text = "User" + std::to_string(i);
				GUIUtils::RoundButton("Images\\logo2.png", user_icon_texture, 40, user_icon_loaded);
				ImGui::SameLine();
				ImGui::Text("%s", text);
			}*/

			ImGui::EndChild();
		}

		void ChatPage()
		{
			RenderTabBar();
			return;

			GUIUtils::InputCentered("##sendMsg", msg, 300, ImGuiInputTextFlags_CharsNoBlank);
			if (GUIUtils::ButtonCentered("Send", 100, 50))
			{
				PigeonPacket pkg = client->BuildPacket(PIGEON_OPCODE::TEXT_MESSAGE, Username, String::StringToBytes(msg));
				client->SendPacket(pkg);
			}
		}
	}

	


    int CreateImGuiWindow()
    {
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_RESIZABLE);

        sdlWindow = SDL_CreateWindow("Pigeon Client", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, windowWidth, windowHeight, window_flags);

        glContext = SDL_GL_CreateContext(sdlWindow);
        if (SDL_GL_MakeCurrent(sdlWindow, glContext) != 0) {
            std::cerr << "Error setting up OpenGL context " << std::endl;
            return -1;
        };

        if (glewInit() != GLEW_OK) {
            std::cout << "Error initializing glew" << std::endl;
        }

        SDL_GL_SetSwapInterval(1);
        return 0;
    }

	void SetUpStyle() {
		ImVec4* colors = ImGui::GetStyle().Colors;
		colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
		colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
		colors[ImGuiCol_WindowBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
		colors[ImGuiCol_ChildBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_PopupBg] = ImVec4(0.19f, 0.19f, 0.19f, 0.92f);
		colors[ImGuiCol_Border] = ImVec4(0.19f, 0.19f, 0.19f, 0.29f);
		colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.24f);
		colors[ImGuiCol_FrameBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		colors[ImGuiCol_FrameBgHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
		colors[ImGuiCol_FrameBgActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		colors[ImGuiCol_TitleBg] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_TitleBgActive] = ImVec4(0.06f, 0.06f, 0.06f, 1.00f);
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_MenuBarBg] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_ScrollbarBg] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
		colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.40f, 0.40f, 0.40f, 0.54f);
		colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
		colors[ImGuiCol_CheckMark] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		colors[ImGuiCol_SliderGrab] = ImVec4(0.34f, 0.34f, 0.34f, 0.54f);
		colors[ImGuiCol_SliderGrabActive] = ImVec4(0.56f, 0.56f, 0.56f, 0.54f);
		//colors[ImGuiCol_Button] = ImVec4(0.05f, 0.05f, 0.05f, 0.54f);
		colors[ImGuiCol_Button] = ImVec4(0.796f, 0.592f, 0.357f, 1.00f);
		colors[ImGuiCol_ButtonHovered] = ImVec4(0.19f, 0.19f, 0.19f, 0.54f);
		colors[ImGuiCol_ButtonActive] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		colors[ImGuiCol_Header] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_HeaderHovered] = ImVec4(0.00f, 0.00f, 0.00f, 0.36f);
		colors[ImGuiCol_HeaderActive] = ImVec4(0.20f, 0.22f, 0.23f, 0.33f);
		colors[ImGuiCol_Separator] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		colors[ImGuiCol_SeparatorHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
		colors[ImGuiCol_SeparatorActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
		colors[ImGuiCol_ResizeGrip] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.44f, 0.44f, 0.44f, 0.29f);
		colors[ImGuiCol_ResizeGripActive] = ImVec4(0.40f, 0.44f, 0.47f, 1.00f);
		colors[ImGuiCol_Tab] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TabHovered] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_TabActive] = ImVec4(0.20f, 0.20f, 0.20f, 0.36f);
		colors[ImGuiCol_TabUnfocused] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.14f, 0.14f, 0.14f, 1.00f);
		colors[ImGuiCol_DockingPreview] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		colors[ImGuiCol_DockingEmptyBg] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotLines] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogram] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_TableHeaderBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TableBorderStrong] = ImVec4(0.00f, 0.00f, 0.00f, 0.52f);
		colors[ImGuiCol_TableBorderLight] = ImVec4(0.28f, 0.28f, 0.28f, 0.29f);
		colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
		colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
		colors[ImGuiCol_TextSelectedBg] = ImVec4(0.20f, 0.22f, 0.23f, 1.00f);
		colors[ImGuiCol_DragDropTarget] = ImVec4(0.33f, 0.67f, 0.86f, 1.00f);
		colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 1.00f);
		colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 0.00f, 0.00f, 0.70f);
		colors[ImGuiCol_NavWindowingDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.20f);
		colors[ImGuiCol_ModalWindowDimBg] = ImVec4(1.00f, 0.00f, 0.00f, 0.35f);

		ImGuiStyle& style = ImGui::GetStyle();
		style.WindowPadding = ImVec2(8.00f, 8.00f);
		style.FramePadding = ImVec2(5.00f, 2.00f);
		style.CellPadding = ImVec2(6.00f, 6.00f);
		style.ItemSpacing = ImVec2(6.00f, 6.00f);
		style.ItemInnerSpacing = ImVec2(6.00f, 6.00f);
		style.TouchExtraPadding = ImVec2(0.00f, 0.00f);
		style.IndentSpacing = 25;
		style.ScrollbarSize = 5;
		style.GrabMinSize = 10;
		style.WindowBorderSize = 1;
		style.ChildBorderSize = 1;
		style.PopupBorderSize = 1;
		style.FrameBorderSize = 1;
		style.TabBorderSize = 1;
		style.WindowRounding = 7;
		style.ChildRounding = 4;
		style.FrameRounding = 3;
		style.PopupRounding = 4;
		style.ScrollbarRounding = 9;
		style.GrabRounding = 3;
		style.LogSliderDeadzone = 4;
		style.TabRounding = 4;
		style.ScaleAllSizes(2.5);
		ImGui::GetStyle().WindowRounding = 0.0f;
		ImGui::GetStyle().ChildRounding = 0.0f;
		ImGui::GetStyle().FrameRounding = 0.0f;
		ImGui::GetStyle().GrabRounding = 0.0f;
		ImGui::GetStyle().PopupRounding = 0.0f;
		ImGui::GetStyle().ScrollbarRounding = 0.0f;
		ImGui::GetStyle().FrameRounding = 100.f;

		largeFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts\\MadimiOne-Regular.ttf", 50);
		mediumFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts\\MadimiOne-Regular.ttf", 30);
		smallFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts\\MadimiOne-Regular.ttf", 20);
	}

    void SetUpImGui() {
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();

        //Docking creo que esta bug con SDL, con glfw si funciona
        //imIO.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        //imIO.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;

        ImGui::StyleColorsDark();

        ImGui_ImplSDL2_InitForOpenGL(sdlWindow, glContext);
        ImGui_ImplOpenGL3_Init("#version 330 core");
        SetUpStyle();
    }

    void StartImGuiFrame() {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame();
        ImGui::NewFrame();

        //This is to make the first imgui window fill all the sdl window
        ImGui::SetNextWindowSize(ImVec2(windowWidth, windowHeight));
        ImGui::SetNextWindowPos(ImVec2(0, 0));
    }

    void Render() {
        //End of the main window
        ImGui::End();
        ImGui::Render();

        glViewport(0, 0, ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y);

        glClearColor(0.10f, 0.10f, 0.10f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
		
		//stbi_image_free(imageData);

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(sdlWindow);
    }

    void MainWindow() {
        ImGui::Begin("Pigeon Client", nullptr, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);

		//Resize Imgui window
		SDL_GetWindowSize(sdlWindow, &windowWidth, &windowHeight);
		ImVec2 newSize = ImVec2(windowWidth * 1.0f, windowHeight * 1.0f);
		ImGui::SetWindowSize(newSize);

		//Detect page change and set styles/size
		currentPage = client && client->isConnected() ? Page::CHAT_PAGE : Page::WELCOME_PAGE;
		if (currentPage != lastPage)
		{
			if (currentPage == Page::CHAT_PAGE)
			{
				SDL_SetWindowSize(sdlWindow, 1280, 720);
				ImGui::GetStyle().WindowPadding = ImVec2(0.00f, 0.00f);
				ImGui::GetStyle().FrameRounding = 0.f;
			}
			else if (currentPage == Page::WELCOME_PAGE)
			{
				SDL_SetWindowSize(sdlWindow, 500, 760);
				ImGui::GetStyle().WindowPadding = ImVec2(8.00f, 8.00f);
			}
		}
		lastPage = currentPage;

		//Select current page
		switch (currentPage)
		{
		case Page::WELCOME_PAGE:
			Welcome::WelcomePage();
			break;
		case Page::CHAT_PAGE:
			Chat::ChatPage();
			break;
		}
		
    }

	void Cleanup() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplSDL2_Shutdown();

		ImGui::DestroyContext();

		SDL_GL_DeleteContext(glContext);
		SDL_DestroyWindow(sdlWindow);
		SDL_Quit();
	}
}