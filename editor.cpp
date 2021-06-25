#include "includes.h"

enum Context_Type {
    CONTEXT_CREATE_ENTITY,
    CONTEXT_EDIT_ENTITY,
};

bool context_open = false;
Vec2 context_position;
Context_Type context_type;
Entity *selected_entity = null;
Entity *copied_entity = null;

bool dragging = false;
Vec2 drag_origin;

char level_list[1024][64] = {};
const char *current_level = null;
int current_level_num = 0;
int num_levels = 0;

enum Dragging_Axis_Mode {
    DA_POSITION,
    DA_SIZE,
};

Texture *move_x_axis = null;
Texture *move_y_axis = null;
Dragging_Axis_Mode da_mode = DA_POSITION;

bool dragging_x = false;
bool dragging_y = false;
Vec2 dragging_x_pos = Vec2(50, 0);
Vec2 dragging_y_pos = Vec2(0, -50);

struct Undo_Record {
    int index = -1;
    int parity = -1;
    
    Entity *get_entity() {
        if(!entities.filled[index]) return null;
        if(entities[index].parity != parity) return null;
        return &entities[index];
    }
    
    void apply() { 
        Entity *entity = get_entity();
        if(entity) apply(entity);
    }
    
    virtual void apply(Entity *entity) {}
};

struct Undo_Record_Position : public Undo_Record {
    Undo_Record_Position(Vec2 old_position) {
        this->old_position = old_position;
    }
    
    Vec2 old_position;
    
    void apply(Entity *entity) {
        entity->po.position = old_position;
    }
};

struct Undo_Record_Size : public Undo_Record {
    Undo_Record_Size(Vec2 old_size) {
        this->old_size = old_size;
    }
    
    Vec2 old_size;
    
    void apply(Entity *entity) {
        entity->po.size = old_size;
    }
};

struct Undo_Record_Create_Entity : public Undo_Record {
    void apply(Entity *entity) {
        entity->remove_me = true;
        if(selected_entity == entity) selected_entity = null;
    }
};

Array<Undo_Record *> undo_records;
Vec2 original_entity_position;
Vec2 original_entity_size;

void record_undo(Entity *entity, Undo_Record *record) {
    record->index = entity->index;
    record->parity = entity->parity;
    array_add(&undo_records, record);
}

void undo() {
    if(undo_records.count <= 0) return;
    
    Undo_Record *record = undo_records[undo_records.count - 1];
    record->apply();
    delete record;
    undo_records.count -= 1;
}

void collect_level_list() {
    num_levels = 0;
    
    tinydir_dir dir;
    tinydir_open(&dir, "data/levels");
    
    while(dir.has_next) {
        tinydir_file file;
        tinydir_readfile(&dir, &file);
        if(file.is_dir && strcmp(file.name, ".") && strcmp(file.name, "..")) {
            strcpy(level_list[num_levels], file.name);
            num_levels++;
        }
        tinydir_next(&dir);
    }
    
    tinydir_close(&dir);
}

void editor_init() {
    collect_level_list();
    
    current_level = level_list[0];
    load_entities(current_level);
    
    move_x_axis = load_texture("data/textures/editor_x.png");
    move_y_axis = load_texture("data/textures/editor_y.png");
}

void editor_shutdown() {
    for(int i = 0; i < undo_records.count; i++) delete undo_records[i];
}

void editor_update() {
}

void editor_render() {
    if(selected_entity) {
        r_render_box(selected_entity->po.position, true, Vec2(selected_entity->po.size.x+5, selected_entity->po.size.y+5), true, Vec4(sin(current_time)*sin(current_time), 1, 1, 1), true);
        
        r_render_texture(move_x_axis, selected_entity->po.position + dragging_x_pos, true, Vec4(1, 1, 1, 1));
        r_render_texture(move_y_axis, selected_entity->po.position + dragging_y_pos, true, Vec4(1, 1, 1, 1));
    }
    
    debug_string("f5 - Enter/Exit play mode, f6 - Restart level");
    debug_string("w - edit position, r - edit size");
    debug_string("w/a/s/d - move, rmb - switch flame position, space - jump");
}

