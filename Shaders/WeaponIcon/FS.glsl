#version 460


//Same in thie file:///home/max/Documents/Intrastellar/Headers/_config.h
#define WEAPONS_NO_WEAOPNS 10

in vec2 v_texCoords;
in flat int v_texInx;

out vec4 out_colour;

uniform int chosenWeaponInx;
uniform float overlayAlpha;
uniform sampler2D samps[WEAPONS_NO_WEAOPNS];

void main() {
	if(chosenWeaponInx == v_texInx)
	{
		vec3 tex = texture(samps[v_texInx], v_texCoords).rgb;
		vec3 invertedTex = vec3(1.0 - tex.r, 1.0 - tex.g, 1.0 - tex.b);
		out_colour = vec4(invertedTex, overlayAlpha);
	}
	else
	{
		vec3 tex = texture(samps[v_texInx], v_texCoords).rgb;
		out_colour = vec4(tex, overlayAlpha);
	}
}