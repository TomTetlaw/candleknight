#version 460

in vec4 position;
in vec4 colour;
in vec2 tex_coord;
in vec3 normal;

uniform sampler2D diffuse_texture;
uniform float time;

out vec4 frag_colour;

void main(void) {
	float texture_sample = texture(diffuse_texture, tex_coord).r;
	frag_colour = colour * vec4(1, 1, 1, texture_sample);
}