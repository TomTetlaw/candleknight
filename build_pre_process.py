import glob, os
import sys
import re

files = []
for file in os.listdir("./"):
	if file.endswith(".h") or file.endswith(".cpp"):
		files.append(file)

class Field:
	name = ""
	type = ""

types_dict = { "none" : [] }

def get_entity_type_names(file):
	current_type = ""
	with open(file) as f:
		while True:
			read_data = f.readline()
			if read_data == "":
				break
			if "@entity_type" in read_data:
				words = read_data.split(' ')
				current_type = words[1]
				types_dict[current_type] = []
			if "@generate_properties" in read_data:
				words = read_data.split(' ')
				current_type = words[1] + '@struct'
				types_dict[current_type] = []
			else:
				if current_type != "":
					if '@prop' in read_data:
						words = list(filter(None, read_data.split(' ')))
						words = [x.strip('\t') for x in words]
						type = words[0]
						name = words[1].replace(';', '')
						if name[0] == '*':
							name = name.replace('*', '')
							type += '*'
						new_field = Field()
						new_field.name = name
						new_field.type = type
						types_dict[current_type].append(new_field)

class Type:
	name = ''
	raw_name = ''
	is_struct = False

def process_entity_types():
	for file in files:
		get_entity_type_names(file)

	types = []
	for k, v in types_dict.items():
		if k != 'none':
			new_type = Type()
			if '@struct' in k:
				new_type.name = k.replace('@struct', '')
				new_type.raw_name = k
				new_type.is_struct = True
			else:
				new_type.name = k
				new_type.raw_name = k
			types.append(new_type)

	with open("entity_generated.cpp", 'w') as f:
		print("generating code:")

		# generate a carray for every entity type
		for t in types:
			if t.is_struct:
				continue
			f.write("Contiguous_Array<" + t.name + "> " + t.name + "::_list;\n")

		for t in types:
			f.write('start_property_block(%s)\n' % (t.name))
			prop_index = 0
			for field in types_dict[t.raw_name]:
				prop_type = 'error'
				struct_type_name = ''
				write_prop = True
				if field.type == 'int':
					prop_type = 'PROP_INT'
				elif  field.type == 'uint':
					prop_type = 'PROP_UINT'
				elif field.type == 'Vec2':
					prop_type = 'PROP_VEC2'
				elif field.type == 'bool':
					prop_type = 'PROP_BOOL'
				elif field.type == 'Texture*':
					prop_type = 'PROP_TEXTURE'
				elif field.type == 'float':
					prop_type = 'PROP_FLOAT'
				else:
					try:
						x = types_dict[field.type]
					except:
						prop_type = 'PROP_STRUCT'
						struct_type_name = field.type
					else:
						write_prop = False
						continue
				if write_prop:
					f.write('declare_property(%s, \"%s\", %s, %s, %d)\n' % (t.name, struct_type_name, field.name, prop_type, prop_index))
					prop_index += 1
			f.write('end_property_block()\n')

		f.write('Property_List _property_lists[] = {\n')
		for t in types:
			f.write('\t{ \"%s\", %s::_properties },\n' % (t.name, t.name))
		f.write('};\n')
		f.write('int _num_property_lists = %d;\n' % (len(types)))		

		f.write('Property *get_derived_property_list(Entity *entity) {\n')
		for t in types:
			if t.is_struct:
				continue
			f.write('\tif (entity->tag == _%s) {\n' % (t.name))
			f.write('\t\treturn %s::_properties;\n' % (t.name))
			f.write('\t}\n')
		f.write('\treturn null;\n')
		f.write('}\n')

		# generate a function to initialize all the lists
		f.write("void initialize_entity_lists() {\n")
		for t in types:
			if t.is_struct:
				continue
			f.write("\tcarray_init(&" + t.name + "::_list, entities_per_list);\n")
		f.write("}\n")
	
		# generate a function to free all the lists
		f.write("void free_entity_lists() {\n")
		for t in types:
			if t.is_struct:
				continue
			f.write("\tcarray_free(&" + t.name + "::_list);\n")
		f.write("}\n")

		# generate a function to assign the correct Entity_Type value to each entity type
		f.write("void assign_entity_type_tags() {\n")
		for t in types:
			if t.is_struct:
				continue
			f.write("\t" + t.name + "::_tag " + "= Entity_Type::_" + t.name + ";\n")
			f.write("\t" + t.name + "::_type_name = \"" + t.name + "\";\n")
		f.write("}\n")

		f.write("void init_num_properties() {\n")
		for t in types:
			name = t.name
			if t.is_struct:
				name = name + "@struct"
			f.write("\t" + t.name + "::_num_properties " + "= " + str(len(types_dict[name])) + ";\n")
		f.write("}\n")

		for t in types:
			f.write("int " + t.name + "::_num_properties" + ";\n")

		# generate the declarations for entity tags
		for t in types:
			if t.is_struct:
				continue
			f.write("Entity_Type " + t.name + "::_tag;\n")
			f.write("const char *" + t.name + "::_type_name;\n")

		f.write("void ent_remove_base(Entity *entity) {\n")
		f.write("\tswitch(entity->tag) {\n");
		for t in types:
			if t.is_struct:
				continue
			f.write("\tcase Entity_Type::_" + t.name + ": {\n")
			f.write("\t\t\t" + t.name + " *derived = (" + t.name + " *)entity->derived;\n")
			f.write("\t\t\tent_remove(derived);\n")
			f.write("\t\t} break;\n")
		f.write("\t}\n")
		f.write("}\n")

		f.write("Entity *ent_create_from_name(const char *type_name) {\n")
		for t in types:
			if t.is_struct:
				continue
			f.write("\tif(!strcmp(type_name, \"" + t.name + "\")) {\n")
			f.write("\t\treturn ent_create(" + t.name + ")->base;\n")
			f.write("\t}\n")
		f.write("\n\treturn null;\n")
		f.write("}\n")

		f.write("Entity *ent_create_from_file(int index, int parity, int derived_index, const char *type_name) {\n")
		f.write("\tEntity *entity = &entities[index];\n")
		f.write("\tentities.filled[index] = true;\n")
		f.write("\tentity->index = index;\n")
		f.write("\tentity->parity = parity;\n")
		f.write("\tentity->derived_index = derived_index;\n")
		for t in types:
			if t.is_struct:
				continue
			f.write("\tif(!strcmp(type_name, \"" + t.name + "\")) {\n")
			f.write("\t\t" + t.name + " *derived = &" + t.name + "::_list[derived_index];\n")
			f.write("\t\t" + t.name + "::_list.filled[derived_index] = true;\n")
			f.write("\t\tderived->base = entity;\n")
			f.write("\t\tentity->tag = " + t.name + "::_tag;\n")
			f.write("\t\tentity->type_name = " + t.name + "::_type_name;\n")
			f.write("\t\tentity->derived = derived;\n")
			f.write("\t\tentity->po.owner = entity;\n")
			f.write("\t\tent_setup_default_state(entity);\n")
			f.write("\t\treturn entity;\n")
			f.write("\t}\n")
		f.write("\treturn null;\n")
		f.write("}\n")

		f.write("void copy_entity(Entity *source, Entity *dest) {\n")
		f.write("\tif(source->tag != dest->tag) {\n")
		f.write("\t\tassert(0, \"copied entities must be of the same type\");\n")
		f.write("\t\treturn;\n")
		f.write("\t}\n")
	
		def write_props(prefix, source_name, dest_name, props):
			for p in props:
				try:
					t = types_dict[p.type + '@struct']
					write_props(prefix, "%s%s." % (source_name, p.name), "%s%s." % (dest_name, p.name), t)
				except:
					f.write("%s%s%s = %s%s;\n" % (prefix, dest_name, p.name, source_name, p.name))
			
		props = types_dict['Entity@struct']
		write_props("\t", "source->", "dest->", props)
		for t in types:
			if t.is_struct:
				continue
			f.write("\tif(!strcmp(source->type_name, \"" + t.name + "\")) {\n")
			f.write("\t\t%s *a = (%s *)source->derived;\n" % (t.name, t.name))
			f.write("\t\t%s *b = (%s *)dest->derived;\n" % (t.name, t.name))
			props = types_dict[t.name]
			write_props("\t\t", "a->", "b->", props)
			f.write("\t}\n")
		f.write("}\n")

		f.write("void ent_reset_all_lists() {\n")
		f.write("not implimented yet sucka!!!!!\n");
		f.write("}\n")

		f.write("const char *entity_type_names[] = {\n")
		for t in types:
			if t.is_struct:
				continue
			f.write("\t\"" + t.name + "\",\n")
		f.write("};\n")

	with open("entity_generated.h", 'w') as f:
		f.write("#ifndef GENERATED_H\n")
		f.write("#define GENERATED_H\n")

		# generate an enum entry for each entity type
		f.write("enum Entity_Type {\n")
		for t in types:
			if t.is_struct:
				continue
			f.write("\t_" + t.name + ",\n")
		f.write("};\n")

		num = 0
		for t in types:
			if t.is_struct:
				continue
			num += 1
		f.write("constexpr int num_entity_types = " + str(num) + ";\n")

		f.write("extern const char *entity_type_names[];\n")

		f.write("struct Entity;\n")
		f.write("void init_num_properties();\n")
		f.write("void assign_entity_type_tags();\n")
		f.write("void ent_remove_base(Entity *entity);\n")
		f.write("Entity *ent_create_from_name(const char *type_name);\n")
		f.write("const char *ent_type_name_from_tag(Entity_Type tag);\n")
		f.write('Property *get_derived_property_list(Entity *entity);\n')
		f.write("void copy_entity(Entity *source, Entity *dest);\n")
		f.write("void ent_reset_all_lists();\n")
		f.write("#endif\n")

		print("%d entity types" % (num))
		print("%d non-entity types" % (len(types) - num))
		print("%d total" % (len(types)))


process_entity_types()