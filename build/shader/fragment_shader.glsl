#version 450
in vec4 color_out;
in vec2 tex_uv;
in vec3 normal;


layout( location = 10) uniform bool has_light = false;
out vec4 frag_color;
in vec4 frag_pos_world;

uniform sampler2D tex;
layout( location = 8) uniform bool has_texture = false;

vec3 light_pos = vec3(0, 5, 0);
vec4 diff_light_color = vec4(1, 1, 1, 1);
vec4 diffuse = vec4(1, 1, 1, 1);


void main() {

	frag_color = vec4(cos(frag_pos_world.x)*cos(frag_pos_world.y), cos(frag_pos_world.y)*cos(frag_pos_world.y), sin(frag_pos_world.z)*sin(frag_pos_world.z), 1);

	if(has_light) {

		float diff_comp = .5;
		vec3 light_dir = normalize(vec3(light_pos - frag_pos_world.xyz));
		float diff_am = max(dot(light_dir, normalize(normal)), 0);

		vec4 diff_light_color = diff_light_color*diff_am*diff_comp;
		diffuse = diff_light_color;
		diffuse.w = 1;

	}


	if(has_texture) {
		frag_color = texture(tex, tex_uv);
		frag_color.w = 1;
		frag_color = frag_color*(diffuse + vec4(0.2, 0.2, 0.2, 1));
	}

	/* frag_color = frag_color;//*(diffuse + vec4(0.2, 0.2, 0.2, 1)); */
}
