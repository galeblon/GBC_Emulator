attribute vec4 al_pos;
attribute vec4 al_color;
attribute vec2 al_texcoord;
uniform mat4 al_projview_matrix;

void main() {
	gl_Position = al_projview_matrix * al_pos;
}