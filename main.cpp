#include "includes.h"

int main(int argc, char *argv[]) {
    if(SDL_Init(SDL_INIT_EVERYTHING) < 0) fatal_error("ERROR", "Could not initialize SDL2: %s", SDL_GetError());
    
    sdl_window = SDL_CreateWindow("Game", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL);
    if(!sdl_window) fatal_error("ERROR", "Could not create window: %s", SDL_GetError());
    
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, SDL_GL_CONTEXT_DEBUG_FLAG);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GLContext context = SDL_GL_CreateContext(sdl_window);
    
    glewInit();
    
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    ImGui::StyleColorsDark();
    ImGui_ImplSDL2_InitForOpenGL(sdl_window, context);
    ImGui_ImplOpenGL3_Init(null);
    
    r_init();
    tex_init();
    hotload_init();
    ent_init();
    editor_init();
    physics_init();
    
    SDL_Event event;
    bool should_quit = false;
    float prev_time = SDL_GetTicks() / 1000.0f;
    while(!should_quit) {
        hotload_check_files_non_blocking();
        
        float now = SDL_GetTicks() / 1000.0f;
        delta_time = (now - prev_time) * (int)!is_paused;
        current_time = now;
        prev_time = current_time;
        if(!is_paused) game_time += delta_time;
        
        int x=0, y=0; SDL_GetMouseState(&x, &y);
        cursor_position = Vec2(x, y);
        
        while(SDL_PollEvent(&event)) {
            input_collect_control_keys(&event);
            
            ImGui_ImplSDL2_ProcessEvent(&event);
            if(io.WantCaptureMouse || io.WantCaptureKeyboard) continue;
            
            if(!editor_process_event(&event)) continue;
            
            ent_handle_event(&event);
            
            switch(event.type) {
                case SDL_QUIT:
                quit();
                break;
            }
        }
        
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL2_NewFrame(sdl_window);
        ImGui::NewFrame();
        
        editor_do_ui();
        editor_update();
        
        physics_step_world(delta_time);
        
        ent_update();
        ent_render();
        
        editor_render();
        
        r_render_frame();
    }
    
    return 0;
}