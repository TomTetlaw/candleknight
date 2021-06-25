Contiguous_Array<Player> Player::_list;
Contiguous_Array<Wall> Wall::_list;
Contiguous_Array<Altar> Altar::_list;
Contiguous_Array<Brazier> Brazier::_list;
Contiguous_Array<Crusher> Crusher::_list;
Contiguous_Array<Wind_Flow> Wind_Flow::_list;
start_property_block(Entity)
declare_property(Entity, "", texture, PROP_TEXTURE, 0)
declare_property(Entity, "", spawn_position, PROP_VEC2, 1)
declare_property(Entity, "", render_position, PROP_VEC2, 2)
declare_property(Entity, "", render_size, PROP_VEC2, 3)
declare_property(Entity, "Physics_Object", po, PROP_STRUCT, 4)
declare_property(Entity, "", test_float, PROP_FLOAT, 5)
end_property_block()
start_property_block(Player)
declare_property(Player, "", is_lit, PROP_BOOL, 0)
declare_property(Player, "", can_jump, PROP_BOOL, 1)
declare_property(Player, "", weight, PROP_FLOAT, 2)
declare_property(Player, "", min_weight, PROP_FLOAT, 3)
declare_property(Player, "", max_weight, PROP_FLOAT, 4)
declare_property(Player, "", weight_factor0, PROP_FLOAT, 5)
declare_property(Player, "", weight_factor1, PROP_FLOAT, 6)
declare_property(Player, "", jump_speed, PROP_FLOAT, 7)
declare_property(Player, "", move_speed, PROP_INT, 8)
declare_property(Player, "", fall_multiplier, PROP_FLOAT, 9)
declare_property(Player, "", low_jump_multiplier, PROP_FLOAT, 10)
declare_property(Player, "", sword_texture, PROP_TEXTURE, 11)
declare_property(Player, "", has_sword, PROP_BOOL, 12)
declare_property(Player, "", flame_on_wick, PROP_BOOL, 13)
declare_property(Player, "", shrink_speed, PROP_FLOAT, 14)
declare_property(Player, "Particle_System", fire_system, PROP_STRUCT, 15)
declare_property(Player, "", dead, PROP_BOOL, 16)
end_property_block()
start_property_block(Wall)
end_property_block()
start_property_block(Altar)
end_property_block()
start_property_block(Brazier)
end_property_block()
start_property_block(Crusher)
end_property_block()
start_property_block(Wind_Flow)
declare_property(Wind_Flow, "", flow_direction, PROP_VEC2, 0)
declare_property(Wind_Flow, "", flow_size, PROP_VEC2, 1)
declare_property(Wind_Flow, "", flow_speed, PROP_FLOAT, 2)
end_property_block()
start_property_block(Particle_System)
declare_property(Particle_System, "", texture, PROP_TEXTURE, 0)
declare_property(Particle_System, "", position, PROP_VEC2, 1)
declare_property(Particle_System, "", position_spread, PROP_VEC2, 2)
declare_property(Particle_System, "", velocity, PROP_VEC2, 3)
declare_property(Particle_System, "", velocity_spread, PROP_VEC2, 4)
declare_property(Particle_System, "", angular_velocity, PROP_FLOAT, 5)
declare_property(Particle_System, "", angular_velocity_spread, PROP_FLOAT, 6)
declare_property(Particle_System, "", lifetime, PROP_FLOAT, 7)
declare_property(Particle_System, "", lifetime_spread, PROP_FLOAT, 8)
declare_property(Particle_System, "", spawn_rate, PROP_FLOAT, 9)
end_property_block()
start_property_block(Physics_Object)
declare_property(Physics_Object, "", position, PROP_VEC2, 0)
declare_property(Physics_Object, "", velocity, PROP_VEC2, 1)
declare_property(Physics_Object, "", goal_velocity, PROP_VEC2, 2)
declare_property(Physics_Object, "", velocity_ramp_speed, PROP_FLOAT, 3)
declare_property(Physics_Object, "", is_sensor, PROP_BOOL, 4)
declare_property(Physics_Object, "", size, PROP_VEC2, 5)
declare_property(Physics_Object, "", mass, PROP_FLOAT, 6)
declare_property(Physics_Object, "", inv_mass, PROP_FLOAT, 7)
declare_property(Physics_Object, "", restitution, PROP_FLOAT, 8)
declare_property(Physics_Object, "", groups, PROP_UINT, 9)
declare_property(Physics_Object, "", mask, PROP_UINT, 10)
end_property_block()
Property_List _property_lists[] = {
	{ "Entity", Entity::_properties },
	{ "Player", Player::_properties },
	{ "Wall", Wall::_properties },
	{ "Altar", Altar::_properties },
	{ "Brazier", Brazier::_properties },
	{ "Crusher", Crusher::_properties },
	{ "Wind_Flow", Wind_Flow::_properties },
	{ "Particle_System", Particle_System::_properties },
	{ "Physics_Object", Physics_Object::_properties },
};
int _num_property_lists = 9;
Property *get_derived_property_list(Entity *entity) {
	if (entity->tag == _Player) {
		return Player::_properties;
	}
	if (entity->tag == _Wall) {
		return Wall::_properties;
	}
	if (entity->tag == _Altar) {
		return Altar::_properties;
	}
	if (entity->tag == _Brazier) {
		return Brazier::_properties;
	}
	if (entity->tag == _Crusher) {
		return Crusher::_properties;
	}
	if (entity->tag == _Wind_Flow) {
		return Wind_Flow::_properties;
	}
	return null;
}
void initialize_entity_lists() {
	carray_init(&Player::_list, entities_per_list);
	carray_init(&Wall::_list, entities_per_list);
	carray_init(&Altar::_list, entities_per_list);
	carray_init(&Brazier::_list, entities_per_list);
	carray_init(&Crusher::_list, entities_per_list);
	carray_init(&Wind_Flow::_list, entities_per_list);
}
void free_entity_lists() {
	carray_free(&Player::_list);
	carray_free(&Wall::_list);
	carray_free(&Altar::_list);
	carray_free(&Brazier::_list);
	carray_free(&Crusher::_list);
	carray_free(&Wind_Flow::_list);
}
void assign_entity_type_tags() {
	Player::_tag = Entity_Type::_Player;
	Player::_type_name = "Player";
	Wall::_tag = Entity_Type::_Wall;
	Wall::_type_name = "Wall";
	Altar::_tag = Entity_Type::_Altar;
	Altar::_type_name = "Altar";
	Brazier::_tag = Entity_Type::_Brazier;
	Brazier::_type_name = "Brazier";
	Crusher::_tag = Entity_Type::_Crusher;
	Crusher::_type_name = "Crusher";
	Wind_Flow::_tag = Entity_Type::_Wind_Flow;
	Wind_Flow::_type_name = "Wind_Flow";
}
void init_num_properties() {
	Entity::_num_properties = 6;
	Player::_num_properties = 17;
	Wall::_num_properties = 0;
	Altar::_num_properties = 0;
	Brazier::_num_properties = 0;
	Crusher::_num_properties = 0;
	Wind_Flow::_num_properties = 3;
	Particle_System::_num_properties = 10;
	Physics_Object::_num_properties = 11;
}
int Entity::_num_properties;
int Player::_num_properties;
int Wall::_num_properties;
int Altar::_num_properties;
int Brazier::_num_properties;
int Crusher::_num_properties;
int Wind_Flow::_num_properties;
int Particle_System::_num_properties;
int Physics_Object::_num_properties;
Entity_Type Player::_tag;
const char *Player::_type_name;
Entity_Type Wall::_tag;
const char *Wall::_type_name;
Entity_Type Altar::_tag;
const char *Altar::_type_name;
Entity_Type Brazier::_tag;
const char *Brazier::_type_name;
Entity_Type Crusher::_tag;
const char *Crusher::_type_name;
Entity_Type Wind_Flow::_tag;
const char *Wind_Flow::_type_name;
void ent_remove_base(Entity *entity) {
	switch(entity->tag) {
	case Entity_Type::_Player: {
			Player *derived = (Player *)entity->derived;
			ent_remove(derived);
		} break;
	case Entity_Type::_Wall: {
			Wall *derived = (Wall *)entity->derived;
			ent_remove(derived);
		} break;
	case Entity_Type::_Altar: {
			Altar *derived = (Altar *)entity->derived;
			ent_remove(derived);
		} break;
	case Entity_Type::_Brazier: {
			Brazier *derived = (Brazier *)entity->derived;
			ent_remove(derived);
		} break;
	case Entity_Type::_Crusher: {
			Crusher *derived = (Crusher *)entity->derived;
			ent_remove(derived);
		} break;
	case Entity_Type::_Wind_Flow: {
			Wind_Flow *derived = (Wind_Flow *)entity->derived;
			ent_remove(derived);
		} break;
	}
}
Entity *ent_create_from_name(const char *type_name) {
	if(!strcmp(type_name, "Player")) {
		return ent_create(Player)->base;
	}
	if(!strcmp(type_name, "Wall")) {
		return ent_create(Wall)->base;
	}
	if(!strcmp(type_name, "Altar")) {
		return ent_create(Altar)->base;
	}
	if(!strcmp(type_name, "Brazier")) {
		return ent_create(Brazier)->base;
	}
	if(!strcmp(type_name, "Crusher")) {
		return ent_create(Crusher)->base;
	}
	if(!strcmp(type_name, "Wind_Flow")) {
		return ent_create(Wind_Flow)->base;
	}

	return null;
}
Entity *ent_create_from_file(int index, int parity, int derived_index, const char *type_name) {
	Entity *entity = &entities[index];
	entities.filled[index] = true;
	entity->index = index;
	entity->parity = parity;
	entity->derived_index = derived_index;
	if(!strcmp(type_name, "Player")) {
		Player *derived = &Player::_list[derived_index];
		Player::_list.filled[derived_index] = true;
		derived->base = entity;
		entity->tag = Player::_tag;
		entity->type_name = Player::_type_name;
		entity->derived = derived;
		entity->po.owner = entity;
		ent_setup_default_state(entity);
		return entity;
	}
	if(!strcmp(type_name, "Wall")) {
		Wall *derived = &Wall::_list[derived_index];
		Wall::_list.filled[derived_index] = true;
		derived->base = entity;
		entity->tag = Wall::_tag;
		entity->type_name = Wall::_type_name;
		entity->derived = derived;
		entity->po.owner = entity;
		ent_setup_default_state(entity);
		return entity;
	}
	if(!strcmp(type_name, "Altar")) {
		Altar *derived = &Altar::_list[derived_index];
		Altar::_list.filled[derived_index] = true;
		derived->base = entity;
		entity->tag = Altar::_tag;
		entity->type_name = Altar::_type_name;
		entity->derived = derived;
		entity->po.owner = entity;
		ent_setup_default_state(entity);
		return entity;
	}
	if(!strcmp(type_name, "Brazier")) {
		Brazier *derived = &Brazier::_list[derived_index];
		Brazier::_list.filled[derived_index] = true;
		derived->base = entity;
		entity->tag = Brazier::_tag;
		entity->type_name = Brazier::_type_name;
		entity->derived = derived;
		entity->po.owner = entity;
		ent_setup_default_state(entity);
		return entity;
	}
	if(!strcmp(type_name, "Crusher")) {
		Crusher *derived = &Crusher::_list[derived_index];
		Crusher::_list.filled[derived_index] = true;
		derived->base = entity;
		entity->tag = Crusher::_tag;
		entity->type_name = Crusher::_type_name;
		entity->derived = derived;
		entity->po.owner = entity;
		ent_setup_default_state(entity);
		return entity;
	}
	if(!strcmp(type_name, "Wind_Flow")) {
		Wind_Flow *derived = &Wind_Flow::_list[derived_index];
		Wind_Flow::_list.filled[derived_index] = true;
		derived->base = entity;
		entity->tag = Wind_Flow::_tag;
		entity->type_name = Wind_Flow::_type_name;
		entity->derived = derived;
		entity->po.owner = entity;
		ent_setup_default_state(entity);
		return entity;
	}
	return null;
}
void copy_entity(Entity *source, Entity *dest) {
	if(source->tag != dest->tag) {
		assert(0, "copied entities must be of the same type");
		return;
	}
	dest->texture = source->texture;
	dest->spawn_position = source->spawn_position;
	dest->render_position = source->render_position;
	dest->render_size = source->render_size;
	dest->po.position = source->po.position;
	dest->po.velocity = source->po.velocity;
	dest->po.goal_velocity = source->po.goal_velocity;
	dest->po.velocity_ramp_speed = source->po.velocity_ramp_speed;
	dest->po.is_sensor = source->po.is_sensor;
	dest->po.size = source->po.size;
	dest->po.mass = source->po.mass;
	dest->po.inv_mass = source->po.inv_mass;
	dest->po.restitution = source->po.restitution;
	dest->po.groups = source->po.groups;
	dest->po.mask = source->po.mask;
	dest->test_float = source->test_float;
	if(!strcmp(source->type_name, "Player")) {
		Player *a = (Player *)source->derived;
		Player *b = (Player *)dest->derived;
		b->is_lit = a->is_lit;
		b->can_jump = a->can_jump;
		b->weight = a->weight;
		b->min_weight = a->min_weight;
		b->max_weight = a->max_weight;
		b->weight_factor0 = a->weight_factor0;
		b->weight_factor1 = a->weight_factor1;
		b->jump_speed = a->jump_speed;
		b->move_speed = a->move_speed;
		b->fall_multiplier = a->fall_multiplier;
		b->low_jump_multiplier = a->low_jump_multiplier;
		b->sword_texture = a->sword_texture;
		b->has_sword = a->has_sword;
		b->flame_on_wick = a->flame_on_wick;
		b->shrink_speed = a->shrink_speed;
		b->fire_system.texture = a->fire_system.texture;
		b->fire_system.position = a->fire_system.position;
		b->fire_system.position_spread = a->fire_system.position_spread;
		b->fire_system.velocity = a->fire_system.velocity;
		b->fire_system.velocity_spread = a->fire_system.velocity_spread;
		b->fire_system.angular_velocity = a->fire_system.angular_velocity;
		b->fire_system.angular_velocity_spread = a->fire_system.angular_velocity_spread;
		b->fire_system.lifetime = a->fire_system.lifetime;
		b->fire_system.lifetime_spread = a->fire_system.lifetime_spread;
		b->fire_system.spawn_rate = a->fire_system.spawn_rate;
		b->dead = a->dead;
	}
	if(!strcmp(source->type_name, "Wall")) {
		Wall *a = (Wall *)source->derived;
		Wall *b = (Wall *)dest->derived;
	}
	if(!strcmp(source->type_name, "Altar")) {
		Altar *a = (Altar *)source->derived;
		Altar *b = (Altar *)dest->derived;
	}
	if(!strcmp(source->type_name, "Brazier")) {
		Brazier *a = (Brazier *)source->derived;
		Brazier *b = (Brazier *)dest->derived;
	}
	if(!strcmp(source->type_name, "Crusher")) {
		Crusher *a = (Crusher *)source->derived;
		Crusher *b = (Crusher *)dest->derived;
	}
	if(!strcmp(source->type_name, "Wind_Flow")) {
		Wind_Flow *a = (Wind_Flow *)source->derived;
		Wind_Flow *b = (Wind_Flow *)dest->derived;
		b->flow_direction = a->flow_direction;
		b->flow_size = a->flow_size;
		b->flow_speed = a->flow_speed;
	}
}
void ent_reset_all_lists() {
not implimented yet sucka!!!!!
}
const char *entity_type_names[] = {
	"Player",
	"Wall",
	"Altar",
	"Brazier",
	"Crusher",
	"Wind_Flow",
};
