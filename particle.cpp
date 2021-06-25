#include "includes.h"

void init_particle_system(Particle_System *system, int capacity) {
    carray_init(&system->particles, capacity);
}

void update_particle_system(Particle_System *system) {
    if(current_time - system->last_spawn_time > 1 / system->spawn_rate) {
        system->last_spawn_time = current_time;
        
        Particle *p = carray_alloc(&system->particles);
        if(p) {
            p->position = system->position + random_vector(-system->position_spread, system->position_spread);
            p->velocity = system->velocity + random_vector(-system->velocity_spread, system->velocity_spread);
            p->angular_velocity = system->angular_velocity +  random_float(-system->angular_velocity_spread, system->angular_velocity_spread);
            p->lifetime = current_time + system->lifetime + random_float(-system->lifetime_spread, system->lifetime_spread); 
        }
    }
    
    for(int i = 0; i < system->particles.size; i++) {
        if(!system->particles.filled[i]) continue;
        Particle *p = &system->particles[i];
        
        if(p->lifetime <= current_time) {
            carray_remove(&system->particles, i);
            continue;
        }
        
        p->position = p->position + (p->velocity * delta_time);
        p->angle = p->angle + (p->angular_velocity * delta_time);
    }
}

void render_particle_system(Particle_System *system) {
    if(!system->texture) return;
    
    for(int i = 0; i < system->particles.size; i++) {
        if(!system->particles.filled[i]) continue;
        Particle *p = &system->particles[i];
        
        r_render_texture(system->texture ? system->texture : load_texture("data/textures/default_particle.png"), p->position, true, system->particle_colour);
    }
}