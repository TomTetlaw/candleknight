#include "includes.h"

internal Mat4 projection_matrix;
internal Mat4 transformation_matrix;

internal bool should_clear = true;
internal Vec4 clear_colour;

internal unsigned int vertex_buffer = 0;
internal unsigned int index_buffer = 0;
internal unsigned int vertex_array = 0;

#define MAX_VERTICIES 1000000
#define MAX_INDICES 1000000

internal Array<Vertex> verts_buffer;
internal Array<unsigned int> indices_buffer;
internal Array<Render_Command> r_commands;

internal Program prog_textured;
internal Program prog_text;

Texture *default_texture = null;

int window_width = 1366;
int window_height = 768;
Vec2 window_size = Vec2(window_width, window_height);

internal Vec2 camera_position;
internal int zoom_level = 0;
internal Vec2 debug_string_position = Vec2(250, 20);

SDL_Window *sdl_window = null;

void opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);

void r_init() {
    glewInit();
    
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_BLEND); 
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 
    glDebugMessageCallback((GLDEBUGPROC)opengl_debug_callback, null);
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, null, GL_TRUE);
    
	FILE *f = null;
	fopen_s(&f, "data/opengl.log", "w");
	if(f) {
		fclose(f);
	}
    
	glGenVertexArrays(1, &vertex_array);
	glBindVertexArray(vertex_array);
    
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, MAX_VERTICIES * sizeof(Vertex), null, GL_DYNAMIC_DRAW);
    
	glGenBuffers(1, &index_buffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, MAX_INDICES * sizeof(unsigned int), null, GL_DYNAMIC_DRAW);
    
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, position));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, colour));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, uv));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (const void *)offsetof(Vertex, normal));
    
    transformation_matrix = create_identity_matrix();
	projection_matrix = create_identity_matrix();
    
    setup_program(&prog_textured, "data/shaders/textured.vert", "data/shaders/textured.frag");
    setup_program(&prog_text, "data/shaders/textured.vert", "data/shaders/text.frag");
    
    default_texture = load_texture("data/textures/default.jpg");
}

void r_shutdown() {
}

unsigned int load_shader(const char *filename, unsigned int type) {
	unsigned int object;
	GLint compiled = 0;
    
	Load_File_Result file = load_entire_file(filename);
	object = glCreateShader(type);
	glShaderSource(object, 1, (const GLchar * const *)&file.data, &file.length);
	glCompileShader(object);
	delete[] file.data;
    
	glGetShaderiv(object, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		GLint blen = 0;	
		GLsizei slen = 0;
		glGetShaderiv(object, GL_INFO_LOG_LENGTH , &blen);       
		if (blen > 1) {
            char *compiler_log = allocn(char, blen);
            glGetShaderInfoLog(object, blen, &slen, compiler_log);
            fatal_error("ERROR", "Failed to compile shader %s: %s", filename, compiler_log);
            free(compiler_log);
		}
        
		glDeleteShader(object);
		return 0;
	}
    
	return object;
}

void load_program(Program *program) {
	program->vertex_object = load_shader(program->vertex_filename, GL_VERTEX_SHADER);
	program->frag_object = load_shader(program->frag_filename, GL_FRAGMENT_SHADER);
	program->program_object = glCreateProgram();
	glAttachShader(program->program_object, program->vertex_object);
	glAttachShader(program->program_object, program->frag_object);
	glLinkProgram(program->program_object);
	
	GLint linked;
	glGetProgramiv(program->program_object, GL_LINK_STATUS, &linked);
	if(!linked) {
		GLint blen = 0;	
		GLsizei slen = 0;
		glGetProgramiv(program->program_object, GL_INFO_LOG_LENGTH, &blen);       
		if (blen > 1) {
            char *compiler_log = allocn(char, blen);
            glGetProgramInfoLog(program->program_object, blen, &slen, compiler_log);
            fatal_error("ERROR", "Failed to link program: %s", compiler_log);
            free(compiler_log);
		}
        
		glDeleteProgram(program->program_object);
	}
}

int add_verts(Vertex *verts, int num_verts, unsigned int *indices, int num_indices) {
    array_reserve(&verts_buffer, verts_buffer.count + num_verts);
    array_reserve(&indices_buffer, indices_buffer.count + num_indices);
    
	int first_index = verts_buffer.count;
	for(int i = 0; i < num_indices; i++) array_add(&indices_buffer, indices[i]);
	for(int i = 0; i < num_verts; i++) array_add(&verts_buffer, verts[i]);
    
	return first_index;
}

