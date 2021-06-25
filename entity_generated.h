#ifndef GENERATED_H
#define GENERATED_H
enum Entity_Type {
	_Player,
	_Wall,
	_Altar,
	_Brazier,
	_Crusher,
	_Wind_Flow,
};
constexpr int num_entity_types = 6;
extern const char *entity_type_names[];
struct Entity;
void init_num_properties();
void assign_entity_type_tags();
void ent_remove_base(Entity *entity);
Entity *ent_create_from_name(const char *type_name);
const char *ent_type_name_from_tag(Entity_Type tag);
Property *get_derived_property_list(Entity *entity);
void copy_entity(Entity *source, Entity *dest);
void ent_reset_all_lists();
#endif
