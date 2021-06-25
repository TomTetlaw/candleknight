#include "includes.h"

Contiguous_Array<Entity> entities;

internal int next_parity = 0;

#include "entity_generated.cpp"

void ent_init() {
    carray_init(&entities, entities_per_list);
    initialize_entity_lists();
    assign_entity_type_tags();
    init_num_properties();
}

void ent_shutdown() {
    carray_free(&entities);
    free_entity_lists();
}

void ent_setup_default_state(Entity *entity) {
    if(entity->tag == _Player) {
        entity->texture = load_texture("data/textures/player.png");
        entity->po.size = entity->texture->size;
        entity->po.set_mass(1);
        entity->po.velocity_ramp_speed = 3000;
        
        Player *player = downcast(entity, Player);
        player->sword_texture = load_texture("data/textures/sword.png");
        player->fire_system.texture = load_texture("data/textures/flame_particle.png");
        player->fire_system.position = Vec2(400, 400);
        player->fire_system.position_spread = Vec2(20, 0);
		player->fire_system.velocity = Vec2(0, -25);
        player->fire_system.velocity_spread = Vec2(10, 0);
        player->fire_system.lifetime = 2.0f;
        player->fire_system.lifetime_spread = 1.0f;
        player->fire_system.spawn_rate = 1000;
        player->fire_system.particle_colour = Vec4(1, 1, 1, 0.8f);
        init_particle_system(&player->fire_system, 1000);
    } else if (entity->tag == _Wall) {
        entity->texture = load_texture("data/textures/grass.jpg");
        entity->po.size = entity->texture->size;
        entity->po.set_mass(0);
    } else if (entity->tag == _Altar) {
        entity->texture = load_texture("data/textures/altar.png");
        entity->po.size = entity->texture->size;
        entity->po.set_mass(0);
        entity->po.is_sensor = true;
    } else if (entity->tag == _Brazier) {
        entity->texture = load_texture("data/textures/brazier_lit.png");
        entity->po.size = entity->texture->size;
        entity->po.set_mass(0);
        entity->po.is_sensor = true;
    } else if (entity->tag == _Crusher) {
        entity->texture = load_texture("data/textures/crusher.png");
        entity->po.size = entity->texture->size;
        entity->po.set_mass(0);
        entity->po.is_sensor = true;
    } else if (entity->tag == _Wind_Flow) {
        entity->texture = load_texture("data/textures/wind_flow.png");
        entity->render_size = entity->texture->size;
        Wind_Flow *wf = downcast(entity, Wind_Flow);
        entity->render_position = Vec2(0, wf->flow_size.y) * Vec2(0.5, 0.5);
        entity->po.size = wf->flow_size;
        entity->po.set_mass(0);
        entity->po.is_sensor = true;
    }
}

template<typename T>
T *_ent_create() {
    int index = -1;
    Entity *entity = carray_alloc(&entities, &index);
    entity->index = index;
    entity->parity = next_parity++;
    entity->tag = T::_tag;
    entity->type_name = T::_type_name;
    entity->po.owner = entity;
    
    int derived_index = -1;
    T *derived = carray_alloc(&T::_list, &derived_index);
    derived->base = entity;
    entity->derived_index = derived_index;
    entity->derived = derived;
    
    ent_setup_default_state(entity);
    
    return derived;
}

Entity *ent_get_from_handle(ehandle handle) {
    if(handle.index < 0 || handle.index >= entities.size)
        return null;
    Entity *entity = &entities[handle.index];
    if(entity->parity != handle.parity)
        return null;
    return entity;
}

template<typename T>
void ent_remove(T *entity) {
    assert(entity->base->index < 0, "trying to remove an entity that hasn't been added yet");
    carray_remove(&T::_list, entity->base->derived_index);
    carray_remove(&entities, entity->base->index);
}

template<typename T>
inline T *_downcast(Entity *base) {
    if(base->tag != T::_tag) {
        assert(false, "Trying to downcast an entity to a type that it isn't.");
        return null;
    }
    return &T::_list[base->derived_index];
}