void r_render_box(Vec2 position, bool use_camera_position, Vec2 size, bool centered, Vec4 colour, bool wireframe) {
	float x0 = position.x;
	float y0 = position.y;
	float x1 = position.x + size.x;
	float y1 = position.y + size.y;
    if(centered) {
        float hw = size.x * 0.5f;
        float hh = size.y * 0.5f;
        x0 = position.x - hw;
        y0 = position.y - hh;
        x1 = position.x + hw;
        y1 = position.y + hh;
    }
    
    Vertex verts[4];
	verts[0].position = Vec3(x0, y0, 0);
	verts[1].position = Vec3(x0, y1, 0);
	verts[2].position = Vec3(x1, y1, 0);
	verts[3].position = Vec3(x1, y0, 0);
	verts[0].uv = Vec2(0, 0);
	verts[1].uv = Vec2(0, 1);
	verts[2].uv = Vec2(1, 1);
	verts[3].uv = Vec2(1, 0);
	verts[0].colour = colour;
	verts[1].colour = colour;
	verts[2].colour = colour;
	verts[3].colour = colour;
    
	unsigned int indicies[6];
	indicies[0] = 0;
	indicies[1] = 1;
	indicies[2] = 2;
	indicies[3] = 0;
	indicies[4] = 3;
	indicies[5] = 2;
    
    Render_Command rc;
    rc.type = RC_QUAD;
    rc.first_index = add_verts(verts, 4, indicies, 6);
    rc.program = &prog_textured;
    rc.use_camera_position = use_camera_position;
    rc.wireframe = wireframe;
    array_add(&r_commands, rc);
}

void r_render_texture(Texture *texture, Vec2 position, bool use_camera_position, Vec4 colour, Vec2 size, bool centered) {
    if(size.x == -1) size.x = texture->width;
    if(size.y == -1) size.y = texture->height;
    
    float u_ratio = size.x / texture->width;
    float v_ratio = size.y / texture->height;
    
    float x0, y0, x1, y1;
    if(centered) {
        x0 = position.x - size.x / 2;
        y0 = position.y - size.y / 2;
        x1 = position.x + size.x / 2;
        y1 = position.y + size.y / 2;
    } else {
        x0 = position.x;
        y0 = position.y;
        x1 = position.x + size.x;
        y1 = position.y + size.y;
    }
    
	Vertex verts[4];
	verts[0].position = Vec3(x0, y0, 0);
	verts[1].position = Vec3(x0, y1, 0);
	verts[2].position = Vec3(x1, y1, 0);
	verts[3].position = Vec3(x1, y0, 0);
	verts[0].uv = Vec2(0, 0);
	verts[1].uv = Vec2(0, 1 * v_ratio);
	verts[2].uv = Vec2(1 * u_ratio, 1 * v_ratio);
	verts[3].uv = Vec2(1 * u_ratio, 0);
	verts[0].colour = colour;
	verts[1].colour = colour;
	verts[2].colour = colour;
	verts[3].colour = colour;
    
	unsigned int indicies[6];
	indicies[0] = 0;
	indicies[1] = 1;
	indicies[2] = 2;
	indicies[3] = 0;
	indicies[4] = 3;
	indicies[5] = 2;
    
    int first_index = add_verts(verts, 4, indicies, 6);
    
	Render_Command rc;
	rc.type = RC_TEXTURE;
	rc.first_index = first_index;
    rc.program = &prog_textured;
	rc.texture.texture = texture;
    rc.use_camera_position = use_camera_position;
    array_add(&r_commands, rc);
}

void r_render_string(Vec2 position, bool use_camera_position, const char *text, Vec4 colour, Font *font, float wrap) {
	if (!font) {
		font = load_font("data/fonts/consolas.ttf", 16);
	}
    
    position.y += font->size;
    
    int length = (int)strlen(text);
    for(int i = 0; i < length; i++) {
        if(text[i] < 32 || text[i] >= 128) continue;
        
        int c = text[i] - 32;
        
        stbtt_aligned_quad q;
        stbtt_GetPackedQuad(font->glyphs, font->texture->width, font->texture->height, c, &position.x, &position.y, &q, 1);
        
        Vertex verts[4];
    	verts[0].position = Vec3(q.x0, q.y0, 0);
    	verts[1].position = Vec3(q.x0, q.y1, 0);
    	verts[2].position = Vec3(q.x1, q.y1, 0);
    	verts[3].position = Vec3(q.x1, q.y0, 0);
    	verts[0].uv = Vec2(q.s0, q.t0);
    	verts[1].uv = Vec2(q.s0, q.t1);
    	verts[2].uv = Vec2(q.s1, q.t1);
    	verts[3].uv = Vec2(q.s1, q.t0);
    	verts[0].colour = colour;
    	verts[1].colour = colour;
    	verts[2].colour = colour;
    	verts[3].colour = colour;
        
    	unsigned int indicies[6];
    	indicies[0] = 0;
    	indicies[1] = 1;
    	indicies[2] = 2;
    	indicies[3] = 0;
    	indicies[4] = 3;
        indicies[5] = 2;
        
        Render_Command rc;
        rc.type = RC_TEXTURE;
        rc.first_index = add_verts(verts, 4, indicies, 6);
        rc.program = &prog_text;
        rc.texture.texture = font->texture;
        rc.use_camera_position = use_camera_position;
        array_add(&r_commands, rc);
    }
}

