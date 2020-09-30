#ifdef GL_ES
precision mediump float;
#endif
uniform sampler2D al_tex;

uniform ivec4 line[160];
uniform int  scale;

void main() {
	vec4 colour;
	uint pixel_n = gl_FragCoord.x/scale;
	colour.r = line[pixel_n].r/255.0;
	colour.g = line[pixel_n].g/255.0;
	colour.b = line[pixel_n].b/255.0;
	colour.a = line[pixel_n].a/255.0;
	gl_FragColor = color;
}