/******************************************************************************\
| OpenGL 4 Example Code.                                                       |
| Accompanies written series "Anton's OpenGL 4 Tutorials"                      |
| Email: anton at antongerdelan dot net                                        |
| First version 27 Jan 2014                                                    |
| Copyright Dr Anton Gerdelan, Trinity College Dublin, Ireland.                |
| See individual libraries for separate legal notices                          |
|******************************************************************************|
| This demo uses the Assimp library to load a mesh from a file, and supports   |
| many formats. The library is VERY big and complex. It's much easier to write |
| a simple Wavefront .obj loader. I have code for this in other demos. However,|
| Assimp will load animated meshes, which will we need to use later, so this   |
| demo is a starting point before doing skinning animation                     |
\******************************************************************************/
#include <time.h>
#include <assimp/cimport.h> // C importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations
#include <GL/glew.h> // include GLEW and new version of GL on Windows
#include <GLFW/glfw3.h> // GLFW helper library
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <string>
#include "maths_funcs.h"
#include "gl_utils.h"
#include "tools.h"
#include "enemigo.h"
//#include "stb_image.h" // Sean Barrett's image loader - http://nothings.org/

#define GL_LOG_FILE "log/gl.log"
#define VERTEX_SHADER_FILE "shaders/test_vs.glsl"
#define FRAGMENT_SHADER_FILE "shaders/test_fs.glsl"
//#define MESH_FILE "monkey2.obj"
//#define MESH_FILE "cosa.obj"
//#define MESH_FILE "mallas/suzanne.obj"
//#define NUM_OBJ 6

// keep track of window size for things like the viewport and the mouse cursor
int g_gl_width = 1280;
int g_gl_height = 690;
GLFWwindow* g_window = NULL;


unsigned int index_mesh = 0;
char ar_models[9][64] = {"mallas/triangle.obj", "mallas/paper cup.obj", "mallas/deer2.obj", "mallas/MAMMOTH.obj", "mallas/Wolf.obj", "mallas/TUNA.obj", "mallas/M4A1.obj", "mallas/suzanne.obj", "mallas/ufo.dae"};


vec3 pos;

#define RANGOX 50.0
#define RANGOY 50.0
#define RANGOZ -10.0

