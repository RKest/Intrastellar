#version 460

layout (location = 0) in vec3 position;
layout (location = 2) in mat4 instanceTransform;

uniform mat4 transform;
uniform mat4 projection;

void main() {
	vec3 v_fragPos = vec3(transform * instanceTransform * vec4(position, 1.0));
	gl_Position = projection * vec4(v_fragPos, 1.0);
}