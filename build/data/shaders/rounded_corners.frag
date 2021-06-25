in vec2 a_uv;

uniform vec2 u_dimensions;
uniform float u_radius;

void main(void) {
    vec2 coords = a_uv * u_dimensions;
    if (length(coords - vec2(0) < u_radius ||
        length(coords - vec2(0, u_dimensions.y) < u_radius ||
        length(coords - vec2(u_dimensions.x, 0) < u_radius ||
        length(coords - u_dimensions) < u_radius) {
        discard;
    }
    // Do everything else otherwise
}