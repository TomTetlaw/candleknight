#ifndef FONT_H
#define FONT_H

struct Font {
    const char *filename = null;
    int size = 0;
    Texture *texture = nullptr;
    stbtt_fontinfo info;
    stbtt_packedchar glyphs[127];
    float scale = 0.0f;
    unsigned char *data = null;
};

void font_shutdown();
Font *load_font(const char *filename, int point_size);

Vec2 get_string_size(Font *font, const char *string);

#endif