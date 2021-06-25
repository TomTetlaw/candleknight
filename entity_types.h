#ifndef ENTITY_TYPES_H
#define ENTITY_TYPES_H

struct Player { //@entity_type
    entity_stuff(Player);
    
    bool is_lit = false; //@prop
    bool can_jump = true; //@prop
    
    float weight = 0; //@prop
    float min_weight = 25.0f; //@prop
    float max_weight = 100.0f; //@prop
    float weight_factor0 = 0.25f; //@prop
    float weight_factor1 = 5.0f; //@prop
    
    float jump_speed = 400; //@prop
    int move_speed = 200; //@prop
    float fall_multiplier = 2.5f; //@prop
    float low_jump_multiplier = 2.0f; //@prop
    int num_jumps = 0;
    
    Texture *sword_texture = null; //@prop
    bool has_sword = false; //@prop
    bool flame_on_wick = false; //@prop
    float shrink_speed = 10.0f; //@prop
    Particle_System fire_system; //@prop
    bool dead = false; //@prop
};

void kill(Player *player);

struct Wall { //@entity_type
    entity_stuff(Wall);
};

struct Altar { //@entity_type
    entity_stuff(Altar);
};

struct Brazier { //@entity_type
    entity_stuff(Brazier);
};

struct Crusher { //@entity_type
    entity_stuff(Crusher);
};

struct Wind_Flow { //@entity_type
    entity_stuff(Wind_Flow);
    
    Vec2 flow_direction = Vec2(0, 1); //@prop
    Vec2 flow_size = Vec2(64, 300); //@prop
    float flow_speed = 100.0f; //@prop
};

#endif
