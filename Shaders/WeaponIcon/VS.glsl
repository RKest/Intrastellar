#version 460

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 texCoords;
layout (location = 3) in mat4 instanceTransform;

uniform mat4 transform;
uniform mat4 projection;

out vec2 v_texCoords;
out flat int v_texInx;

void main() {
	v_texCoords = texCoords;
	v_texInx = gl_InstanceID;
	// v_tex = texture(samps[gl_InstanceID], texCoords).rgb;
	vec3 v_fragPos = vec3(transform * instanceTransform * vec4(position, 1));
	gl_Position = projection * vec4(v_fragPos, 1.0);
}