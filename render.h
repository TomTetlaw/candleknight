#ifndef RENDER_H
#define RENDER_H

struct Program {
	const char *vertex_filename = nullptr;
	const char *frag_filename = nullptr;
	unsigned int vertex_object = 0;
	unsigned int frag_object = 0;
	unsigned int program_object = 0;
};

struct Vertex {
	Vec3 position;
	Vec4 colour;
	Vec2 uv;
	Vec3 normal;
};

struct Texture;
struct RC_Texture {
	Texture *texture = null;
};

struct RC_Quad {
};

enum Render_Command_Type {
	RC_TEXTURE,
    RC_QUAD,
};

struct Render_Command {
	Render_Command() {}
    
	Render_Command_Type type;
    int first_index = -1;
    Program *program = null;
    bool wireframe = false;
    bool use_camera_position = false;
    
	union {
		RC_Texture texture;
        RC_Quad quad;
	};
};

extern int window_width;
extern int window_height;
extern Vec2 window_size;
extern Vec2 camera_position;
extern int zoom_level;

void r_init();
void r_shutdown();
void r_execute_commands();

void r_render_frame();

void setup_program(Program *program, const char *vert_filename, const char *frag_filename);

void r_render_texture(Texture *texture, Vec2 position, bool use_camera_position, Vec4 colour, Vec2 size = Vec2(-1, -1), bool centered = true);

struct Font;
void debug_string(const char *text, ...);
void r_render_string(Vec2 position, bool use_camera_position, const char *text, Vec4 colour = Vec4(1, 1, 1, 1), Font *font = nullptr, float wrap = -1.0f);
void r_render_string_format(Vec2 position, bool use_camera_position, Vec4 colour, Font *font, float wrap, const char *text, ...);
void r_render_string_format_lazy(Vec2 position, bool use_camera_position, const char *text, ...);
void r_render_box(Vec2 position, bool use_camera_position, Vec2 size, bool centered = false, Vec4 colour = Vec4(1, 1, 1, 1), bool wireframe = false);

#endif
