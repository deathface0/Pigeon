#include <string>
#include <map>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_stdlib.h>
#include <GLEW/GL/glew.h>

static enum Page
{
	WELCOME_PAGE = 0,
	CHAT_PAGE
};

namespace PigeonClientGUIInfo
{
	inline bool shouldClose = false;
	inline bool shouldDelete = false;

	inline bool shouldLog = false;

	inline int windowWidth = 500;
	inline int windowHeight = 760;

	inline ImFont* smallFont, *mediumFont, *largeFont;

	inline std::string Username, Address, Port;
	inline bool fetchingData = false;

	inline GLuint welcome_texture;
	inline int my_image_width, my_image_height;
	inline bool welcome_loaded = false;

	inline GLuint user_icon_texture;
	inline bool user_icon_loaded = false;

	inline int currentPage = Page::WELCOME_PAGE, lastPage = Page::WELCOME_PAGE;
	inline std::string msg = "";

	inline std::map<std::string, std::string> Users;
}