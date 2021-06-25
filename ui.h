#ifndef UI_H
#define UI_H

struct UI_Style {
    Vec4 window_box_colour = Vec4(39 / 255.0f, 121 / 255.0f, 204 / 255.0f, 1.0f);
    
    Font *label_text_font = null;
    Vec4 label_text_colour = Vec4(1, 1, 1, 1);
    
    Font *button_text_font = null;
    Vec4 button_box_colour = Vec4(43 / 255.0f, 149 / 255.0f, 255 / 255.0f, 1.0f);
    Vec4 button_box_highlight_colour = Vec4(1, 1, 1, 1.0f);
    Vec4 button_text_colour = Vec4(1, 1, 1, 1);
    
    Font *header_text_font = null;
    Texture *header_arrow = null;
    Vec4 header_box_colour = Vec4(43 / 255.0f, 149 / 255.0f, 255 / 255.0f, 1.0f);
    Vec4 header_text_colour = Vec4(1,1,1,1);
    
    Font *field_text_font = null;
    Vec4 field_box_colour = Vec4(43 / 255.0f, 149 / 255.0f, 255 / 255.0f, 1.0f);
    Vec4 field_text_colour = Vec4(1,1,1,1);
};

void ui_copy_default_style(UI_Style *dest);
void ui_push_style(UI_Style *style);
void ui_pop_style();

void ui_init();
void ui_shutdown();

void ui_begin(Vec2 position, const char *name, bool no_label = false);
void ui_end();

void ui_seperator();
void ui_label(const char *text);
bool ui_header(const char *text, bool *open);
bool ui_button(const char *text);

bool ui_float_field(Text_Edit *te, const char *name, float *value);

void ui_issue_draw_commands();

void ui_process_event(SDL_Event *event);
extern bool ui_wants_mouse_input;

#endif