void r_render_string_format(Vec2 position, bool use_camera_position, Vec4 colour, Font *font, float wrap, const char *text, ...) {
	//@todo: make this better?
	constexpr int max_string_length = 1024;
    
	va_list argptr;
	char message[max_string_length];
    
	assert(strlen(text) < max_string_length, "");
    
	va_start(argptr, text);
	vsnprintf_s(message, 1024, text, argptr);
	va_end(argptr);
    
	r_render_string(position, use_camera_position, message, colour, font, wrap);
}

// if you don't want to think about colour/font/wrap the default values will be same as those in render_string
void r_render_string_format_lazy(Vec2 position, bool use_camera_position, const char *text, ...) {
	//@todo: make this better?
	constexpr int max_string_length = 1024;
    
	va_list argptr;
	char message[max_string_length];
    
	assert(strlen(text) < max_string_length, "");
    
	va_start(argptr, text);
	vsnprintf_s(message, 1024, text, argptr);
	va_end(argptr);
    
	r_render_string(position, use_camera_position, message);
}

void debug_string(const char *text, ...) {
	//@todo: make this better?
	constexpr int max_string_length = 1024;
    
	va_list argptr;
	char message[max_string_length];
    
	va_start(argptr, text);
	vsnprintf_s(message, 1024, text, argptr);
	va_end(argptr);
    
	r_render_string_format_lazy(debug_string_position, false, message);
	debug_string_position.y += get_string_size(null, message).y;
}

void hotload_vertex_callback(const char *filename, void *data) {
	Program *program = (Program *)data;
	
	glDetachShader(program->program_object, program->vertex_object);
	glDeleteShader(program->vertex_object);
	program->vertex_object = load_shader(program->vertex_filename, GL_VERTEX_SHADER);
	glAttachShader(program->program_object, program->vertex_object);
	glLinkProgram(program->program_object);
}

void hotload_frag_callback(const char *filename, void *data) {
	Program *program = (Program *)data;
	
	glDetachShader(program->program_object, program->frag_object);
	glDeleteShader(program->frag_object);
	program->frag_object = load_shader(program->frag_filename, GL_FRAGMENT_SHADER);
	glAttachShader(program->program_object, program->frag_object);
	glLinkProgram(program->program_object);
}	

void setup_program(Program *program, const char *vertex_filename, const char *frag_filename) {
	program->vertex_filename = vertex_filename;
	program->frag_filename = frag_filename;
	load_program(program);
    hotload_add_file(program->vertex_filename, program, hotload_vertex_callback);
    hotload_add_file(program->frag_filename, program, hotload_frag_callback);
}

