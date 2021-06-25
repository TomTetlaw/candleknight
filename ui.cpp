#include "includes.h"

bool ui_wants_mouse_input = false;

struct UI_Window {
    char name[1024] = {};
    
    Vec2 position;    
    float current_y = 0;
    float width = 0;
};

String_Table<UI_Window> windows;
Array<UI_Style> styles;

#define current_style (&styles[styles.count - 1])

UI_Window *current_window = null;

UI_Style default_style;

enum UI_DCType {
    UIDC_LABEL,
    UIDC_HEADER,
    UIDC_WINDOW,
    UIDC_BUTTON,
    UIDC_FIELD,
};

struct UIDC_Label {
    char text[1024] = {};
    Vec2 position;
};

struct UIDC_Window {
    UI_Window *window;
};

struct UIDC_Button {
    char text[1024] = {};
    Vec2 position;
    Vec2 size;
};

struct UIDC_Header {
    char text[1024] = {};
    Vec2 position;
    Vec2 size;
    Vec2 arrow_position;
};

struct UIDC_Field {
    char *text = null;
    int cursor = 0;
    Vec2 position;
    Vec2 size;
};

struct UI_Draw_Command {
    UI_DCType type;
    
    UI_Style style;
    UIDC_Window window;
    UIDC_Button button;
    UIDC_Label label;
    UIDC_Header header;
    UIDC_Field field;
};

internal Array<UI_Draw_Command> ui_commands;

struct UI_Click {
    Vec2 position;
};

Array<UI_Click> clicks;
Array<SDL_Scancode> key_presses;

Text_Edit *focused_te = null;

void ui_issue_draw_commands() {
    ui_wants_mouse_input = false;
    
    for(int i = 0; i < ui_commands.count; i++) {
        UI_Draw_Command *command = &ui_commands[i];
        switch(command->type) {
            case UIDC_WINDOW:
            if(point_box_intersection(cursor_position, 
                                      command->window.window->position, 
                                      Vec2(command->window.window->width, command->window.window->current_y))) 
                ui_wants_mouse_input = true;
            
            r_render_box(command->window.window->position, false, Vec2(command->window.window->width, command->window.window->current_y), false, command->style.window_box_colour);
            break;
            case UIDC_LABEL: {
                r_render_string(command->label.position, false, command->label.text, command->style.label_text_colour, command->style.label_text_font);
            }
            case UIDC_BUTTON: {
                Vec4 button_box_colour = command->style.button_box_colour;
                if(point_box_intersection(cursor_position, command->button.position, command->button.size)) {
                    button_box_colour = command->style.button_box_highlight_colour;
                }
                r_render_box(command->button.position, false, command->button.size, false, button_box_colour);
                r_render_string(command->button.position, false, command->button.text, command->style.button_text_colour, command->style.button_text_font);
            } break;
            case UIDC_HEADER: {
                r_render_box(command->header.position, false, command->header.size, false, command->style.header_box_colour);
                r_render_string(command->header.position, false, command->header.text, command->style.header_text_colour, command->style.header_text_font);
                r_render_texture(command->style.header_arrow, command->header.arrow_position, true, Vec4(1,1,1,1), Vec2(-1,-1), false);
            } break;
            case UIDC_FIELD: {
                r_render_box(command->field.position, false, command->field.size, false, command->style.field_box_colour);
                r_render_string(command->field.position, false, command->field.text, command->style.field_text_colour, command->style.field_text_font);
            } break;
        }
    }
    
    ui_commands.count = 0;
    clicks.count = 0;
    key_presses.count = 0;
}

void ui_process_event(SDL_Event *event) {
    if(event->type == SDL_MOUSEBUTTONDOWN && event->button.button == SDL_BUTTON_LEFT) {
        UI_Click click;
        click.position = cursor_position;
        array_add(&clicks, click);
    }
    
    if(event->type == SDL_KEYDOWN) {
        array_add(&key_presses, event->key.keysym.scancode);
    }
}

void ui_init() {
    st_init(&windows);
    
    default_style.header_arrow = load_texture("data/textures/header_arrow.png");
    array_add(&styles, default_style);
}

void ui_shutdown() {
    st_free(&windows);
}