void player_add_weight(Player* player, float w) {
    player->weight = clamp(player->weight + w, player->min_weight, player->max_weight);
}

float player_weight_curve(float x) {
    return 1 / (powf(2, x)); // see https://www.google.com/search?q=y%3D1%2Fx%5E2
}

float player_weight_factor(Player *player) {
    float fraction = map_range(player->weight, player->min_weight, player->max_weight, 0, 1);
    float weight01 = player_weight_curve(fraction);
    return map_range(weight01, 0, 1, player->weight_factor0, player->weight_factor1);
}

void ent_update() {
    if(is_paused) return;
    
    for(int i = 0; i < Crusher::_list.size; i++) {
        if(!Crusher::_list.filled[i]) continue;
        Crusher *crusher = &Crusher::_list[i];
        Entity *entity = crusher->base;
        
        //assert(entity->tag == _Wall, "oh no");
        
        int rand1 = rand() % 2;
        int rand2 = rand() % 100;
        
        //entity->po.position.y = entity->spawn_position.y + (sin(game_time * (rand1 + 1))) * (100 + rand2);
        entity->po.position.y = entity->spawn_position.y + (sin(game_time)) * 100;
    }
    
    for(int i = 0; i < entities.size; i++) {
        if(!entities.filled[i]) continue;
        if(entities[i].po.mass > 0) entities[i].po.goal_velocity.y = 300;
    }
    
    for(int i = 0; i < entities.size; i++) {
        if(!entities.filled[i]) continue;
        if(entities[i].remove_me) ent_remove_base(&entities[i]);
    }
    
    Player *player = null;
    
    for(int i = 0; i < Player::_list.size; i++) {
        if(Player::_list.filled[i]) {
            player = &Player::_list[i];
            break;
        }
    }
    
    debug_string("player_weight_factor = %f", player_weight_factor(player));
    debug_string("player speed = %f", player->move_speed * player_weight_factor(player));
    debug_string("player jump speed = %f", player->jump_speed * player_weight_factor(player));
    Entity *entity = player->base;
    
    camera_position = entity->po.position - (Vec2(0.5f, 0.5f) * window_size);
    
    if(entity->tag == _Player) {
        if(player->dead) return;
        
        entity->po.goal_velocity.x = 0;
        
        if(input_get_key_state(SDL_SCANCODE_A)) {
            entity->po.goal_velocity.x = -player->move_speed * player_weight_factor(player);
        }
        if(input_get_key_state(SDL_SCANCODE_D)) {
            entity->po.goal_velocity.x = player->move_speed * player_weight_factor(player);
        }
        
        if(player->is_lit) {
            entity->po.size.y -= (player->shrink_speed * delta_time);
            entity->po.position.y += (player->shrink_speed * delta_time);
            
            player_add_weight(player, -(player->shrink_speed * delta_time));
            
            if(player->weight <= 0.0f) {
                kill(player);
            }
        }
        if(player->flame_on_wick) {
            player->fire_system.position = entity->po.position + Vec2(0, -40);
        } else {
            player->fire_system.position = entity->po.position + Vec2(20, -40);
        }
        if(player->is_lit || player->has_sword) {
            update_particle_system(&player->fire_system);
        }
    }
}

void player_jump(Entity* entity)
{
    Player* player = downcast(entity, Player);
    
    /*if (player->can_double_jump && num_jumps < 2 && player->po.velocity.y != 0)
    {
        player->po.velocity += new Vec2(player->po.velocity.x, 0);
        player->po.velocity += Vec2(0, 1) * player->jump_height;
        num_jumps++;
        player->can_jump = false;
    }
    else*/
    
    if (player->can_jump) {
        entity->po.velocity.y = -(player->jump_speed * player_weight_factor(player));
        player->num_jumps++;
        player->can_jump = false;
    }
}

