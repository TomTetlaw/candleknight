#include "includes.h"

float current_time = 0.0f;
float game_time = 0.0f;
float delta_time = 0.0f;
Vec2 cursor_position;
bool is_paused = true;

void quit() {
    editor_shutdown();
    font_shutdown();
    ent_shutdown();
    hotload_shutdown();
    r_shutdown();
    tex_shutdown();
    
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImGui::DestroyContext();
    
    SDL_Quit();
    
    exit(0);
}

void fatal_error(const char *title, const char *text, ...) {
    va_list args;
    char buffer[1024] = {0};
    
    va_start(args, text);
    vsprintf(buffer, text, args);
    va_end(args);
    
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, title, buffer, null);
    quit();
}

Load_File_Result load_entire_file(const char *filename) {
	FILE *f = null;
	Load_File_Result result;
    
	fopen_s(&f, filename, "rb");
    defer { fclose(f); };
    
	if (!f) {
        assert(false, "failed to load file %s", filename);
		return result;
	}
    
	int len = 0;
	fseek(f, 0, SEEK_END);
	len = ftell(f);
	fseek(f, 0, SEEK_SET);
    
    uchar *buffer = allocn(uchar, len + 1);
	fread_s((void*)buffer, len + 1, len, 1, f);
	buffer[len] = 0;
    
	result.data = buffer;
	result.length = len;
	return result;
}

bool point_box_intersection(Vec2 point, Vec2 boxp, Vec2 boxs, bool box_centered) {
    if(box_centered) {
        float halfw = boxs.x / 2;
        float halfh = boxs.y / 2;
        if(point.x < boxp.x - halfw) return false;
        if(point.y < boxp.y - halfh) return false;
        if(point.x > boxp.x + halfw) return false;
        if(point.y > boxp.y + halfh) return false;
    } else {
        if(point.x < boxp.x) return false;
        if(point.y < boxp.y) return false;
        if(point.x > boxp.x + boxs.x) return false;
        if(point.y > boxp.y + boxs.y) return false;
    }
    
    return true;
}

float scale_for_zoom_level() {
	float scale = 0.0f;
	if (zoom_level == 0) {
		scale = 1.0f;
	}
	else if (zoom_level < 0) {
		scale = 1.0f / (((zoom_level * 0.125f) * -1) + 1);
	}
	else if (zoom_level > 0) {
		scale = (zoom_level * 0.25f) + 1;
	}
	return scale;
}

float inverse_scale_for_zoom_level() {
	float scale = 0.0f;
	if (zoom_level == 0) {
		scale = 1.0f;
	}
	else if (zoom_level < 0) {
		scale = ((zoom_level * 0.125f) * -1) + 1;
	}
	else if (zoom_level > 0) {
		scale = 1.0f / ((zoom_level * 0.25f) + 1);
	}
	return scale;
}

Vec2 screen_to_world(Vec2 s) {
    return (s + camera_position) * scale_for_zoom_level();
}

Vec2 world_to_screen(Vec2 w) {
    return w - camera_position * inverse_scale_for_zoom_level();
}

char *format(const char *string, ...) {
    va_list args;
    static char buffer[32][1024] = {0};
    int index = 0;
    
    va_start(args, string);
    vsprintf(buffer[index], string, args);
    va_end(args);
    
    char *ret = buffer[index];
    index = (index + 1) & 31;
    
    return ret;
}

