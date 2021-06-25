#include "includes.h"

Array<Font *> fonts;

void font_shutdown() {
	for (int i = 0; i < fonts.count; i++) {
        free(fonts[i]->data);
        free(fonts[i]);
    }
}

Font *load_font(const char *filename, int size) {
	Font *font_info = nullptr;
    
	for (int i = 0; i < fonts.count; i++) {
		font_info = fonts[i];
		if (!strcmp(font_info->filename, filename) && font_info->size == size) {
			return font_info;
		}
	}
    
    // need to keep the data around until shutdown because font_info->info holds pointers into it 
    Load_File_Result result = load_entire_file(filename);
    if(!result.data) return null;
    
    font_info = alloc(Font);
    font_info->data = result.data;
    font_info->filename = filename;
    
    int bitmap_width = 512;
    int bitmap_height = 50;
    unsigned char *bitmap = allocn(unsigned char, bitmap_width * bitmap_height);
    defer { free(bitmap); };
    
    stbtt_InitFont(&font_info->info, result.data, stbtt_GetFontOffsetForIndex(result.data, 0));
    font_info->scale = stbtt_ScaleForPixelHeight(&font_info->info, size);
    
    stbtt_pack_context spc;
    stbtt_PackBegin(&spc, bitmap, bitmap_width, bitmap_height, bitmap_width, 1, null);
    stbtt_PackFontRange(&spc, result.data, 0, size, 32, 96, font_info->glyphs);
    stbtt_PackEnd(&spc);
    
    font_info->texture = create_bitmap(font_info->filename, bitmap, bitmap_width, bitmap_height);
    
    font_info->size = size;
    array_add(&fonts, font_info);
    
    return font_info;
}

Vec2 get_string_size(Font *font, const char *string) {
    if(!font) font = load_font("data/fonts/consolas.ttf", 16);    
    
    int advance = 0;
    int total = 0;
    int lsb = 0;
    int ascent = 0;
    int descent = 0;
    int line_gap = 0;
    
    stbtt_GetFontVMetrics(&font->info, &ascent, &descent, &line_gap);
    while(*string++) {
        stbtt_GetCodepointHMetrics(&font->info, *string, &advance, &lsb);
        total += advance;
    }
    
    Vec2 size;
    size.x = total * font->scale;
    size.y = (ascent - descent + line_gap) * font->scale;
    return size;
}