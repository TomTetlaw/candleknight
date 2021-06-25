#ifndef PARTICLE_H
#define PARTICLE_H

struct Particle {
    Vec2 position;
    float angle = 0.0f;
    Vec2 velocity;
    float angular_velocity = 0.0f;
    float lifetime = 0.0f;
};

struct Particle_System { //@generate_properties
    Texture *texture = null; //@prop
    Vec4 particle_colour = Vec4(1, 1, 1, 1);
    Vec2 position; //@prop
    Vec2 position_spread; //@prop
    Vec2 velocity; //@prop
    Vec2 velocity_spread; //@prop
    float angular_velocity = 0.0f; //@prop
    float angular_velocity_spread = 0.0f; //@prop
    float lifetime = 0.0f; //@prop
    float lifetime_spread = 0.0f; //@prop
    float spawn_rate = 0.0f; //@prop
    float last_spawn_time = 0.0f;
    Contiguous_Array<Particle> particles;
    
    declare_property_block();
};

void init_particle_system(Particle_System *system, int capacity);
void update_particle_system(Particle_System *system);
void render_particle_system(Particle_System *system);

#endif
