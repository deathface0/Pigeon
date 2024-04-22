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

#include "PigeonClient/PigeonClient.h"

//MSVC
#else

#include "PigeonClient/PigeonClient.h"

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
			ImGui::PushFont(Font::MadimiOne::px50);
			GUIUtils::TextCentered("Welcome to Pigeon");

			//Logo
			GUIUtils::ImageCentered("Logo", Texture::welcome, 250, 250);

			//Form
			ImGui::NewLine();
			ImGui::PushFont(Font::MadimiOne::px20);
			GUIUtils::TextCentered("Server Address");
			GUIUtils::InputCentered("##addressField", Address, 400, fetchingData ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_CharsNoBlank);

			GUIUtils::TextCentered("Port");
			GUIUtils::InputCentered("##portField", Port, 400, fetchingData ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_CharsNoBlank);

			GUIUtils::TextCentered("Username");
			GUIUtils::InputCentered("##usernameField", Username, 400, fetchingData ? ImGuiInputTextFlags_ReadOnly : ImGuiInputTextFlags_CharsNoBlank);

			ImGui::NewLine();
			if (GUIUtils::ButtonCentered("Connect", 200, 50) || ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter)))
			{
				/*if (Address.empty() || Port.empty() || Username.empty())
					return;*/

				if (Username.size() > MAX_USERNAME)
				{
					//ERROR: USERNAME TOO LONG
					return;
				}

				Address = "192.168.1.135";
				Port = "4444";
				
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
				return Texture::online;
			case 1:
				return Texture::idle;
			case 2:
				return Texture::dnd;
			default:
				return Texture::error;
			}
		}

		void renderFileUpload(std::string username, std::string filename, std::string ext)
		{
			static int numFiles = 0;
			std::string label = "File" + std::to_string(numFiles++);
			std::string file = filename + ext;

			ImGui::BeginChild(label.c_str(), ImVec2(windowWidth - 220, 100), true);

			if (ext != ".png" && ext != ".jpg" && ext != ".jpeg")
			{
				ImGui::SetCursorPosX(10); ImGui::Text("%s:", username);
				ImGui::Dummy(ImVec2(0.0f, 10.0f)); ImGui::SetCursorPosX(7); GUIUtils::Image(label, Texture::file, 40, 40);
				ImGui::SameLine();

				ImVec2 textPos = ImGui::GetCursorScreenPos();
				ImVec2 textSize = ImGui::CalcTextSize(file.c_str());

				if (ImGui::IsMouseHoveringRect(textPos, { textPos.x + textSize.x, textPos.y + + textSize.y }))
				{
					ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.212, 0.722, 1.00, 1.00f));
					ImGui::Text("%s", file.c_str());
					ImGui::PopStyleColor();

					if (ImGui::IsMouseClicked(0))
					{
						std::string json = R"({"filename":")" + filename + R"("})";
						PigeonPacket pkt = client->BuildPacket(MEDIA_DOWNLOAD, Username, std::vector<unsigned char>(json.begin(), json.end()));
						client->SendPacket(pkt);
					}
				}
				else
				{
					ImGui::Text("%s", file.c_str());
				}
				
			}
			else {
				//RENDER IMAGE
			}

			ImGui::EndChild();
		}

		void printMsgBuffer()
		{
			for (const auto& msg : msgBuffer)
			{
				switch (msg.type) {
				case MSG_TYPE::PIGEON_TEXT:
					ImGui::SetCursorPosX(10);
					ImGui::Text("%s: %s", msg.username, msg.content);
					ImGui::Separator();
					break;
				case MSG_TYPE::PIGEON_FILE:
					size_t pos = msg.content.find_last_of('.');
					std::string filename = msg.content.substr(0, pos);
					std::string ext;

					if (pos != std::string::npos) {
						ext = msg.content.substr(pos + 1);

						if (!ext.empty()) {
							ext = '.' + ext;
						}
					}

					//Converting extension to lowercase
					std::transform(ext.begin(), ext.end(), ext.begin(),
						[](unsigned char c) { return std::tolower(c); });

					renderFileUpload(msg.username, filename, ext);
					ImGui::Separator();
					break;
				}
			}
		}

		void LeftMenu() {
			ImGui::PushFont(Font::OpenSans::px30);

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

			// Menu desplegable
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

			ImGui::BeginChild("Chat Info", ImVec2(windowWidth - 220, 50), true);
			
			int TextSize = 0;

			ImGui::SetCursorPosY(10);
			ImGui::SetCursorPosX(infoPos);
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.525, 0.502, 0.89, 1.00f));
			ImGui::Text("Connected to \"%s\"", client->GetServername().c_str());
			TextSize += ImGui::GetItemRectSize().x;
			ImGui::PopStyleColor();
			ImGui::SameLine();
			ImGui::Text(" - ");
			ImGui::SameLine();
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.89, 0.878, 0.129, 1.00f));
			ImGui::Text("\"%s\"", MOTD.c_str());
			TextSize += ImGui::GetItemRectSize().x;
			ImGui::PopStyleColor();

			infoPos++;
			if (infoPos >= windowWidth - 220)
				infoPos = -TextSize;
			
			ImGui::GetStyle().Colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);

			ImGui::EndChild();

			ImGui::BeginChild("Chat Log", ImVec2(windowWidth - 220, windowHeight - 100), true);

			printMsgBuffer();
			ImGui::SetScrollHereY(1.0f); //Always stay on the bottom side of the chat

			ImGui::EndChild(); //End of Chat Log Child

			ImGui::BeginChild("MSG", ImVec2(windowWidth - 220, 50), true);

			if (focusMSG) //Only focus InputText when new msg is sent
			{
				ImGui::SetKeyboardFocusHere(0); //Maintain input text always selected
				focusMSG = false;
			}
			ImGui::PushFont(Font::OpenSans::px40);
			ImGui::PushItemWidth(windowWidth - 370);
			ImGui::InputText("##MSG", &msg);
			if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter))) {
				focusMSG = true;
				if (!msg.empty())
				{
					PigeonPacket pkg = client->BuildPacket(PIGEON_OPCODE::TEXT_MESSAGE, Username, String::StringToBytes(msg));
					client->SendPacket(pkg);
					msg.clear();
				}
			}
			ImGui::SameLine();
			if (GUIUtils::ImageButton("Upload", Texture::upload, ImVec2((float)47, (float)47), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 1, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f))) {
#ifdef _WIN32
				std::string filepath = File::selectFile();

				size_t lastSlashPos = filepath.find_last_of('/');
				std::string filenameWithExt = filepath.substr(lastSlashPos + 1);

				size_t lastDotPos = filenameWithExt.find_last_of('.');
				std::string filename = filenameWithExt.substr(0, lastDotPos);
				std::string extension = (lastDotPos != std::string::npos) ? filenameWithExt.substr(lastDotPos + 1) : "";
				extension.erase(std::remove(extension.begin(), extension.end(), '\0'), extension.end());

				if (!filepath.empty())
				{
					std::string encoded_file = B64::base64_encode(String::BytesToString(File::DiskToBuffer(filepath)));
					
					std::string json = R"({"content":")" + encoded_file + R"(", "ext":")" + extension + R"(", "filename":")" + filename + R"("})";
					PigeonPacket pkt = client->BuildPacket(MEDIA_FILE, Username, std::vector<unsigned char>(json.begin(), json.end()));
					client->SendPacket(pkt);
				}
#endif
			}
			ImGui::SameLine();
			if (GUIUtils::ImageButton("Settings", Texture::settings, ImVec2((float)47, (float)47), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 1, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f))) {
				settings = true;
			}
			ImGui::SameLine();
			if (GUIUtils::ImageButton("Disconnect", Texture::disconnect, ImVec2((float)47, (float)47), ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), 1, ImVec4(0.0f, 0.0f, 0.0f, 0.0f), ImVec4(1.0f, 1.0f, 1.0f, 1.0f))) {
				PigeonPacket pkg = client->BuildPacket(PIGEON_OPCODE::PRESENCE_UPDATE, Username, String::StringToBytes("DISCONNECT"));
				client->SendPacket(pkg);
			}
			
			ImGui::EndChild(); //End of MSG Child

			ImGui::EndChild(); //End of Chat Child
		}

		void ChatPage()
		{
			LeftMenu();
			ImGui::SameLine();
			RightMenu();
		}
	}

	namespace Settings
	{
		void SettingsPage()
		{
			ImGui::PushFont(Font::OpenSans::px30);

			ImGui::SetCursorPos(ImVec2(windowWidth - 80, 13));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 300);
			if (GUIUtils::ImageButton("##ReturnToChat", Texture::close, ImVec2(40, 40)))
				settings = false;
			ImGui::PopStyleVar();

			static int settingPosY = 80;
			ImGui::SetCursorPos(ImVec2(20, settingPosY));
			ImGui::PushItemWidth(600);
			ImGui::InputText("##DownloadPath", &donwloadPath);
			ImGui::SameLine();
			if (GUIUtils::ImageButton("##SelectDownloadPath", Texture::folder, ImVec2(30, 30)))
				donwloadPath = File::SelectDirectory();
			ImGui::SameLine(); ImGui::Dummy(ImVec2(10, 0)); ImGui::SameLine();
			ImGui::Text("Download path");
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
	}

	void SetChatStyle()
	{
		ImGui::GetStyle().WindowPadding = ImVec2(0.00f, 0.00f);
		ImGui::GetStyle().FrameRounding = 0.f;
		ImGui::GetStyle().ItemSpacing = ImVec2(0.00f, 0.00f);

		ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.f));
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

		ImGui::GetIO().IniFilename = NULL;
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
		if (!client || !client->isConnected())
			currentPage = Page::WELCOME;
		else if (settings)
			currentPage = Page::SETTINGS;
		else
			currentPage = Page::CHAT;

		//Select current page
		switch (currentPage)
		{
		case Page::WELCOME:
			SDL_SetWindowSize(sdlWindow, 500, 760);
			SetUpStyle();
			Welcome::WelcomePage();
			break;
		case Page::CHAT:
			SDL_SetWindowSize(sdlWindow, 1280, 720);
			SetChatStyle();
			Chat::ChatPage();
			break;
		case Page::SETTINGS:
			SDL_SetWindowSize(sdlWindow, 1280, 720);
			SetChatStyle();
			Settings::SettingsPage();
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