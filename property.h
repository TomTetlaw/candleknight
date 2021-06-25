#ifndef PROPERTY_H
#define PROPERTY_H

enum Property_Type {
    PROP_INT,
    PROP_UINT,
    PROP_FLOAT,
    PROP_BOOL,
    PROP_TEXTURE,
    PROP_VEC2,
    PROP_STRUCT,
};

struct Property {
    const char *name = null;
    const char *struct_type_name = null;
    Property_Type type;
    int offset;
    int index;
};

struct Property_List {
    const char *name;
    Property *properties;
};

Property *find_property_list(const char *type_name);

#define declare_property_block() static Property _properties[]; static int _num_properties

#define start_property_block(type_name) Property type_name::_properties[] = {
#define end_property_block() { null, "", (Property_Type)0, 0 } };
#define declare_property(type_name, struct_type_name, var, type, index) { #var, struct_type_name, type, offsetof(type_name, var), index },

#endif