void r_execute_commands() {
	assert(verts_buffer.count < MAX_VERTICIES, "ran out of verts!");
	assert(indices_buffer.count < MAX_INDICES, "ran out of indices!");
	glBufferSubData(GL_ARRAY_BUFFER, 0, verts_buffer.count * sizeof(Vertex), verts_buffer.data);
	glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, indices_buffer.count * sizeof(unsigned int), indices_buffer.data);
    
	for(int i = 0; i < r_commands.count; i++) {
        Render_Command *rc = &r_commands[i];
        
        glUseProgram(rc->program->program_object);
        
        int time_loc = glGetUniformLocation(rc->program->program_object, "time");
        glUniform1f(time_loc, current_time);
        
        transformation_matrix = create_identity_matrix();
        projection_matrix = create_ortho_matrix(0, window_width, window_height, 0);
        
        int projection_matrix_loc = glGetUniformLocation(rc->program->program_object, "projection_matrix");
        int transformation_matrix_loc = glGetUniformLocation(rc->program->program_object, "transformation_matrix");
        glUniformMatrix4fv(transformation_matrix_loc, 1, GL_FALSE, transformation_matrix.e);
        glUniformMatrix4fv(projection_matrix_loc, 1, GL_FALSE, projection_matrix.e);
        
        int sampler_loc = glGetUniformLocation(rc->program->program_object, "diffuse_texture");
        glUniform1i(sampler_loc, 0);
        glActiveTexture(GL_TEXTURE0);
        
        switch(rc->type) {
            case RC_TEXTURE:
            glBindTexture(GL_TEXTURE_2D, rc->texture.texture->id);
            break;
            case RC_QUAD:
            glBindTexture(GL_TEXTURE_2D, default_texture->id);
            break;
        }
        
        Vec2 cam = camera_position;
        
        int zoom_level_loc = glGetUniformLocation(rc->program->program_object, "zoom_level");
        if(rc->use_camera_position) {
            glUniform1i(zoom_level_loc, zoom_level);
        } else {
            glUniform1i(zoom_level_loc, 0);
        }
        
        int camera_position_loc = glGetUniformLocation(rc->program->program_object, "camera_position");
        if(rc->use_camera_position) glUniform2f(camera_position_loc, cam.x, cam.y);
        else glUniform2f(camera_position_loc, 0, 0);
        
        if(rc->wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDrawElementsBaseVertex(GL_TRIANGLES, 6, GL_UNSIGNED_INT, null, rc->first_index);
        if(rc->wireframe) glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
    
	r_commands.count = 0;
	verts_buffer.count = 0;
	indices_buffer.count = 0;
    debug_string_position.y = 20;
}

void r_render_frame() {
    if(should_clear) {
		glClearColor(clear_colour.x, clear_colour.y, clear_colour.z, clear_colour.w);
		glClear(GL_COLOR_BUFFER_BIT);
	}
    
	glClear(GL_DEPTH_BUFFER_BIT);
    
	r_execute_commands();
    
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    
	SDL_GL_SwapWindow(sdl_window);
    
	glUseProgram(0);
}

void opengl_debug_output_to_file(unsigned int source, unsigned int type, unsigned int id, unsigned int severity, const char* message, bool should_print) {
    FILE* f = null;
    fopen_s(&f, "data/opengl.log", "a");
    if(f) {
        char debSource[16] = {};
        char debType[20] = {};
        char debSev[5] = {};
        
        switch(source) {
			case GL_DEBUG_SOURCE_API_ARB:
            strcpy_s(debSource, "OpenGL");
            break;
			case GL_DEBUG_SOURCE_SHADER_COMPILER_ARB:
            strcpy_s(debSource, "Shader Compiler");
            break;
			case GL_DEBUG_SOURCE_THIRD_PARTY_ARB:
            strcpy_s(debSource, "Third Party");
            break;
			case GL_DEBUG_SOURCE_APPLICATION_ARB:
            strcpy_s(debSource, "Application");
            break;
			case GL_DEBUG_SOURCE_OTHER_ARB:
            strcpy_s(debSource, "Other");
            break;
			case GL_DEBUG_SOURCE_WINDOW_SYSTEM_ARB:
            strcpy_s(debSource, "Windows");
            break;
        }
        
        switch(type) {
			case GL_DEBUG_TYPE_ERROR_ARB:
            strcpy_s(debType, "Error");
            break;
			case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR_ARB:
            strcpy_s(debType, "Deprecated behavior");
            break;
			case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR_ARB:
            strcpy_s(debType, "Undefined behavior");
            break;
			case GL_DEBUG_TYPE_PORTABILITY_ARB:
            strcpy_s(debType, "Portability");
            break;
			case GL_DEBUG_TYPE_PERFORMANCE_ARB:
            strcpy_s(debType, "Performance");
            break;
			case GL_DEBUG_TYPE_OTHER_ARB:
            strcpy_s(debType, "Other");
            break;
        }
        
        switch(severity) {
			case GL_DEBUG_SEVERITY_HIGH_ARB:
            strcpy_s(debSev, "High");
            break;
			case GL_DEBUG_SEVERITY_MEDIUM_ARB:
            strcpy_s(debSev, "Medium");
            break;
			case GL_DEBUG_SEVERITY_LOW_ARB:
            strcpy_s(debSev, "Low");
            break;
        }
        
        if(should_print) {
            printf("Source:%s\tType:%s\tID:%d\tSeverity:%s\tMessage:%s\n", debSource, debType, id, debSev, message);
        }
        fprintf(f, "Source:%s\tType:%s\tID:%d\tSeverity:%s\tMessage:%s\n", debSource, debType, id, debSev, message);
        fclose(f);
    }
}

void opengl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	if(id == 131185) {
		/* Message:Buffer detailed info: Buffer object 1 (bound to GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB (0), GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB (1), GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING_ARB (2), and GL_ARRAY_BUFFER_ARB, usage hint is GL_STREAM_DRAW) will use VIDEO memory as the source for buffer object operations.*/
		return;
	}
    
    opengl_debug_output_to_file(source, type, id, severity, message, severity == GL_DEBUG_SEVERITY_MEDIUM_ARB || severity == GL_DEBUG_SEVERITY_HIGH_ARB);
}