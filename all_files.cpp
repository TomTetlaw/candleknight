#pragma comment(lib, "../lib/sdl2.lib")
#pragma comment(lib, "../lib/sdl2_ttf.lib")
#pragma comment(lib, "../lib/glew32.lib")
#pragma comment(lib, "opengl32.lib")
#define STB_IMAGE_IMPLEMENTATION
#define STB_TRUETYPE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define SDL_MAIN_HANDLED
#include "includes.h"
#include "imgui/imgui.cpp"
#include "imgui/imgui_draw.cpp"
#include "imgui/imgui_impl_opengl3.cpp"
#include "imgui/imgui_impl_sdl.cpp"
#include "imgui/imgui_tables.cpp"
#include "imgui/imgui_widgets.cpp"
#include "main.cpp"
#include "utils.cpp"
#include "render.cpp"
#include "texture.cpp"
#include "hotload_win.cpp"
#include "entity.cpp"
#include "font.cpp"
//#include "ui.cpp"
#include "editor.cpp"
#include "physics.cpp"
#include "particle.cpp"
#include "entity_types.cpp"
#include "textedit.cpp"