void do_property(Property *prop, uchar *base) {
    switch(prop->type) {
        case PROP_INT:
        ImGui::InputScalar(prop->name, ImGuiDataType_S32, (int *)(base + prop->offset));
        break;
        
        case PROP_UINT:
        ImGui::InputScalar(prop->name, ImGuiDataType_U32, (uint *)(base + prop->offset));
        break;
        
        case PROP_FLOAT:
        ImGui::InputFloat(prop->name, (float *)(base + prop->offset));
        break;
        
        case PROP_BOOL:
        ImGui::Checkbox(prop->name, (bool *)(base + prop->offset));
        break;
        
        case PROP_TEXTURE: 
        ImGui::Text("%s = %s", prop->name, (*((Texture **)(base + prop->offset))) ? (*((Texture **)(base + prop->offset)))->filename : "");
        break;
        
        case PROP_VEC2:
        ImGui::InputFloat2(prop->name, ((Vec2 *)(base + prop->offset))->data);
        break;
        
        case PROP_STRUCT:
        Property *first_prop = find_property_list(prop->struct_type_name);
        if(ImGui::CollapsingHeader(prop->struct_type_name, true)) {
            ImGui::Indent();
            while(first_prop->name) {
                do_property(first_prop, base + prop->offset);
                first_prop++;
            }
            ImGui::Unindent();
            break;
        }
    }
}

void editor_do_ui() {
    ImGui::Begin("Level");
    ImGui::Text("Current Level: %s", current_level);
    
    if(da_mode == DA_POSITION) ImGui::Text("Editing position");
    else ImGui::Text("Editing size");
    
    if(ImGui::Button("Next Level")) {
        if(current_level_num + 1 < num_levels) {
            current_level_num++;
            current_level = level_list[current_level_num];
            load_entities(current_level);
            selected_entity = null;
        }
    }
    if(ImGui::Button("Previous Level")) {
        if(current_level_num - 1 >= 0) {
            current_level_num--;
            current_level = level_list[current_level_num];
            load_entities(current_level);
            selected_entity = null;
        }
    }
    if(ImGui::Button("Save")) save_entities(current_level);
    
    for(int i = 0; i < num_entity_types; i++) {
        if(ImGui::Button(entity_type_names[i])) {
            Entity *ent = ent_create_from_name(entity_type_names[i]);
            ent->po.position = screen_to_world(camera_position + (window_size * Vec2(0.5f, 0.5f)));
            context_open = false;
            record_undo(ent, new Undo_Record_Create_Entity());
            break;
        }
    }
    
    ImGui::End();
    
    if (selected_entity) {
        Property* props = Entity::_properties;
        Property* derived_props = get_derived_property_list(selected_entity);
        
        ImGui::Begin("Inspector");
        if (ImGui::Button("Reset to default state")) {
            ent_setup_default_state(selected_entity);
        }
        
        ImGui::Text("identifiers: [%s %d %d %d]", selected_entity->type_name, selected_entity->index, selected_entity->parity, selected_entity->derived_index);
        
        while(props->name) {
            do_property(props, (uchar *)selected_entity);
            props++;
        }
        
        if(ImGui::CollapsingHeader(selected_entity->type_name, true)) {
            ImGui::Indent();
            while(derived_props->name) {
                do_property(derived_props, (uchar *)(selected_entity->derived));
                derived_props++;
            }
            ImGui::Unindent();
        }
        ImGui::End();
    }
}

Entity *find_entity_at_point(Vec2 point) {
    Entity *found = null;
    for(int i = 0; i < entities.size; i++) {
        if(!entities.filled[i]) continue;
        if(entities[i].remove_me) continue;
        Entity *entity = &entities[i];
        
        if(point_box_intersection(point, entity->po.position, entity->po.size, true)) {
            found = entity;
            break;
        }
    }
    return found;
}

