#include "includes.h"

internal Array<Texture *> textures;

internal void unload_texture(Texture *texture) {
    glDeleteTextures(1, &texture->id);
}

void tex_init() {
}

void tex_shutdown() {
    for(int i = 0; i < textures.count; i++) unload_texture(textures[i]);
}

void create_texture_data(Texture *texture, const unsigned char *data, int width, int height) {
	unsigned int t;
	glGenTextures(1, &t);
	glBindTexture(GL_TEXTURE_2D, t);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, (void *)data);
    
	texture->width = width;
	texture->height = height;
    texture->size = Vec2(width, height);
	texture->id = t;
}

void create_texture_data_from_surface(Texture *texture, SDL_Surface *surf) {
	if (!surf) {
		return;
	}
    
	if (surf->format->format != SDL_PIXELFORMAT_RGBA8888) {
		SDL_PixelFormat format = { 0 };
		format.BitsPerPixel = 32;
		format.BytesPerPixel = 4;
		format.format = SDL_PIXELFORMAT_RGBA8888;
		format.Rshift = 0;
		format.Gshift = 8;
		format.Bshift = 16;
		format.Ashift = 24;
		format.Rmask = 0xff << format.Rshift;
		format.Gmask = 0xff << format.Gshift;
		format.Bmask = 0xff << format.Bshift;
		format.Amask = 0xff << format.Ashift;
        
		SDL_Surface *newSurf = SDL_ConvertSurface(surf, &format, 0);
		SDL_FreeSurface(surf);
		surf = newSurf;
	}
    
	unsigned int t;
	glGenTextures(1, &t);
	glBindTexture(GL_TEXTURE_2D, t);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, surf->w, surf->h, 0, GL_RGBA, GL_UNSIGNED_BYTE, surf->pixels);
    
	texture->width = surf->w;
	texture->height = surf->h;
    texture->size = Vec2(surf->w, surf->h);
	texture->id = t;
    
    SDL_FreeSurface(surf);
}

bool load_texture_base(const char *filename, uint *id, int *width, int *height) {
    unsigned char *pixels = stbi_load(filename, width, height, 0, STBI_rgb_alpha);
    if(pixels == null) {
        assert(false, "Could not load image %s: %s", filename, stbi_failure_reason());
        return false;
    }
    
    glGenTextures(1, id);
	glBindTexture(GL_TEXTURE_2D, *id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    if(!strcmp(filename, "data/textures/player.png") || !strcmp(filename, "data/textures/player_lit.png")) {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, *width, *height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    return true;
}

void texture_hotload_callback(const char *filename, void *data) {
	Texture *texture = (Texture *)data;
	glDeleteTextures(1, &texture->id);
	load_texture_base(filename, &texture->id, &texture->width, &texture->height);
}

Texture *load_texture(const char *filename) {
    for(int i = 0; i < textures.count; i++) {
        if(textures[i]->filename == filename) return textures[i];
    }
    
    uint id = -1;
    int width = 0, height = 0;
    if(!load_texture_base(filename, &id, &width, &height)) return null;
    
    Texture *texture = alloc(Texture);
    texture->id = id;
    texture->width = width;
    texture->height = height;
    texture->size = Vec2(width, height);
    strcpy(texture->filename, filename);
    array_add(&textures, texture);
    hotload_add_file(filename, (void *)texture, texture_hotload_callback);
    
    return texture;
}

Texture *create_texture(const char *name, const unsigned char *data, int width, int height) {
	Texture *texture = alloc(Texture);
	strcpy(texture->filename, name);
	array_add(&textures, texture);
	create_texture_data(texture, data, width, height);
    return texture;
}

Texture *create_texture_from_surface(const char *name, SDL_Surface *surface) {
	Texture *texture = alloc(Texture);
	strcpy(texture->filename, name);
	array_add(&textures, texture);
	create_texture_data_from_surface(texture, surface);
    return texture;
}

Texture *create_bitmap(const char *name, const unsigned char *data, int width, int height) {
    Texture *texture = alloc(Texture);
    strcpy(texture->filename, name);
    array_add(&textures, texture);
    texture->width = width;
    texture->height = height;
    texture->size = Vec2(width, height);
    glGenTextures(1, &texture->id);
    glBindTexture(GL_TEXTURE_2D, texture->id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, data);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    return texture;
}