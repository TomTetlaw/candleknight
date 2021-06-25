#include "includes.h"

void kill(Player *player) {
    player->base->texture = load_texture("data/textures/player_dead.png");
    player->base->render_size = player->base->texture->size;
    player->base->render_position = Vec2(0, 32);
    player->is_lit = false;
    player->has_sword = false;
    player->dead = true;
    player->base->po.velocity = Vec2(0,0);
    player->base->po.goal_velocity = Vec2(0,0);
}