/*void check_fall()
{
    Entity* player = player->base;

    if (player->po.velocity.y > 0) {
        player->po.velocity += Vec2(0, 1) * Physics2D.gravity.y * (player->fall_multiplier - 1) * delta_time;
    }
    else if (player->po.velocity.y < 0 && (event->type == SDL_KEYUP && event->key.keysym.scancode == SDL_SCANCODE_SPACE) {
        player->po.velocity += Vec2(0, 1) * Physics2D.gravity.y * (player->low_jump_multiplier - 1) * delta_time;
    }
}*/

void ent_handle_event(SDL_Event *event) {
    Player *player = null;
    
    for(int i = 0; i < Player::_list.size; i++) {
        if(Player::_list.filled[i]) {
            player = &Player::_list[i];
            break;
        }
    }
    
    if (!player) return;
    if(player->dead) return;
    
    Entity *entity = player->base;
    
    if(event->type == SDL_MOUSEBUTTONDOWN || event->type == SDL_MOUSEBUTTONUP) {
        bool down = event->type == SDL_MOUSEBUTTONDOWN;
        int button = event->button.button;
        
        if(button == SDL_BUTTON_RIGHT && down) {
            if(player->has_sword) {
                player->is_lit = !player->is_lit;
                if (player->is_lit) {
                    player->sword_texture = load_texture("data/textures/sword.png");
                    player->flame_on_wick = true;
                    entity->texture = load_texture("data/textures/player_lit.png");
                }
                else {
                    player->sword_texture = load_texture("data/textures/sword_lit.png");
                    player->flame_on_wick = false;
                    entity->texture = load_texture("data/textures/player.png");
                }
            }
        }
    }
    
    if(event->type == SDL_KEYDOWN || event->type == SDL_KEYUP) {
        bool key_down = event->type == SDL_KEYDOWN;
        bool key_up = event->type == SDL_KEYUP;
        int key = event->key.keysym.scancode;
        if(key == SDL_SCANCODE_SPACE && key_down) {
            player_jump(entity);
        }
        if ((key == SDL_SCANCODE_A || key == SDL_SCANCODE_D) && key_up)
        {
            entity->po.goal_velocity.x = 0;
            entity->po.velocity.x = 0;
        }
    }
}

void ent_handle_collision(Entity *a, Entity *b) {
    if(a->tag == _Player && b->tag == _Brazier) {
        Player *player = downcast(a, Player);
        if(player->dead) return;
        player->is_lit = true;
        player->flame_on_wick = true;
        a->texture = load_texture("data/textures/player_lit.png");
    }
    
    if(a->tag == _Player && b->tag == _Altar) {
        Player *player = downcast(a, Player);
        if(player->dead) return;
        
        a->po.size.y = a->texture->size.y;
        player->weight = 100.0f;
        
        if(player->is_lit){ 
            b->texture = load_texture("data/textures/altar_melted.png");
            player->has_sword = true;
        }
    }
    
    if(a->tag == _Player && b->tag == _Crusher) {
        kill(downcast(a, Player));
    }
    
    if(a->tag == _Player && b->tag == _Wind_Flow) {
        Player *player = downcast(a, Player);
        if(player->dead) return;
        
        auto wind_flow = downcast(b, Wind_Flow);
        a->po.velocity = a->po.velocity + Vec2(0, -wind_flow->flow_speed * delta_time * 15);
    }
    
    if (a->tag == _Player && b->tag == _Wall) {
        Player* player = downcast(a, Player);
        player->num_jumps = 0;
        player->can_jump = true;
    }
}

void ent_render() {
    for(int i = 0; i < entities.size; i++) {
        if(!entities.filled[i]) continue;
        if(entities[i].remove_me) continue;
        Entity *entity = &entities[i];
        if(!entity->texture) continue;
        
        Vec2 render_size = entity->render_size;
        if(render_size.x == -1) render_size.x = entity->po.size.x;
        if(render_size.y == -1) render_size.y = entity->po.size.y;
        
        
        Vec2 render_position = entity->po.position;
        if(render_position.x != -1) render_position.x += entity->render_position.x;
        if(render_position.y != -1) render_position.y += entity->render_position.y;
        
        r_render_texture(entity->texture, render_position, true, Vec4(1, 1, 1, 1), render_size);
        
        if(entity->tag == _Player) {
            Player *player = downcast(entity, Player);
            if(player->has_sword) {
                r_render_texture(player->sword_texture, entity->po.position + Vec2(20, 0), true, Vec4(1, 1, 1, 1), player->sword_texture->size);
            }
            if(player->is_lit || player->has_sword) {
                render_particle_system(&player->fire_system);
            }
        }
        
        //physics_render_debug(&entity->po);
    }
}

