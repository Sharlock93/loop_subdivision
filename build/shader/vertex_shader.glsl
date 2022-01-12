#version 450
layout( location = 0 ) in vec3 vp;
layout( location = 1 ) in vec4 color;
layout( location = 2 ) in vec2 uv;
layout( location = 9 ) in vec3 normals;

layout( location = 3 ) uniform mat4 model = mat4(1);
layout( location = 4 ) uniform mat4 camera = mat4(1);
layout( location = 5 ) uniform mat4 projection = mat4(1);
layout( location = 6 ) uniform bool use_uniform_color = true;
layout( location = 7 ) uniform vec4 uniform_color = vec4(1, 1, 1, 1);
layout( location = 8 ) uniform bool has_texture = false;

layout( location = 10) uniform bool has_light = false;


out vec4 color_out;
out vec2 tex_uv;
out vec4 frag_pos_world;
out vec3 normal;



void main() {

	vec4 pos_world = model*vec4(vp, 1);
	gl_Position = projection*camera*pos_world;
	frag_pos_world = pos_world;

	if(use_uniform_color) {
		color_out = pos_world; //uniform_color;
	} else {
		color_out = color;
	}

	tex_uv = uv;
	normal = normals;
}