bool editor_process_event(SDL_Event *event) {
    if(event->type == SDL_KEYDOWN) {
        if(event->key.keysym.scancode == SDL_SCANCODE_F5) {
            toggle_paused();
            //selected_entity = null;
            context_open = false;
            
            if(is_paused) {
                load_entities(current_level);
            } else {
                save_entities(current_level);
                for(int i = 0; i < entities.size; i++) {
                    if(!entities.filled[i]) continue;
                    if(entities[i].remove_me) continue;
                    Entity *entity = &entities[i];
                    entity->spawn_position = entity->po.position;
                }
            }
        }
        
        if(event->key.keysym.scancode == SDL_SCANCODE_F6) {
            load_entities(current_level);
            toggle_paused();
            toggle_paused();
        }
    }
    
    if(!is_paused) return true;
    
    if(event->type == SDL_MOUSEWHEEL) {
        if(event->wheel.y > 0) {
			zoom_level -= 1;
			camera_position = camera_position + ((cursor_position - (window_size * Vec2(0.5, 0.5))) * inverse_scale_for_zoom_level());;
		}
        if(event->wheel.y < 0) {
			zoom_level += 1;
		}
    }
    
    if(event->type == SDL_MOUSEBUTTONUP) {
        if(event->button.button == SDL_BUTTON_LEFT) {
            if(dragging_x || dragging_y) {
                switch(da_mode){ 
                    case DA_POSITION:
                    record_undo(selected_entity, new Undo_Record_Position(original_entity_position));
                    break;
                    case DA_SIZE:
                    record_undo(selected_entity, new Undo_Record_Size(original_entity_size));
                    break;
                }
            }
            dragging_x = false;
            dragging_y = false;
        }
        
        if(event->button.button == SDL_BUTTON_RIGHT) {
            context_open = true;
            context_position = cursor_position;
            
            Entity *found = find_entity_at_point(screen_to_world(cursor_position));
            
            if(found) {
                context_type = CONTEXT_EDIT_ENTITY;
                selected_entity = found;
            } else {
                context_type = CONTEXT_CREATE_ENTITY;
            }
        }
        
        if(event->button.button == SDL_BUTTON_MIDDLE) {
            dragging = false;
        }
    }
    
    if(event->type == SDL_MOUSEBUTTONDOWN) {
        if(event->button.button == SDL_BUTTON_LEFT) {
            if(selected_entity && point_box_intersection(screen_to_world(cursor_position), selected_entity->po.position + dragging_x_pos, move_x_axis->size + Vec2(20,20), true)) {
                dragging_x = true;
                original_entity_position = selected_entity->po.position;
                original_entity_size = selected_entity->po.size;
            } else if (selected_entity && point_box_intersection(screen_to_world(cursor_position), selected_entity->po.position + dragging_y_pos, move_x_axis->size + Vec2(20,20), true)) {
                dragging_y = true;
                original_entity_position = selected_entity->po.position;
                original_entity_size = selected_entity->po.size;
            } else {
                Entity *found = find_entity_at_point(screen_to_world(cursor_position));
                if(found) {
                    selected_entity = found;
                } else {
                    context_open = false;
                    selected_entity = null;
                }
            }
        }
        
        if(event->button.button == SDL_BUTTON_MIDDLE) {
            dragging = true;
            drag_origin = cursor_position - (window_size * Vec2(0.5f, 0.5f));
        }
    }
    
    if(event->type == SDL_MOUSEMOTION) {
        if(dragging) {
            Vec2 diff = Vec2(event->motion.xrel, event->motion.yrel);
            camera_position = camera_position - diff;
        }
        
        if(selected_entity) {
            if(dragging_x) {
                Vec2 diff = Vec2(event->motion.xrel, 0) * scale_for_zoom_level();
                switch(da_mode) {
                    case DA_POSITION:
                    selected_entity->po.position = selected_entity->po.position + diff;
                    if(shift_down) selected_entity->po.position =  align(selected_entity->po.position, 4);
                    break;
                    case DA_SIZE:
                    selected_entity->po.size = selected_entity->po.size + diff;
                    if(shift_down) selected_entity->po.size = align(selected_entity->po.size, 4);
                    break;
                }
                
            }
            
            if(dragging_y) {
                Vec2 diff = Vec2(0, event->motion.yrel) * scale_for_zoom_level();
                switch(da_mode) {
                    case DA_POSITION:
                    selected_entity->po.position = selected_entity->po.position + diff;
                    if(shift_down) selected_entity->po.position = align(selected_entity->po.position, 4);
                    break;
                    case DA_SIZE:
                    selected_entity->po.size = selected_entity->po.size - diff;
                    if(shift_down) selected_entity->po.size = align(selected_entity->po.size, 4);
                    break;
                }
            }
        }
    }
    
    if(event->type == SDL_KEYDOWN) {
        int key = event->key.keysym.scancode;
        
        if(key == SDL_SCANCODE_W) da_mode = DA_POSITION;
        if(key == SDL_SCANCODE_R) da_mode = DA_SIZE;
        
        if(key == SDL_SCANCODE_DELETE) {
            if(selected_entity) {
                selected_entity->remove_me = true;
                selected_entity = null;
                context_open = false;
            }
        }
        
        if(key == SDL_SCANCODE_C && control_down) {
            if(selected_entity) {
                copied_entity = selected_entity;
            }
        }
        
        if(key == SDL_SCANCODE_V && control_down) {
            if(copied_entity) {
                Entity *new_entity = ent_create_from_name(copied_entity->type_name);
                copy_entity(copied_entity, new_entity);
                selected_entity = new_entity;
            }
        }
        
        if(key == SDL_SCANCODE_Z && control_down) {
            undo();
        }
    }
    
    return true;
}