// taken from https://stackoverflow.com/questions/10402499/mkdir-c-function
bool directory_exists(const char *path) {
    struct stat info;
    if(stat(path, &info) != 0) return 0;
    else if(info.st_mode & S_IFDIR) return 1;
    else return 0;
}

Property *find_property_list(const char *type_name){
    for(int i = 0; i < _num_property_lists; i++) {
        if(!strcmp(_property_lists[i].name, type_name)) {
            return _property_lists[i].properties;
        }
    }
    
    return null;
}

void save_property(FILE *f, uchar *base, Property *prop, int indent_level) {
    for(int i = 0; i < indent_level; i++) fprintf(f, "    ");
    switch(prop->type) {
        case PROP_INT:
        fprintf(f, "%s %d\n", prop->name, *(int *)(base + prop->offset));
        break;
        case PROP_UINT:
        fprintf(f, "%s %u\n", prop->name, *(uint *)(base + prop->offset));
        break;
        case PROP_FLOAT:
        fprintf(f, "%s %f\n", prop->name, *(float *)(base + prop->offset));
        break;
        case PROP_BOOL:
        fprintf(f, "%s %d\n", prop->name, (int)(*(bool *)(base + prop->offset)));
        break;
        case PROP_TEXTURE:
        fprintf(f, "%s %s\n", prop->name, (*((Texture **)(base + prop->offset))) ? (*((Texture **)(base + prop->offset)))->filename : "none");
        break;
        case PROP_VEC2:
        fprintf(f, "%s [%f, %f]\n", prop->name, ((Vec2 *)(base + prop->offset))->x, ((Vec2 *)(base + prop->offset))->y);
        break;
        case PROP_STRUCT:
        Property *first_prop = find_property_list(prop->struct_type_name);
        fprintf(f, "%s {\n", prop->name);
        while(first_prop->name) {
            save_property(f, base + prop->offset, first_prop, indent_level + 1);
            first_prop++;
        }
        for(int i = 0; i < indent_level; i++) fprintf(f, "    ");
        fprintf(f, "}\n");
        break;
    }
}

void save_entities(const char *level_name) {
    char folder_path[1024] = {};
    strcpy(folder_path, format("data/levels/%s", level_name));
    
    if(directory_exists(folder_path)) {
        tinydir_dir dir;
        tinydir_open(&dir, folder_path);
        
        while(dir.has_next) {
            tinydir_file file;
            tinydir_readfile(&dir, &file);
            if(!file.is_dir && !strcmp(file.extension, "entity")) {
                remove(file.path);
            }
            tinydir_next(&dir);
        }
        
        tinydir_close(&dir);
    } else {
        _mkdir(folder_path);
    }
    
    for(int i = 0; i < entities.size; i++) {
        if(!entities.filled[i]) continue;
        if(entities[i].remove_me) continue;
        Entity *entity = &entities[i];
        
        char file_name[1024] = {};
        strcpy(file_name, format("%s/%s %d %d %d.entity", folder_path, entity->type_name, entity->index, entity->parity, entity->derived_index));
        
        FILE *f = fopen(file_name, "w");
        Property *prop = &Entity::_properties[0];
        while(prop->name) {
            save_property(f, (uchar *)entity, prop, 0);
            prop++;
        }
        prop = get_derived_property_list(entity);
        fprintf(f, "%s {\n", entity->type_name);
        while(prop->name) {
            save_property(f, (uchar *)entity->derived, prop, 1);
            prop++;
        }
        fprintf(f, "}\n");
        fclose(f);
    }
}

