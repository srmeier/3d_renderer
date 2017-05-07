#version 410

in vec3 color, pos_eye, normal_eye;
out vec4 frag_color;

uniform vec3 Lpos;

uniform mat4 view;

in vec2 texcoords;
uniform sampler2D basic_texture;
//layout(binding=0) uniform sampler2D basic_texture;
//texture2D(basic_texture, texcoords);

// the light
vec3 Ls = vec3(1.0, 1.0, 1.0); // white specular color
vec3 Ld = vec3(0.7, 0.7, 0.7); // dull white diffuse light color
vec3 La = vec3(0.2, 0.2, 0.2); // grey ambient color

// surface reflectance
vec3 Ks = vec3(1.0, 1.0, 1.0);  // fully reflect specular light
vec3 Kd = vec3(1.0, 0.5, 0.0);//color;  // orange diffuse surface reflectance
vec3 Ka = vec3(1.0, 1.0, 1.0);  // fully reflect ambient light
float specular_exponent = 100.0; // specular 'power'

void main() {
	Kd = vec3(texture(basic_texture, texcoords));

	// ambient intensity
	vec3 Ia = La * Ka;
	
	// diffuse intensity
	vec3 light_position_eye = vec3(view * vec4(Lpos, 1.0));
	vec3 dist_to_light_eye = light_position_eye - pos_eye;
	vec3 dir_to_light_eye  = normalize(dist_to_light_eye);
	float dot_prod = dot(dir_to_light_eye, normal_eye);
	dot_prod = max(dot_prod, 0.0);
	vec3 Id = Ld * Kd * dot_prod;
	
	// specular intensity
	vec3 reflection_eye = reflect(-dir_to_light_eye, normal_eye);
	vec3 surface_to_viewer_eye = normalize(-pos_eye);
	float dot_prod_specular = dot(reflection_eye, surface_to_viewer_eye);
	dot_prod_specular = max(dot_prod_specular, 0.0);
	float specular_factor = pow(dot_prod_specular, specular_exponent);
	vec3 Is = Ls * Ks * specular_factor; // final specular intensity

	// the final thing
	//frag_color = vec4(Is+Id+Ia , 1.0);
	frag_color = vec4(Kd, 1.0);
}
