#version 460

layout(location = 0) in vec3 vertex_position;
layout(location = 1) in vec4 vertex_colour;
layout(location = 2) in vec2 vertex_tex_coord;
layout(location = 3) in vec3 vertex_normal;

uniform mat4 projection_matrix;
uniform mat4 transformation_matrix;
uniform vec2 camera_position;
uniform float time;
uniform int zoom_level;

out vec4 position;
out vec4 colour;
out vec2 tex_coord;
out vec3 normal;

float scale_for_zoom_level() {
	float scale = 0.0f;
	if (zoom_level == 0) {
		scale = 1.0f;
	}
	else if (zoom_level < 0) {
		scale = 1.0f / (((zoom_level * 0.125f) * -1) + 1);
	}
	else if (zoom_level > 0) {
		scale = (zoom_level * 0.25f) + 1;
	}
	return scale;
}

float inverse_scale_for_zoom_level() {
	float scale = 0.0f;
	if (zoom_level == 0) {
		scale = 1.0f;
	}
	else if (zoom_level < 0) {
		scale = ((zoom_level * 0.125f) * -1) + 1;
	}
	else if (zoom_level > 0) {
		scale = 1.0f / ((zoom_level * 0.25f) + 1);
	}
	return scale;
}

void main(void) {
	position = projection_matrix * vec4((vertex_position * inverse_scale_for_zoom_level()) + vec3(-camera_position, 1), 1);

	colour = vertex_colour;  
	tex_coord = vertex_tex_coord;
	normal = vertex_normal;

	gl_Position = position;
}