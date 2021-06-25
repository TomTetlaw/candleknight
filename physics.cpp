#include "includes.h"

internal int physics_fps = 60;
internal float accumulator = 0.0f;

struct Intersection {
    Physics_Object *a = nullptr;
    Physics_Object *b = nullptr;
    Vec2 normal = Vec2(0, 0);
    float depth = 0.0f;
};

internal Array<Intersection> intersections;

internal bool should_collide(uint a_groups, uint a_mask, uint b_groups, uint b_mask) {
    return (a_mask & b_groups) != 0 &&
        (a_groups & b_mask) != 0;
}

internal bool should_collide(Physics_Object *a, Physics_Object *b) {
    return should_collide(a->groups, a->mask,
                          b->groups, b->mask);
}

internal bool should_collide(Physics_Object *a, Collision_Filter filter) {
    return should_collide(a->groups, a->mask,
                          filter.groups, filter.mask);
}

internal bool box_box_intersects(Intersection *intersection) {
    Physics_Object *a = intersection->a;
    Physics_Object *b = intersection->b;
    
    if(a->extents.right < b->extents.left || a->extents.left > b->extents.right) return false;
    if(a->extents.bottom < b->extents.top || a->extents.top > b->extents.bottom) return false;
    
    Vec2 n = a->position - b->position;
    
    float a_extent = (a->extents.right - a->extents.left) / 2;
    float b_extent = (b->extents.right - b->extents.left) / 2;
    float x_overlap = a_extent + b_extent - fabs(n.x);
    if(x_overlap > 0) {
        a_extent = (a->extents.bottom - a->extents.top) / 2;
        b_extent = (b->extents.bottom - b->extents.top) / 2;
        float y_overlap = a_extent + b_extent - fabs(n.y);
        
        if(x_overlap < y_overlap) {
            if(n.x < 0) intersection->normal = Vec2(-1, 0);
            else intersection->normal = Vec2(1, 0);
            intersection->depth = x_overlap;
            return true;
        } else {
            if(n.y < 0) intersection->normal = Vec2(0, -1);
            else intersection->normal = Vec2(0, 1);
            intersection->depth = y_overlap;
            return true;
        }
    }
    
    return false;
}

internal void update_object_properties(Physics_Object *po) {
    po->hw = po->size.x / 2;
    po->hh = po->size.y / 2;
    po->extents = { po->position.y - po->hh, po->position.x - po->hw, po->position.y + po->hh, po->position.x + po->hw };
    po->edges[0].a = Vec2(po->extents.left, po->extents.top);
    po->edges[0].b = Vec2(po->extents.right, po->extents.top);
    po->edges[1].a = Vec2(po->extents.right, po->extents.top);
    po->edges[1].b = Vec2(po->extents.right, po->extents.bottom);
    po->edges[2].a = Vec2(po->extents.right, po->extents.bottom);
    po->edges[2].b = Vec2(po->extents.left, po->extents.bottom);
    po->edges[3].a = Vec2(po->extents.left, po->extents.bottom);
    po->edges[3].b = Vec2(po->extents.left, po->extents.top);
}

internal void physics_step_object(Physics_Object *po, float dt) {
    po->velocity = approach(po->velocity, po->goal_velocity, dt * po->velocity_ramp_speed);
    po->position = po->position + (po->velocity * dt);
    update_object_properties(po);
}

internal void check_for_intersects() {
    for(int i = 0; i < entities.size; i++) {
        if(!entities.filled[i]) continue;
        if(entities[i].remove_me) continue;
        
        for(int j = 0; j < entities.size; j++) {
            if(!entities.filled[j]) continue;
            if(entities[j].remove_me) continue;
            
            if(j == i) continue;
            
            Physics_Object *a = &entities[i].po;
            Physics_Object *b = &entities[j].po;
            
            if(a->mass == 0.0f && b->mass == 0.0f) continue;
            
            if(!should_collide(a, b)) continue;
            
            bool intersects = false;
            Intersection intersection;
            intersection.a = a;
            intersection.b = b;
            intersects = box_box_intersects(&intersection);
            
            if(intersects) {
                array_add(&intersections, intersection);
                a->colliding = true;
            } else {
                a->colliding = false;
            }
        }
    }
}

struct Line_Intersection {
    Vec2 a, b, c, d;
    Vec2 point;
    float t1 = 0.0f;
};

internal float det(float a, float b, float c, float d) {
    return a*d - b*c;
}

