#version 460

out vec4 out_colour;
uniform float alpha;

void main() {
	out_colour = vec4(1.0, 1.0, 1.0, alpha);
}
