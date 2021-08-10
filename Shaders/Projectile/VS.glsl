#version 460

layout (location = 0) in vec3 position;
layout (location = 2) in mat4 instanceTransform;

uniform mat4 transform;
uniform mat4 projection;
uniform vec3 colours[3];

out vec3 v_colour;

void main() {
	v_colour = colours[gl_InstanceID % 3];
	vec3 v_fragPos = vec3(transform * instanceTransform * vec4(position, 1.0));
	gl_Position = projection * vec4(v_fragPos, 1.0);
}