internal bool lines_intersect(Line_Intersection *intersection) {
    Vec2 a = intersection->a;
    Vec2 b = intersection->b;
    Vec2 c = intersection->c;
    Vec2 d = intersection->d;
    
    // ray point
    float r_px = a.x; 
    float r_py = a.y;
    // ray direction
    float r_dx = b.x - a.x; 
    float r_dy = b.y - a.y;
    // segment point
    float s_px = c.x; 
    float s_py = c.y;
    // segment direction
    float s_dx = d.x - c.x; 
    float s_dy = d.y - c.y;   
    // ray length
    float r_mag = Vec2(r_dx, r_dy).length_squared(); // don't use slow sqrt
    // segment length
    float s_mag = Vec2(s_dx, s_dy).length_squared(); // don't use slow sqrt
    
    // line directions are the same
    if((r_dx*r_dx) / r_mag == (s_dx*s_dx) / s_mag && (r_dy*r_dy) / r_mag == (s_dy*s_dy) / s_mag) return false;
    
    float t2 = (r_dx*(s_py-r_py) + r_dy*(r_px-s_px))/(s_dx*r_dy - s_dy*r_dx);
    float t1 = (s_px+s_dx*t2-r_px)/r_dx;
    
    if(isnan(t1)) return false;
    if(t1 < 0.0f || t1 > 1.0f) return false;
    if(t2 < 0.0f || t2 > 1.0f) return false;
    
    intersection->point.x = r_px + r_dx * t1;
    intersection->point.y = r_py + r_dy * t1;
    intersection->t1 = t1;
    return true;
}

internal void get_edges(Array<Edge> &edges, Collision_Filter filter) {
    for(int i = 0; i < entities.size; i++) {
        if(!entities.filled[i]) continue;
        if(entities[i].remove_me) continue;
        
        Physics_Object *po = &entities[i].po;
        if(!should_collide(po, filter)) continue;
        // @todo: we do this here so that the raycasts in generate_nav_mesh in 
        // game.cpp will give correct results, but probably should do it in a
        // better place.
        update_object_properties(po);
        
        for(int j = 0; j < 4; j++) array_add(&edges, po->edges[j]);
    }
}

bool raycast(Vec2 a, Vec2 b, Raycast_Hit *hit, Collision_Filter filter) {
    Array<Edge> edges;
    get_edges(edges, filter);
    
    bool has_one_hit = false;
    
    Array<Line_Intersection> line_intersections;
    for(int i = 0; i < edges.count; i++) {
        auto it = edges[i];
        
        Line_Intersection intersection;
        intersection.a = a;
        intersection.b = b;
        intersection.c = it.a;
        intersection.d = it.b;
        if(lines_intersect(&intersection)) {
            array_add(&line_intersections, intersection);
            has_one_hit = true;
        }
    }
    
    if(!has_one_hit) return false;
    
    float lowest_dist = 10000.0f;
    int lowest_index = 0;
    for(int i = 0; i < line_intersections.count; i++) {
        float dist = a.distance_to(line_intersections[i].point);
        if(dist < lowest_dist) {
            lowest_dist = dist;
            lowest_index = i;
        }
    }
    
    hit->point = line_intersections[lowest_index].point;
    hit->alpha = line_intersections[lowest_index].t1;
    return true;
}

internal void correct_position(Intersection *intersection) {
    Physics_Object *a = intersection->a;
    Physics_Object *b = intersection->b;
    
    const float percent = 0.2f;
    const float slop = 0.1f;
    Vec2 correction = intersection->normal * ((max(intersection->depth - slop, 0.0f) / (a->inv_mass + b->inv_mass)) * percent);
    if(!b->is_sensor) a->position = a->position + (correction * a->inv_mass);
    if(!a->is_sensor) b->position = b->position - (correction * b->inv_mass);
}

internal void resolve_intersections() {   
    for(int i = 0; i < intersections.count; i++) {
        auto it = intersections[i];
        Physics_Object *a = it.a;
        Physics_Object *b = it.b;
        
        //if(a->inv_mass == 0.0f) continue;
        
        Vec2 relative_velocity = b->velocity - a->velocity;
        
        float velocity_along_normal = relative_velocity.dot(it.normal);
        // don't collide if objects are seperating
        if(velocity_along_normal < 0) return;
        
        float e = min(a->restitution, b->restitution);
        
        // calculate impulse scalar
        float j = -(1 + e) * velocity_along_normal;
        j /= a->inv_mass + b->inv_mass;
        
        // apply impulse
        Vec2 impulse = it.normal * j;
        if(!b->is_sensor) a->velocity = a->velocity - (impulse * a->inv_mass);
        if(!a->is_sensor) b->velocity = b->velocity + (impulse * b->inv_mass);
        
        correct_position(&it);
        
        ent_handle_collision(a->owner, b->owner);
    }
}

void physics_init() {
}

internal void integrate(float dt) {
    for(int i = 0; i < entities.size; i++) {
        if(!entities.filled[i]) continue;
        if(entities[i].remove_me) continue;
        physics_step_object(&entities[i].po, dt);
    }
    
    intersections.count = 0;
    check_for_intersects();
    resolve_intersections();
}

void physics_step_world(float dt) {
    if(is_paused) return;
    
    float physics_dt = 1.0f / (float)physics_fps;
    
    accumulator += dt;
    while(accumulator >= physics_dt) {
        integrate(physics_dt);
        accumulator -= physics_dt;
    }
    
    // step over the last little bit of time @todo: this is not correct
    integrate(accumulator);
    accumulator = 0.0f;
}

void physics_render_debug(Physics_Object *po) {
    Vec4 colour = Vec4(1, 0, 0, 1);
    if(po->colliding) colour = Vec4(0, 1, 0, 1);
    r_render_box(po->position, true, po->size, true, colour, true);
}