int main(const int argc, const char *argv[]){
	 srand(time(NULL));
    int K = 3;
	int L = 0;
	int obj_rand[]={};
	int aux;
	bool found = false;
	int i = 0;
	

    if(argc != 3){
        fprintf(stderr, "./prog L mesh\n");
        exit(EXIT_FAILURE);
    }
    L = atoi(argv[1]);
    index_mesh = atoi(argv[2]); 
    unsigned long L3 = L;
 	vec3 *obj_pos_wor= (vec3*)malloc(sizeof(vec3)*L3);
    printf("creando %lu posiciones de mallas.....", L3); fflush(stdout);
 	for (int i = 0; i < L3; ++i)
 	{
        vec3 a;
        float r = (float)rand()/RAND_MAX;
        a.v[0] = r*RANGOX  -  (float)RANGOX/2.0;
        r = (float)rand()/RAND_MAX;
        a.v[1] = r*RANGOY  -  (float)RANGOY/2.0;
        r = (float)rand()/RAND_MAX;
        a.v[2] = r*RANGOZ; 
        obj_pos_wor[i] = a;
       
 	}
    printf("ok\n"); fflush(stdout);
    unsigned long NUM_OBJ = L3;
  
	restart_gl_log ();
	start_gl ();
    glfwSwapInterval(0);
	glEnable (GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc (GL_LESS); // depth-testing interprets a smaller value as "closer"
	glEnable (GL_CULL_FACE); // cull face
	glCullFace (GL_BACK); // cull back face
	glFrontFace (GL_CCW); // set counter-clock-wise vertex order to mean the front
	glClearColor (0.2, 0.2, 0.2, 1.0); // grey background to help spot mistakes
	glViewport (0, 0, g_gl_width, g_gl_height);


    /* objetos enemigos */
    //enemigo **e = (enemigo**)malloc(sizeof(enemigo*)*L3);
    enemigo *e = new enemigo((char*)ar_models[index_mesh]);
    
	
/*-------------------------------CREATE SHADERS-------------------------------*/
	GLuint shader_programme = create_programme_from_files (
		VERTEX_SHADER_FILE, FRAGMENT_SHADER_FILE
	);
	
	int model_mat_location = glGetUniformLocation (shader_programme, "model");

	#define ONE_DEG_IN_RAD (2.0 * M_PI) / 360.0 // 0.017444444
	// input variables
	float near = 0.1f; // clipping plane
	float far = 100.0f; // clipping plane
	float fov = 67.0f * ONE_DEG_IN_RAD; // convert 67 degrees to radians
	float aspect = (float)g_gl_width / (float)g_gl_height; // aspect ratio
	// matrix components
	float range = tan (fov * 0.5f) * near;
	float Sx = (2.0f * near) / (range * aspect + range * aspect);
	float Sy = near / range;
	float Sz = -(far + near) / (far - near);
	float Pz = -(2.0f * far * near) / (far - near);
	GLfloat proj_mat[] = {
		Sx, 0.0f, 0.0f, 0.0f,
		0.0f, Sy, 0.0f, 0.0f,
		0.0f, 0.0f, Sz, -1.0f,
		0.0f, 0.0f, Pz, 0.0f
	};
	
		
/*-------------------------------CREATE CAMERA--------------------------------*/
	float cam_speed = 8.0f; // 1 unit per second
	float cam_yaw_speed = 30.0f; // 10 degrees per second
	float cam_pos[] = {0.0f, 0.0f, 50.0f}; // don't start at zero, or we will be too close
	float cam_yaw = 0.0f; // y-rotation in degrees
	mat4 T = translate (identity_mat4 (), vec3 (-cam_pos[0], -cam_pos[1], -cam_pos[2]));
	mat4 R = rotate_y_deg (identity_mat4 (), -cam_yaw);
	mat4 view_mat = R * T;

	mat4 *model_mats = (mat4*)malloc(sizeof(mat4)*L3);
    printf("creando %lu matrices.....", L3); fflush(stdout);
	for (int i = 0; i < NUM_OBJ; i++) {
		model_mats[i] = translate (identity_mat4 (), obj_pos_wor[i]);
	}
    /*
	for (int i = 0; i < NUM_OBJ; i++) {
        e[i] = new enemigo((char*)ar_models[index_mesh]);
	}
    */
    printf("ok\n");
	
	int view_mat_location = glGetUniformLocation (shader_programme, "view");
	glUseProgram (shader_programme);
	glUniformMatrix4fv (view_mat_location, 1, GL_FALSE, view_mat.m);
	int proj_mat_location = glGetUniformLocation (shader_programme, "proj");
	glUseProgram (shader_programme);
	glUniformMatrix4fv (proj_mat_location, 1, GL_FALSE, proj_mat);

/*
	// load texture
	GLuint tex;
	//assert (load_texture ("skulluvmap.png", &tex));
	assert (load_texture ("textura.png", &tex));
	
	
	glEnable (GL_CULL_FACE); // cull face
	glCullFace (GL_BACK); // cull back face
	glFrontFace (GL_CCW); // GL_CCW for counter clock-wise
*/
    float *alturas = (float*)malloc(sizeof(float)*L3);
    float altura = 0.0f;
	for (int i = 0; i < NUM_OBJ; i++) {
        float r = (float)rand()/RAND_MAX;
        r = r*RANGOY  -  (float)RANGOY/2.0;
        alturas[i] = r;
    }
	while (!glfwWindowShouldClose (g_window)) {
		static double previous_seconds = glfwGetTime ();
		double current_seconds = glfwGetTime ();
		double elapsed_seconds = current_seconds - previous_seconds;
		previous_seconds = current_seconds;
	
		_update_fps_counter (g_window);
		// wipe the drawing surface clear
		glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport (0, 0, g_gl_width, g_gl_height);
		
		glUseProgram (shader_programme);
		
		for (int i = 0; i < NUM_OBJ; i++) {
            //mat4 rotate_y_deg (const mat4& m, float deg);
            alturas[i] += ((float)(rand()%2) - 0.5f)/100.0f;
            model_mats[i] = translate(identity_mat4(), obj_pos_wor[i] + vec3(0.0f, sin(alturas[i]), 0.0f));
			glUniformMatrix4fv (model_mat_location, 1, GL_FALSE, model_mats[i].m);
			//glDrawArrays(GL_TRIANGLES,0,e[i]->getnumvertices());
			glDrawArrays(GL_TRIANGLES,0,e->getnumvertices());
		}
		// update other events like input handling 
		glfwPollEvents ();

		// control keys
		bool cam_moved = false;
		if (glfwGetKey (g_window, GLFW_KEY_A)) {
			cam_pos[0] -= cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey (g_window, GLFW_KEY_D)) {
			cam_pos[0] += cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey (g_window, GLFW_KEY_UP)) {
			cam_pos[1] += cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey (g_window, GLFW_KEY_DOWN)) {
			cam_pos[1] -= cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey (g_window, GLFW_KEY_W)) {
			cam_pos[2] -= cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey (g_window, GLFW_KEY_S)) {
			cam_pos[2] += cam_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey (g_window, GLFW_KEY_Y)) {
            altura += 0.05;
		    model_mats[0] = translate (identity_mat4 (), obj_pos_wor[0] + vec3(0.0f, altura, 0.0f));
		}
		if (glfwGetKey (g_window, GLFW_KEY_LEFT)) {
			cam_yaw += cam_yaw_speed * elapsed_seconds;
			cam_moved = true;
		}
		if (glfwGetKey (g_window, GLFW_KEY_RIGHT)) {
			cam_yaw -= cam_yaw_speed * elapsed_seconds;
			cam_moved = true;
		}
		// update view matrix
		if (cam_moved) {
			mat4 T = translate (identity_mat4 (), vec3 (-cam_pos[0], -cam_pos[1], -cam_pos[2])); // cam translation
			mat4 R = rotate_y_deg (identity_mat4 (), -cam_yaw); // 
			mat4 view_mat = R * T;
			glUniformMatrix4fv (view_mat_location, 1, GL_FALSE, view_mat.m);
		}
		
		if (GLFW_PRESS == glfwGetKey (g_window, GLFW_KEY_ESCAPE)) {
			glfwSetWindowShouldClose (g_window, 1);
		}
		// put the stuff we've been drawing onto the display
		glfwSwapBuffers (g_window);
	}
	
	// close GL context and any other GLFW resources
	glfwTerminate();
	return 0;
}
