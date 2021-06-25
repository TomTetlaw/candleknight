#ifndef EDITOR_H
#define EDITOR_H

void editor_init();
void editor_shutdown();
void editor_update();
void editor_render();

void editor_do_ui();

bool editor_process_event(SDL_Event *event);

void editor_create_pe_data();

#endif
