#ifndef ENTITY_H
#define ENTITY_H

#include "entity_generated.h"

struct ehandle {
    int index = -1;
    int parity = -1;
};

struct Entity { //@generate_properties
    int index = -1;
    int parity = -1;
    int derived_index = -1;
    
    Entity_Type tag;
    const char *type_name = null;
    void *derived = null;
    bool remove_me = false;
    
    Texture *texture = null; // @prop
    Vec2 spawn_position; //@prop
    Vec2 render_position = Vec2(-1, -1); //@prop
    Vec2 render_size = Vec2(-1, -1); //@prop
    Physics_Object po; //@prop
    float test_float = 0; //@prop
    
    declare_property_block();
};

const int entities_per_list = 1024;
extern Contiguous_Array<Entity> entities;

#define entity_stuff(T) static Entity_Type _tag; static const char *_type_name; static Contiguous_Array<T> _list; Entity *base = null; declare_property_block()

void ent_init();
void ent_shutdown();
void ent_update();
void ent_render();

void ent_handle_collision(Entity *a, Entity *b);

void ent_handle_event(SDL_Event *event);

void ent_setup_default_state(Entity *entity);

void save_entities(const char *level_name);
void load_entities(const char *level_name);

#define ent_create(T) _ent_create<T>()
template<typename T>
T *_ent_create();

Entity *create_entity_from_file(int index, int parity, int derived_index, const char *type_name);

#define downcast(e, T) _downcast<T>(e)
template<typename T>
T *_downcast(Entity *base);

Entity *ent_get_from_handle(ehandle handle);

template<typename T>
void ent_remove(T *entity);

#endif
