#version 130
#define SPOTLIGHT_CUT 0.999f    // valor limite: es inverso al radio del cono de luz

in vec3 position_eye, normal_eye;
in vec2 st;
uniform mat4 view_mat;

// fixed point light properties and spotlight direction
vec3 light_position_world  = vec3 (150.0, 0.0, 10.0);

vec3 Ls = vec3 (1.0, 1.0, 1.0); // white specular colour
vec3 Ld = vec3 (1.0, 1.0, 1.0); // dull white diffuse light colour
vec3 La = vec3 (0.0, 0.0, 0.0); // grey ambient colour
  
// surface reflectance
vec3 Ks = vec3 (1.0, 1.0, 1.0); // fully reflect specular light
vec3 Kd = vec3 (1.0, 1.0, 1.0); // diffuse surface reflectance
vec3 Ka = vec3 (1.0, 1.0, 1.0); // fully reflect ambient light
float specular_exponent = 300.0; // specular 'power'

out vec4 fragment_colour; // final colour of surface
void main () {
	// ambient intensity
	vec3 Ia = La * Ka;
    vec3 Id = vec3(0.0f, 0.0f, 0.0f);
    vec3 Is = vec3(0.0f, 0.0f, 0.0f);

    vec3 light_position_eye = vec3(view_mat * vec4 (light_position_world, 1.0));

    // diffuse intensity
    // raise light position to eye space
    vec3 distance_to_light_eye = light_position_eye - position_eye;
    vec3 direction_to_light_eye = normalize (distance_to_light_eye);
    float dot_prod = dot (direction_to_light_eye, normal_eye);
    dot_prod = max (dot_prod, 0.0);
    Id = Ld * Kd * dot_prod; // final diffuse intensity
    
    // specular intensity (blinn)
    vec3 surface_to_viewer_eye = normalize (-position_eye);
    vec3 half_way_eye = normalize (surface_to_viewer_eye + direction_to_light_eye);
    float dot_prod_specular = max (dot (half_way_eye, normal_eye), 0.0);
    float specular_factor = pow (dot_prod_specular, specular_exponent);
    Is = Ls * Ks * specular_factor; // final specular intensity
	
	// final colour
	vec4 texel = vec4(0.0f, 0.0f, 1.0f, 1.0f);
	//fragment_colour = 0.5 * vec4 (Is + Id + Ia, 1.0) + texel;
	fragment_colour = vec4 (Is + Id*texel.xyz + Ia*texel.xyz, 1.0);
}
