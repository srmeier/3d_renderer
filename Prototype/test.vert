#version 410

in vec3 vert_pos;
in vec3 norms;

in vec2 vt_coords;
out vec2 texcoords;

out vec3 color, pos_eye, normal_eye;

uniform mat4 model;
uniform mat4 view;
uniform mat4 proj;

void main() {
	texcoords = vt_coords;

	color = norms;
	
	pos_eye = vec3(view * model * vec4(vert_pos, 1.0));
	normal_eye = vec3(view * model * vec4(norms, 0.0));
	
	gl_Position = proj * view * model * vec4(vert_pos.x, vert_pos.y, vert_pos.z, 1.0);
}
