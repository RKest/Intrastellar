#version 460

out vec4 out_colour;

in vec3 v_colour;

void main() {
	out_colour = vec4(v_colour, 1.0);
}