#include "includes.h"

bool tedit_handle_key(Text_Edit *te, SDL_Scancode scancode) {
    char ch = '\0';
    if(input_translate_scancode(scancode, &ch)) {
        char input_copy[line_size] = {0};
        memcpy(input_copy, te->line, line_size);
        for(int i = 0; i < line_size; i++) {
            if(i < te->cursor) te->line[i] = input_copy[i];
            else if(i > te->cursor) te->line[i] = input_copy[i-1];
            else if(i == te->cursor) te->line[i] = ch;
        }
        
        te->cursor += 1;
    } else {
        if(scancode == SDL_SCANCODE_LEFT) {
            te->cursor -= 1;
        }
        else if(scancode == SDL_SCANCODE_RIGHT) {
            te->cursor += 1;
        } else if(scancode == SDL_SCANCODE_RETURN) {
            return true;
        } else if(scancode == SDL_SCANCODE_DELETE) {
            char input_copy[line_size] = {0};
            memcpy(input_copy, te->line, line_size);
            for(int i = 0; i < line_size - 1; i++) {
                if(i < te->cursor) te->line[i] = input_copy[i];
                else if(i >= te->cursor) te->line[i] = input_copy[i + 1];
            }
        } else if(scancode == SDL_SCANCODE_BACKSPACE) {
            if(te->cursor > 0) {
                char input_copy[line_size] = {0};
                memcpy(input_copy, te->line, line_size);
                te->cursor -= 1;
                for(int i = 0; i < line_size - 1; i++) {
                    if(i < te->cursor) te->line[i] = input_copy[i];
                    else if(i >= te->cursor) te->line[i] = input_copy[i + 1];
                }
            }  
        }
    }
    
    if(te->cursor < 0) te->cursor = 0;
    if(te->cursor > strlen(te->line)) te->cursor = strlen(te->line);
    
    return false;
}