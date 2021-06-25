#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#define line_size 1024

struct Text_Edit {
    int cursor = 0;
    char line[line_size] = {};
};

bool tedit_handle_key(Text_Edit *te, SDL_Scancode scancode);

#endif
