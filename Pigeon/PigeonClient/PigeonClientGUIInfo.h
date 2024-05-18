#include <string>
#include <map>
#include <unordered_map>
#include <queue>

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

#endif

enum Page
{
	WELCOME = 0,
	CHAT,
	SETTINGS
};

enum MSG_TYPE
{
	PIGEON_TEXT,
	PIGEON_FILE
};

struct GUI_MSG
{
	MSG_TYPE type;
	std::string username;
	std::string content;
};

#define MAX_USERNAME 20

namespace PigeonClientGUIInfo
{
	inline bool shouldClose = false;
	inline bool shouldDelete = false;

	inline int windowWidth = 500;
	inline int windowHeight = 760;

	inline int infoPos = 0;

	inline bool settings = false;
	inline int currentPage = Page::WELCOME;

	inline std::string Username, Address, Port;
	inline bool fetchingData = false;

	inline const char* status_vec[] = { "ONLINE", "IDLE", "DND" };
	inline int currentStatus = 0;
	inline int showMenu = false;
	inline bool selecting = true;
	inline bool focusMSG = false;

	//inline ImGuiTextBuffer msgBuffer;
	inline std::vector<GUI_MSG> msgBuffer;
	inline std::string msg = "";

	inline std::map<std::string, std::string> Users;

	inline std::string donwloadPath = "";
	inline std::string MOTD = "";
}

namespace Texture {
	inline GLuint welcome;
	inline GLuint your_icon;
	inline GLuint upload, settings, disconnect;
	inline GLuint online, idle, dnd, error;
	inline GLuint file;
	inline GLuint close;
	inline GLuint folder;
}

namespace Font {
	namespace MadimiOne {
		inline ImFont* px10, *px20, *px30, *px40, *px50;
	}
	namespace OpenSans {
		inline ImFont* px10, *px20, *px30, *px40, *px45, *px50;
	}
}