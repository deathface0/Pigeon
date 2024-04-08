#pragma once

#include "PigeonClient.h"
#include  "../Utils/Utils.h"

#include <SDL2/SDL.h>

//UBUNTU
#ifdef __linux__

#include "../imgui/imgui.h"
#include "../imgui/imgui_impl_sdl2.h"
#include "../imgui/imgui_impl_opengl3.h"
#include "../imgui/imgui_stdlib.h"

#include <GL/glew.h>

//MSVC
#else


#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_stdlib.h>
#include <GLEW/GL/glew.h>
#include "Utils/Utils.h"


#endif
#include <iostream>
#include <chrono>
#include <ctime>
#include <vector>
#include <map>

using namespace PigeonClientGUIInfo;

void printMsgBuffer()
{
	const char* buf_begin = msgBuffer.begin();
	const char* buf_end = msgBuffer.end();
	if (buf_begin != buf_end) {
		const char* line = buf_begin;
		for (const char* p = buf_begin; p != buf_end; p++) {
			if (*p == '\n') {
				ImGui::TextUnformatted(line, p);
				ImGui::Separator(); // A�adir una l�nea de separaci�n despu�s de cada mensaje
				line = p + 1;
			}
		}
		if (line != buf_end)
			ImGui::TextUnformatted(line, buf_end);
	}
}

namespace PigeonClientGUI
{
	PigeonClient* client = nullptr;

    SDL_Window* sdlWindow = nullptr;
    SDL_GLContext glContext = NULL;
    SDL_Event currentEvent;

	namespace Welcome
	{
		void WelcomePage()
		{
			//Title
			//ImGui::PushFont(largeFont);
			GUIUtils::TextCentered("Welcome to Pigeon");

			//Logo
			GUIUtils::ImageCentered(welcome_texture, 250, 250);

			//Form
			ImGui::NewLine();
			//ImGui::PushFont(smallFont);
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

				if (Username.size() > MAX_USERNAME)
				{
					//ERROR: USERNAME TOO LONG
					return;
				}

				//Address = "192.168.1.135";
				//Port = "4444";
				
				/*Username = "Ahuesag";*/

				client = new PigeonClient(Address, stoi(Port), Username);
				client->Run();
			}
		}
	}

	namespace Chat
	{
		GLuint& getStatusImage(int status)
		{
			switch (status)
			{
			case 0:
				return online_texture;
			case 1:
				return idle_texture;
			case 2:
				return dnd_texture;
			default:
				return error_texture;
			}
		}

		void LeftMenu() {
			//ImGui::PushFont(mediumFont);

			ImGui::BeginChild("Left Menu", ImVec2(220, windowHeight), true);

			ImGui::BeginChild("Connected clients", ImVec2(220, windowHeight - 50), true);
			
			int clientIndex = 0; ImVec2 clientStatusPos = { 10, 10 };
			for (const auto& pair : Users) {
				ImGui::SetCursorPos(clientStatusPos);
				if (GUIUtils::RoundButton(pair.first, getStatusImage(stoi(pair.second)), 30)) //Fix, utils no reload images
				{
					std::cout << "User: " << pair.first << ", Status: " << pair.second << std::endl;
				}

				ImGui::SameLine();
				ImGui::SetCursorPosX(clientStatusPos.x + 35);
				ImGui::Text("%s", pair.first.c_str());

				clientStatusPos.y += 40;
			}

			ImGui::EndChild(); //End of Connected clients Child

			ImGui::BeginChild("User Status", ImVec2(220, 50), true);

			currentStatus = stoi(Users[Username]); //Users[Username] always exists (own username)
			ImGui::SetCursorPos(ImVec2(10, 10));
			if (GUIUtils::RoundButton("##your_status", getStatusImage(currentStatus), 30))
			{
				showMenu = true;
			}
			ImGui::SameLine();
			ImGui::SetCursorPos(ImVec2(45, 10));
			ImGui::Text("%s", Username.c_str());

			// Men� desplegable
			if (showMenu) {
				ImGui::SetNextWindowPos(ImVec2(0, windowHeight - 180));
				ImGui::Begin("Status window", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBackground);

				if (!ImGui::IsWindowFocused() && !selecting) //Maintain window on top
					ImGui::SetWindowFocus(); 

				selecting = false;
				ImGui::SetNextItemWidth(220);
				if (ImGui::BeginCombo("##A", status_vec[currentStatus])) {
					selecting = true;

					for (int i = 0; i < IM_ARRAYSIZE(status_vec); i++) {
						bool isSelected = (currentStatus == i);
						if (ImGui::Selectable(status_vec[i], isSelected))
						{
							client->ChangeStatus(status_vec[i]);
							showMenu = false;
						}
						if (isSelected)
							ImGui::SetItemDefaultFocus();   // Establecer el enfoque en la opci�n seleccionada
					}
					ImGui::EndCombo();
				}
				ImGui::End();
			}

			ImGui::EndChild(); //End of User Status Child

			ImGui::EndChild(); //End of Left Menu Child
		}

		void RightMenu()
		{
			ImGui::BeginChild("Chat", ImVec2(windowWidth - 220, windowHeight), true);

			ImGui::BeginChild("Chat Log", ImVec2(windowWidth - 220, windowHeight - 50), true);

			printMsgBuffer();

			ImGui::EndChild(); //End of Chat Log Child

			ImGui::BeginChild("MSG", ImVec2(windowWidth - 220, 50), true);

			if (focusMSG) //Only focus InputText when new msg is sent
			{
				ImGui::SetKeyboardFocusHere(0); //Maintain input text always selected
				focusMSG = false;
			}
			//ImGui::PushFont(msgFont);
			ImGui::PushItemWidth(windowWidth - 220);
			ImGui::InputText("##MSG", &msg);
			if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter))) {
				PigeonPacket pkg = client->BuildPacket(PIGEON_OPCODE::TEXT_MESSAGE, Username, String::StringToBytes(msg));
				client->SendPacket(pkg);
				msg.clear();

				focusMSG = true;
			}

			ImGui::EndChild(); //End of MSG Child

			ImGui::EndChild(); //End of Chat Child
		}

		void ChatPage()
		{
			//ImGui::ShowDemoWindow();
			LeftMenu();
			ImGui::SameLine();
			RightMenu();

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

		largeFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/MadimiOne-Regular.ttf", 50);
		mediumFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/MadimiOne-Regular.ttf", 30);
		smallFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/MadimiOne-Regular.ttf", 20);
		msgFont = ImGui::GetIO().Fonts->AddFontFromFileTTF("Fonts/MadimiOne-Regular.ttf", 40);

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
				ImGui::GetStyle().ItemSpacing = ImVec2(0.00f, 0.00f);
			}
			else if (currentPage == Page::WELCOME_PAGE)
			{
				SDL_SetWindowSize(sdlWindow, 500, 760);
				ImGui::GetStyle().WindowPadding = ImVec2(8.00f, 8.00f);
				ImGui::GetStyle().ItemSpacing = ImVec2(6.00f, 6.00f);
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