Property *find_prop_in_list(Property *list, const char *prop_name, int *offset) {
    while(list->name) {
        if(!strcmp(prop_name, list->name)) {
            *offset = list->offset;
            return list;
        }
        
        list++;
    }
    
    return null;
}

Property *find_prop_in_list(Property_List *list, const char *prop_name, int *offset) {
    return find_prop_in_list(list->properties, prop_name, offset);
}

void load_property(Property *prop, int offset, uchar *base, const char *value) {
    switch(prop->type) {
        case PROP_INT: {
            int i;
            sscanf(value, "%d", &i);
            *(int *)(base + offset) = i;
        } break;
        case PROP_UINT: {
            uint i;
            sscanf(value, "%u", &i);
            *(uint *)(base + offset) = i;
        } break;
        case PROP_FLOAT: {
            float f;
            sscanf(value, "%f", &f);
            *(float *)(base + offset) = f;
        } break;
        case PROP_BOOL: {
            int b;
            sscanf(value, "%d", &b);
            *(bool *)(base + offset) = b == 1 ? true : false;                        
        } break;
        case PROP_TEXTURE: {
            Texture *t = load_texture(value);
            *((Texture **)(base + offset)) = t;
        } break;
        case PROP_VEC2: {
            float x, y;
            sscanf(value, "%f, %f", &x, &y);
            ((Vec2 *)(base + offset))->x = x;
            ((Vec2 *)(base + offset))->y = y;
        } break;
    }
}

const char *parse_entity_save_file(Entity *entity, Property *list, uchar *base, const char *data) {
    if(!list) return null;
    
    char token[1024] = {};
    while(data) {
        data = parse_token(data, token);
        if(!data || token[0] == '}') return data;
        
        int offset = 0;
        Property *prop = find_prop_in_list(list, token, &offset);
        if(!prop) {
            if(!strcmp(token, entity->type_name)) {
                data = parse_token(data, token); // skip the '{'
                data = parse_entity_save_file(entity, find_property_list(entity->type_name), (uchar *)entity->derived, data);
            }
            
            if(!prop) break;
        }
        
        if(prop->type == PROP_STRUCT) {
            data = parse_token(data, token); // skip the '{'
            data = parse_entity_save_file(entity, find_property_list(prop->struct_type_name), base + offset, data);
        } else {
            data = parse_token(data, token);
            load_property(prop, offset, base, token);
        }
    }
    
    return null;
}

void load_entities(const char *level_name) {
    next_parity = 0;
    for(int i = 0; i < entities.size; i++) {
        entities.filled[i] = false;
        Entity dummy;
        memcpy(&entities[i], &dummy, sizeof(Entity));
    }
    
    ent_reset_all_lists();
    
    char folder_path[1024] = {};
    strcpy(folder_path, format("data/levels/%s", level_name));
    if(!directory_exists(folder_path)) {
        assert(false, "Level %s does not exist!", level_name);
        return;
    }
    
    tinydir_dir dir;
    tinydir_open(&dir, folder_path);
    
    while(dir.has_next) {
        tinydir_file file;
        tinydir_readfile(&dir, &file);
        if(!file.is_dir) {
            int index = -1;
            int parity = -1;
            int derived_index = -1;
            char type_name[1024] = {};
            sscanf(file.name, "%s %d %d %d.entity", type_name, &index, &parity, &derived_index);
            
            Entity *entity = ent_create_from_file(index, parity, derived_index, type_name);
            if(parity >= next_parity)
                next_parity = parity + 1;
            
            Load_File_Result entire_file = load_entire_file(format("%s/%s", folder_path, file.name));
            defer { free(entire_file.data); };
            const char *data = (const char *)entire_file.data;
            
            Property *list = &Entity::_properties[0];
            uchar *base = (uchar *)entity;
            
            parse_entity_save_file(entity, list, base, data);
        }
        
        tinydir_next(&dir);
    }
    
    tinydir_close(&dir);
}