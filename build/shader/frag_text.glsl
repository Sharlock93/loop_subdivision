#version 450

out vec4 frag_color;
in vec4 color_out;
uniform sampler2D t;
uniform vec4 color;

void main() {
   /* frag_color = vec4(color.rgb, texture(t, out_tex.xy).r); */
}
