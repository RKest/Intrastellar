#version 460

layout (location = 0) in vec2 position;

uniform mat4 transform;
uniform mat4 projection;

void main() {
	vec3 v_fragPos = vec3(transform * vec4(position, 0.0, 1.0));
	gl_Position = projection * vec4(v_fragPos, 1.0);
}