void ui_begin(Vec2 position, const char *name, bool no_label) {
    assert(current_window == null, "Call ui_end() first.");
    
    current_window = st_find_or_add(&windows, name);
    strcpy(current_window->name, name);
    current_window->position = position;
    current_window->width = 0;
    current_window->current_y = 0;
    
    UI_Draw_Command command;
    command.type = UIDC_WINDOW;
    command.style = *current_style;
    command.window.window = current_window;
    array_add(&ui_commands, command);
    
    if(!no_label) ui_label(name);
}

void ui_end() {
    current_window = null;
}

void ui_seperator() {
    Vec2 string_size = get_string_size(current_style->button_text_font, "Hey there");
    current_window->current_y += string_size.y;
}

void ui_label(const char *text) {
    if(!current_window) {
        assert(false, "No current window.");
        return;
    }
    
    Vec2 string_size = get_string_size(current_style->button_text_font, text);
    
    UI_Draw_Command command;
    command.type = UIDC_LABEL;
    command.style = *current_style;
    strcpy(command.label.text, text);
    command.label.position = current_window->position + Vec2(0, current_window->current_y);
    array_add(&ui_commands, command);
    
    current_window->current_y += string_size.y;
    current_window->width = max(current_window->width, string_size.x);
}

bool ui_header(const char *text, bool *open) {
    if(!current_window) {
        assert(false, "No current window.");
        return false;
    }
    
    Vec2 string_size = get_string_size(current_style->button_text_font, text);
    Vec2 arrow_size = current_style->header_arrow->size;
    
    UI_Draw_Command command;
    command.type = UIDC_HEADER;
    command.style = *current_style;
    strcpy(command.header.text, text);
    command.header.position = current_window->position + Vec2(0, current_window->current_y);
    command.header.size = string_size;
    command.header.arrow_position = command.header.position + string_size - Vec2(0, arrow_size.y);
    array_add(&ui_commands, command);
    
    current_window->current_y += string_size.y;
    current_window->width = max(current_window->width, string_size.x);
    
    for(int i = 0; i < clicks.count; i++) {
        if(point_box_intersection(clicks[i].position, command.header.arrow_position, arrow_size, false)) *open = !*open;
    }
    
    return *open;
}

bool ui_button(const char *text) {
    if(!current_window) {
        assert(false, "No current window.");
        return false;
    }
    
    Vec2 string_size = get_string_size(current_style->button_text_font, text);
    
    UI_Draw_Command command;
    command.type = UIDC_BUTTON;
    command.style = *current_style;
    strcpy(command.button.text, text);
    command.button.position = current_window->position + Vec2(0, current_window->current_y);
    command.button.size = string_size;
    array_add(&ui_commands, command);
    
    current_window->current_y += string_size.y;
    current_window->width = max(current_window->width, string_size.x);
    
    for(int i = 0; i < clicks.count; i++) {
        if(point_box_intersection(clicks[i].position, command.button.position, command.button.size)) return true;
    }
    
    return false;
}

bool ui_float_field(Text_Edit *te, const char *name, float *value) {
    Vec2 string_size = get_string_size(current_style->field_text_font, te->line);
    
    UI_Draw_Command command;
    command.type = UIDC_FIELD;
    command.style = *current_style;
    command.field.size = string_size;
    command.field.text = te->line;
    command.field.cursor = te->cursor;
    command.field.position = current_window->position + Vec2(0, current_window->current_y);
    array_add(&ui_commands, command);
    
    current_window->current_y += string_size.y;
    current_window->width = max(current_window->width, string_size.x);
    
    for(int i = 0; i < clicks.count; i++) {
        if(point_box_intersection(clicks[i].position, command.field.position, command.field.size)) {
            focused_te = te;
        }
    }
    
    if(te == focused_te) {
        bool changed = false;
        for(int i = 0; i < key_presses.count; i++) {
            if(tedit_handle_key(te, key_presses[i])) *value = atof(te->line);
            changed = true;
        }
        return changed;
    }
    
    return false;
}

void ui_copy_default_style(UI_Style *dest) {
    memcpy(dest, &default_style, sizeof(UI_Style));
}

void ui_push_style(UI_Style *style) {
    array_add(&styles, *style);
}

void ui_pop_style() {
    styles.count--;
    if(styles.count < 1) styles.count = 1;
}