#include <string>
#include <map>
#include <unordered_map>


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
	WELCOME_PAGE = 0,
	CHAT_PAGE
};

#define MAX_USERNAME 15

namespace PigeonClientGUIInfo
{
	inline bool shouldClose = false;
	inline bool shouldDelete = false;

	inline int windowWidth = 500;
	inline int windowHeight = 760;

	inline int currentPage = Page::WELCOME_PAGE, lastPage = Page::WELCOME_PAGE;

	inline ImFont *smallFont, *mediumFont, *largeFont, *msgFont;

	inline std::string Username, Address, Port;
	inline bool fetchingData = false;

	inline GLuint welcome_texture;
	inline GLuint your_icon_texture;
	inline GLuint file_texture;
	inline GLuint online_texture, idle_texture, dnd_texture, error_texture;
	inline const char* status_vec[] = { "ONLINE", "IDLE", "DND" };
	inline int currentStatus = 0;
	inline int showMenu = false;
	inline bool selecting = true;
	inline bool focusMSG = false;

	inline ImGuiTextBuffer msgBuffer;
	inline std::string msg = "";

	inline std::map<std::string, std::string> Users;
}