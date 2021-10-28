#version 460

out vec4 out_colour;

uniform float overlayAlpha;

void main() {
	out_colour = vec4(0.0, 0.0, 0.0, overlayAlpha);
}