const char *parse_token(const char *text, char token[MAX_TOKEN_LENGTH]) {
	const char *position = text;
	int n = 0;
    
	token[0] = 0;
    
	if (!text) {
		return nullptr;
	}
    
	if (*position == '\0') {
		token[n] = 0;
		return nullptr;
	}
    
	while (*position <= 32) {
		if (*position == '\0') {
			return nullptr;
		}
		position++;
	}
    
	if (*position == '/' && *(position + 1) == '/') {
		while (*position != '\n') {
			if (*position == 0) {
				token[n] = 0;
				return nullptr;
			}
			position++;
		}
	}
    
	if (*position == '"') {
		position++;
		while (*position != '"') {
			if (*position == 0) {
				token[n] = 0;
				return nullptr;
			}
			token[n] = *position;
			n++;
			position++;
		}
		token[n] = 0;
		position++;
		return position;
	}
    
	if (*position == '(') {
		position++;
		while (*position != ')') {
			if (*position == 0) {
				token[n] = 0;
				return nullptr;
			}
			token[n] = *position;
			n++;
			position++;
		}
		token[n] = 0;
		position++;
		return position;
	}
    
    if (*position == '[') {
		position++;
		while (*position != ']') {
			if (*position == 0) {
				token[n] = 0;
				return nullptr;
			}
			token[n] = *position;
			n++;
			position++;
		}
		token[n] = 0;
		position++;
		return position;
	}
    
	while (*position > 32) {
		token[n] = *position;
		n++;
		position++;
        
		if (*position == '"' || *position == '(' || *position == '[') {
			token[n] = 0;
			return position;
		}
	}
    
	token[n] = 0;
	return position;
}

int input_get_key_state(SDL_Scancode scancode) {
	return SDL_GetKeyboardState(nullptr)[scancode];
}

bool shift_down = false;
bool control_down = false;
bool alt_down = false;

void input_collect_control_keys(SDL_Event *event) {
    if(event->type == SDL_KEYDOWN) {
        if(event->key.keysym.sym == SDLK_RCTRL || event->key.keysym.sym == SDLK_LCTRL)
            control_down = true;
        if(event->key.keysym.sym == SDLK_RALT || event->key.keysym.sym == SDLK_LALT)
            alt_down = true;
        if(event->key.keysym.sym == SDLK_RSHIFT || event->key.keysym.sym == SDLK_LSHIFT)
            shift_down = true;
    }
    
    if(event->type == SDL_KEYUP) {
        if(event->key.keysym.sym == SDLK_RCTRL || event->key.keysym.sym == SDLK_LCTRL)
            control_down = false;
        if(event->key.keysym.sym == SDLK_RALT || event->key.keysym.sym == SDLK_LALT)
            alt_down = false;
        if(event->key.keysym.sym == SDLK_RSHIFT || event->key.keysym.sym == SDLK_LSHIFT)
            shift_down = false;
    }
}

float align(float x, float a) {
    return roundf(x / a) * a;
}

Vec2 align(Vec2 v, float a) {
    return Vec2(align(v.x, a), align(v.y, a));
}

void toggle_paused() {
    is_paused = !is_paused;
    game_time = 0.0f;
    camera_position = Vec2(0, 0);
	zoom_level = 0;
}

bool input_translate_scancode(SDL_Scancode scancode, char *ch){
    if(scancode == SDL_SCANCODE_A)            { *ch = shift_down ? 'A' : 'a';  return true; }
	else if(scancode == SDL_SCANCODE_B)            { *ch = shift_down ? 'B' : 'b';  return true; }
	else if(scancode == SDL_SCANCODE_C)            { *ch = shift_down ? 'C' : 'c';  return true; }
	else if(scancode == SDL_SCANCODE_D)            { *ch = shift_down ? 'D' : 'd';  return true; }
	else if(scancode == SDL_SCANCODE_E)            { *ch = shift_down ? 'E' : 'e';  return true; }
	else if(scancode == SDL_SCANCODE_F)            { *ch = shift_down ? 'F' : 'f';  return true; }
	else if(scancode == SDL_SCANCODE_G)            { *ch = shift_down ? 'G' : 'g';  return true; }
	else if(scancode == SDL_SCANCODE_H)            { *ch = shift_down ? 'H' : 'h';  return true; }
	else if(scancode == SDL_SCANCODE_I)            { *ch = shift_down ? 'I' : 'i';  return true; }
	else if(scancode == SDL_SCANCODE_J)            { *ch = shift_down ? 'J' : 'j';  return true; }
	else if(scancode == SDL_SCANCODE_K)            { *ch = shift_down ? 'K' : 'k';  return true; }
	else if(scancode == SDL_SCANCODE_L)            { *ch = shift_down ? 'L' : 'l';  return true; }
	else if(scancode == SDL_SCANCODE_M)            { *ch = shift_down ? 'M' : 'm';  return true; }
	else if(scancode == SDL_SCANCODE_N)            { *ch = shift_down ? 'N' : 'n';  return true; }
	else if(scancode == SDL_SCANCODE_O)            { *ch = shift_down ? 'O' : 'o';  return true; }
	else if(scancode == SDL_SCANCODE_P)            { *ch = shift_down ? 'P' : 'p';  return true; }
	else if(scancode == SDL_SCANCODE_Q)            { *ch = shift_down ? 'Q' : 'q';  return true; }
	else if(scancode == SDL_SCANCODE_R)            { *ch = shift_down ? 'R' : 'r';  return true; }
	else if(scancode == SDL_SCANCODE_S)            { *ch = shift_down ? 'S' : 's';  return true; }
	else if(scancode == SDL_SCANCODE_T)            { *ch = shift_down ? 'T' : 't';  return true; }
	else if(scancode == SDL_SCANCODE_U)            { *ch = shift_down ? 'U' : 'u';  return true; }
	else if(scancode == SDL_SCANCODE_V)            { *ch = shift_down ? 'V' : 'v';  return true; }
	else if(scancode == SDL_SCANCODE_W)            { *ch = shift_down ? 'W' : 'w';  return true; }
	else if(scancode == SDL_SCANCODE_X)            { *ch = shift_down ? 'X' : 'x';  return true; }
	else if(scancode == SDL_SCANCODE_Y)            { *ch = shift_down ? 'Y' : 'y';  return true; }
	else if(scancode == SDL_SCANCODE_Z)            { *ch = shift_down ? 'Z' : 'z';  return true; }
	else if(scancode == SDL_SCANCODE_0)            { *ch = shift_down ? ')' : '0';  return true; }
	else if(scancode == SDL_SCANCODE_1)            { *ch = shift_down ? '!' : '1';  return true; }
	else if(scancode == SDL_SCANCODE_2)            { *ch = shift_down ? '@' : '2';  return true; }
	else if(scancode == SDL_SCANCODE_3)            { *ch = shift_down ? '#' : '3';  return true; }
	else if(scancode == SDL_SCANCODE_4)            { *ch = shift_down ? '$' : '4';  return true; }
	else if(scancode == SDL_SCANCODE_5)            { *ch = shift_down ? '%' : '5';  return true; }
	else if(scancode == SDL_SCANCODE_6)            { *ch = shift_down ? '^' : '6';  return true; }
	else if(scancode == SDL_SCANCODE_7)            { *ch = shift_down ? '&' : '7';  return true; }
	else if(scancode == SDL_SCANCODE_8)            { *ch = shift_down ? '*' : '8';  return true; }
	else if(scancode == SDL_SCANCODE_9)            { *ch = shift_down ? '(' : '9';  return true; }
	else if(scancode == SDL_SCANCODE_MINUS) 	   { *ch = shift_down ? '_' : '-';  return true; }
	else if(scancode == SDL_SCANCODE_EQUALS) 	   { *ch = shift_down ? '+' : '=';  return true; }
	else if(scancode == SDL_SCANCODE_LEFTBRACKET)  { *ch = shift_down ? '{' : '[';  return true; }
	else if(scancode == SDL_SCANCODE_RIGHTBRACKET) { *ch = shift_down ? '}' : ']';  return true; }
	else if(scancode == SDL_SCANCODE_BACKSLASH)    { *ch = shift_down ? '|' : '\\'; return true; }
	else if(scancode == SDL_SCANCODE_SEMICOLON)    { *ch = shift_down ? ':' : ';';  return true; }
	else if(scancode == SDL_SCANCODE_APOSTROPHE)   { *ch = shift_down ? '"' : '\''; return true; }
	else if(scancode == SDL_SCANCODE_COMMA) 	   { *ch = shift_down ? '<' : ',';  return true; }
	else if(scancode == SDL_SCANCODE_PERIOD) 	   { *ch = shift_down ? '>' : '.';  return true; }
	else if(scancode == SDL_SCANCODE_SLASH) 	   { *ch = shift_down ? '?' : '/';  return true; }
	else if(scancode == SDL_SCANCODE_SPACE) 	   { *ch =                       ' ';  return true; }
	else return false;
}