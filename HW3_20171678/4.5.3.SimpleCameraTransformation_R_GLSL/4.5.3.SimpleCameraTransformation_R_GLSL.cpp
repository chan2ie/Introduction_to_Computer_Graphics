#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>

// Begin of shader setup
#include "Shaders/LoadShaders.h"
GLuint h_ShaderProgram; // handle to shader program
GLint loc_ModelViewProjectionMatrix, loc_primitive_color; // indices of uniform variables

void prepare_shader_program(void) {
	ShaderInfo shader_info[3] = {
		{ GL_VERTEX_SHADER, "Shaders/simple.vert" },
		{ GL_FRAGMENT_SHADER, "Shaders/simple.frag" },
		{ GL_NONE, NULL }
	};

	h_ShaderProgram = LoadShaders(shader_info);
	glUseProgram(h_ShaderProgram);

	loc_ModelViewProjectionMatrix = glGetUniformLocation(h_ShaderProgram, "u_ModelViewProjectionMatrix");
	loc_primitive_color = glGetUniformLocation(h_ShaderProgram, "u_primitive_color");
}
// End of shader setup

// Begin of geometry setup
#include "4.5.3.GeometrySetup.h"
// End of geometry setup

// Begin of Callback function definitions

// include glm/*.hpp only if necessary
// #include <glm/glm.hpp> 
#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, lookAt, perspective, etc.v

#define TO_RADIAN 0.01745329252f  
#define TO_DEGREE 57.295779513f

typedef struct {
	glm::vec3 prp, vrp, vup; // in this example code, make vup always equal to the v direction.
	float fov_y, aspect_ratio, near_clip, far_clip, zoom_factor;
} CAMERA;

typedef struct {
	int x, y, w, h;
} VIEWPORT;

typedef enum {
	VIEW_WORLD, VIEW_SQUARE, VIEW_TIGER, VIEW_COW
} VIEW_MODE;

#define NUMBER_OF_CAMERAS 5

CAMERA camera[NUMBER_OF_CAMERAS];
VIEWPORT viewport;

// ViewProjectionMatrix = ProjectionMatrix * ViewMatrix
glm::mat4 ViewProjectionMatrix[NUMBER_OF_CAMERAS], ViewMatrix[NUMBER_OF_CAMERAS], ProjectionMatrix[NUMBER_OF_CAMERAS];
// ModelViewProjectionMatrix = ProjectionMatrix * ViewMatrix * ModelMatrix
glm::mat4 ModelViewProjectionMatrix; // This one is sent to vertex shader when it is ready.

void create_camera(int, glm::vec3, glm::vec3, glm::vec3, float, float);
void camera_pan(int , int , int );
void camera_tilt(int, int, int);

void print_mat4(const char *string, glm::mat4 M) {
	fprintf(stdout, "\n***** %s ******\n", string);
	for (int i = 0; i < 4; i++)
		fprintf(stdout, "*** COL[%d] (%f, %f, %f, %f)\n", i, M[i].x, M[i].y, M[i].z, M[i].w);
	fprintf(stdout, "**************\n\n");
}

int current_camera = 0;
int cam_mode = 0;
float rotation_angle_cow;
VIEW_MODE view_mode;

int wolf_timer = 0;
bool wolf_moving = true;
int tiger_timer = 0;
bool tiger_moving = true;
int ben_timer = 0;
bool ben_moving = true;
int spider_timer = 0;
bool spider_moving = true;

void display_camera(int camera_index) {
	// should optimize this dispaly function further to reduce the amount of floating-point operations.
	glm::mat4 MatRotAxes;

	glViewport(viewport.x, viewport.y, viewport.w, viewport.h);
	
	// At this point, the matrix ViewProjectionMatrix has been properly set up.
	
	ModelViewProjectionMatrix = glm::scale(ViewProjectionMatrix[camera_index], glm::vec3(5.0f, 5.0f, 5.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glLineWidth(5.0f);
	draw_axes(); // draw the WC axes.
	glLineWidth(1.0);

	ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix[camera_index], glm::vec3(-12.5f, -12.5f,-0.01f));
	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(25.0f, 25.0f, 25.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	draw_object(OBJECT_SQUARE16, 20 / 255.0f, 90 / 255.0f, 50 / 255.0f); //  draw the floor.
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

	MatRotAxes = glm::rotate(glm::mat4(1.0f), 90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
 
	ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix[camera_index], glm::vec3(-5.0f, -5.0f, 0.05f));
	ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, 135.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix *= MatRotAxes;
	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(0.04f, 0.04f, 0.04f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_object(OBJECT_GODZILA, 0 / 255.0f, 0 / 255.0f, 255 / 255.0f); // Blue

	ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix[camera_index], glm::vec3(5.0f, 5.0f, 0.05f));
	ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, -135.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(0.3f, 0.3f, 0.3f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_object(OBJECT_DRAGON, 255.0f / 255.0f, 0 / 255.0f, 0.0f / 255.0f);

	ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix[camera_index], glm::vec3(2.0f, -3.0f, 0.05f));
	ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, 110.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, 15.0f * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewProjectionMatrix *= MatRotAxes;
	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_object(OBJECT_BIKE, 0 / 255.0f, 255.0f / 255.0f, 0.0f / 255.0f);

	ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix[camera_index], glm::vec3(-2.5f, 5.0f, 5.0f));
	ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, 75.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, 45.0f * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix *= MatRotAxes;
	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(0.5f, 0.5f, 0.5f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_object(OBJECT_IRONMAN, 255.0f / 255.0f, 0 / 255.0f, 0.0f / 255.0f);

	ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix[camera_index], glm::vec3(8.0f, -8.2f, 0.8f));
	ModelViewProjectionMatrix *= MatRotAxes;
	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(2.0f, 2.0f, 2.0f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_object(OBJECT_COW, 255.0f / 255.0f, 0 / 255.0f, 255.0 / 255.0f); 

	float wolf_angle = wolf_timer;
	float wolf_x = cos(wolf_angle * TO_RADIAN) * 3;
	float wolf_y = sin(wolf_angle * TO_RADIAN) * 3;

	ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix[camera_index], glm::vec3(8.0f + wolf_x, -8.2f + wolf_y, 0.0f));
	ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, (wolf_angle + 180) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix *= MatRotAxes;
	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(1.5f, 1.5f, 1.5f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_wolf(255.0f / 255.0f, 255.0f / 255.0f, 0.0 / 255.0f);

	float spider_angle = spider_timer * TO_RADIAN;
	float spider_z = cos(spider_angle) * 3 + 3.7;

	ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix[camera_index], glm::vec3(0.0f, 0.0f, spider_z));
	ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, spider_angle * 3, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(0.8, 0.8f, 0.8f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_spider(255.0f / 255.0f, 255.0f / 255.0f, 255.0f / 255.0f);

	float ben_x = sin(ben_timer * TO_RADIAN) * 4;
	float ben_y = ben_x;

	float ben_angle;

	if (ben_timer % 360 < 80) {
		ben_angle = 90;
	}
	else if (ben_timer % 360 < 100) {
		ben_angle = 90 + (ben_timer % 360 - 80) * 9;
	}
	else if (ben_timer % 360 < 260) {
		ben_angle = 270;
	}
	else if (ben_timer % 360 < 280) {
		ben_angle = 270 + (ben_timer % 360 - 260) * 9;
	}
	else ben_angle = 90;

	ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix[camera_index], glm::vec3(ben_x, ben_y, 0.0f));
	ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, (ben_angle + 45) * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, -90.0f * TO_RADIAN, glm::vec3(1.0f, 0.0f, 0.0f));
	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(1.5f, 1.5f, 1.5f));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_ben(0.0f / 255.0f, 255.0f / 255.0f, 0.0 / 255.0f);

	float tiger_angle = tiger_timer * TO_RADIAN;
	float tiger_size = 0.02 + sin(tiger_angle) * 0.01;

	ModelViewProjectionMatrix = glm::translate(ViewProjectionMatrix[camera_index], glm::vec3(-5.0, 4.0, 0.0f));
	ModelViewProjectionMatrix = glm::rotate(ModelViewProjectionMatrix, tiger_angle, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelViewProjectionMatrix = glm::scale(ModelViewProjectionMatrix, glm::vec3(tiger_size, tiger_size, tiger_size));
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_tiger(255.0f / 255.0f, 155.0f / 255.0f, 0.0 / 255.0f);

}

void display(void) {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	display_camera(current_camera);
	glutSwapBuffers();
}

unsigned int leftbutton_pressed = 0, rotation_mode_cow = 1, timestamp_cow = 0;
int prevx, prevy;

void timer_scene(int value) {
	rotation_angle_cow = (float)(timestamp_cow);
	glutPostRedisplay();

	timestamp_cow = (timestamp_cow + 1) % 360;
	if (wolf_moving) {
		cur_frame_wolf = (cur_frame_wolf + 1) % N_WOLF_FRAMES;
		wolf_timer = (wolf_timer + 1) % UINT_MAX;
	}
	if (tiger_moving) {
		cur_frame_tiger = (cur_frame_tiger + 1) % N_TIGER_FRAMES;
		tiger_timer = (tiger_timer + 1) % UINT_MAX;
	}
	if (ben_moving) {
		cur_frame_ben = (cur_frame_ben + 1) % N_BEN_FRAMES;
		ben_timer = (ben_timer + 1) % UINT_MAX;
	}
	if (spider_moving) {
		cur_frame_spider = (cur_frame_spider + 1) % N_SPIDER_FRAMES;
		spider_timer = (spider_timer + 1) % UINT_MAX;
	}
	if (rotation_mode_cow)
	glutTimerFunc(20, timer_scene, 0);
}

void mousepress(int button, int state, int x, int y)  {
	if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_DOWN)) {
		prevx = x, prevy = y;
		leftbutton_pressed = 1;
		glutPostRedisplay();
	}
	else if ((button == GLUT_LEFT_BUTTON) && (state == GLUT_UP)) {	
		leftbutton_pressed = 0;
		glutPostRedisplay();
	}
}

#define CAM_ROT_SENSITIVITY 0.15f
void motion_1(int x, int y) {
	glm::mat4 mat4_tmp;
	glm::vec3 vec3_tmp;
	float delx, dely;

	if (leftbutton_pressed) {
		delx = (float)(x - prevx), dely = -(float)(y - prevy);
		prevx = x, prevy = y;

		mat4_tmp = glm::translate(glm::mat4(1.0f), camera[0].vrp);
		mat4_tmp = glm::rotate(mat4_tmp, CAM_ROT_SENSITIVITY*delx*TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
		mat4_tmp = glm::translate(mat4_tmp, -camera[0].vrp);

 		camera[0].prp = glm::vec3(mat4_tmp*glm::vec4(camera[0].prp, 1.0f));
 		camera[0].vup = glm::vec3(mat4_tmp*glm::vec4(camera[0].vup, 0.0f));

		vec3_tmp = glm::cross(camera[0].vup, camera[0].vrp - camera[0].prp);
		mat4_tmp = glm::translate(glm::mat4(1.0f), camera[0].vrp);
		mat4_tmp = glm::rotate(mat4_tmp, CAM_ROT_SENSITIVITY*dely*TO_RADIAN, vec3_tmp);
		mat4_tmp = glm::translate(mat4_tmp, -camera[0].vrp);

 		camera[0].prp = glm::vec3(mat4_tmp*glm::vec4(camera[0].prp, 1.0f));
	 	camera[0].vup = glm::vec3(mat4_tmp*glm::vec4(camera[0].vup, 0.0f));

		ViewMatrix[0] = glm::lookAt(camera[0].prp, camera[0].vrp, camera[0].vup);

		ViewProjectionMatrix[0] = ProjectionMatrix[0] * ViewMatrix[0];
		glutPostRedisplay();
	}
}

void motion_2(int x, int y) {
	glm::mat4 mat4_tmp;
	glm::vec3 vec3_tmp;
	float delx, dely;

	if (leftbutton_pressed) {
		delx = (float)(x - prevx), dely = -(float)(y - prevy);
		prevx = x, prevy = y;

		mat4_tmp = glm::translate(glm::mat4(1.0f), camera[0].vrp);
	 	mat4_tmp = glm::rotate(mat4_tmp, CAM_ROT_SENSITIVITY*delx*TO_RADIAN, camera[0].vup);
		mat4_tmp = glm::translate(mat4_tmp, -camera[0].vrp);

		camera[0].prp = glm::vec3(mat4_tmp*glm::vec4(camera[0].prp, 1.0f));
		camera[0].vup = glm::vec3(mat4_tmp*glm::vec4(camera[0].vup, 0.0f));

		vec3_tmp = glm::cross(camera[0].vup, camera[0].vrp - camera[0].prp);

		mat4_tmp = glm::translate(glm::mat4(1.0f), camera[0].vrp);
		mat4_tmp = glm::rotate(mat4_tmp, CAM_ROT_SENSITIVITY*dely*TO_RADIAN, vec3_tmp);
		mat4_tmp = glm::translate(mat4_tmp, -camera[0].vrp);

	 	camera[0].prp = glm::vec3(mat4_tmp*glm::vec4(camera[0].prp, 1.0f));
	 	camera[0].vup = glm::vec3(mat4_tmp*glm::vec4(camera[0].vup, 0.0f));

		ViewMatrix[0] = glm::lookAt(camera[0].prp, camera[0].vrp, camera[0].vup);

		ViewProjectionMatrix[0] = ProjectionMatrix[0] * ViewMatrix[0];
		glutPostRedisplay();
	}
}

void motion_3(int x, int y) {
	glm::mat4 mat4_tmp;
	glm::vec3 vec3_right, vec3_up, vec3_view, vec3_tmp;
	float delx, dely, length;

	if (leftbutton_pressed) {
		delx = (float)(x - prevx), dely = -(float)(y - prevy);
		prevx = x, prevy = y;
		length = sqrtf(delx*delx + dely*dely);
		if (length == 0.0f) return;
		
	 	vec3_view = glm::vec3(-ViewMatrix[0][0].z, -ViewMatrix[0][1].z, -ViewMatrix[0][2].z); // -n vector		
		vec3_up = camera[0].vup; // v vector
		vec3_right = glm::vec3(ViewMatrix[0][0].x, ViewMatrix[0][1].x, ViewMatrix[0][2].x); // u vector

		vec3_tmp = delx*vec3_right + dely*vec3_up;
		vec3_tmp = glm::cross(vec3_tmp, vec3_view); // the rotation axis
	
		mat4_tmp = glm::translate(glm::mat4(1.0f), camera[0].vrp);
		mat4_tmp = glm::rotate(mat4_tmp, CAM_ROT_SENSITIVITY*length*TO_RADIAN, vec3_tmp);
		mat4_tmp = glm::translate(mat4_tmp, -camera[0].vrp);

		camera[0].prp = glm::vec3(mat4_tmp*glm::vec4(camera[0].prp, 1.0f));
		camera[0].vup = glm::vec3(mat4_tmp*glm::vec4(camera[0].vup, 0.0f)); // vup is a vector.

		ViewMatrix[0] = glm::lookAt(camera[0].prp, camera[0].vrp, camera[0].vup);
 		camera[0].vup = glm::vec3(ViewMatrix[0][0].y, ViewMatrix[0][1].y, ViewMatrix[0][2].y); // vup may have changed slightly.
		ViewProjectionMatrix[0] = ProjectionMatrix[0] * ViewMatrix[0];
		glutPostRedisplay();
	}
}

#define CAM_FAR_CLIP_DISTANCE_STEP	2.0f
#define CAM_MAX_FAR_CLIP_DISTANCE	100.0f
#define CAM_ZOOM_STEP			0.05f
#define CAM_MAX_ZOOM_IN_FACTOR	0.25f
#define CAM_MAX_ZOOM_OUT_FACTOR	2.00f

#define CAM_PAN_STEP			0.05f

#define PAN_MODE				0
#define ROT_MODE				1

void keyboard(unsigned char key, int x, int y) {
	if (key == 27) { // ESC key
		glutLeaveMainLoop(); // incur destuction callback for cleanups.
		return;
	}
	switch (key) {
	case 'w':
		if (current_camera == 4 && cam_mode == PAN_MODE) {
			camera_pan(current_camera, 2, 1);
		}
		if ((current_camera == 4 && cam_mode == ROT_MODE) || current_camera == 0) {
			camera_tilt(current_camera, 2, 1);
		}
		break;
	case 'a':
		if (current_camera == 4 && cam_mode == PAN_MODE) {
			camera_pan(current_camera, 1, -1);
		}
		if ((current_camera == 4 && cam_mode == ROT_MODE) || current_camera == 0) {
			camera_tilt(current_camera, 1, -1);
		}
		break;
	case 's':
		if (current_camera == 4 && cam_mode == PAN_MODE) {
			camera_pan(current_camera, 2, -1);
		}
		if ((current_camera == 4 && cam_mode == ROT_MODE) || current_camera == 0) {
			camera_tilt(current_camera, 2, -1);
		}
		break;
	case 'd':
		if (current_camera == 4 && cam_mode == PAN_MODE) {
			camera_pan(current_camera, 1, 1);
		}
		if ((current_camera == 4 && cam_mode == ROT_MODE) || current_camera == 0) {
			camera_tilt(current_camera, 1, 1);
		}
		break;
	case '1':
		current_camera = 0;
		break;
	case '2':
		current_camera = 1;
		break;
	case '3':
		current_camera = 2;
		break;
	case '4':
		current_camera = 3;
		break;
	case '5':
		current_camera = 4;
	case 'v':
		wolf_moving = !wolf_moving;
		break;
	case 'b':
		tiger_moving = !tiger_moving;
		break;
	case 'n':
		spider_moving = !spider_moving;
		break;
	case 'm':
		ben_moving = !ben_moving;
		break;
	case 'p':
		cam_mode = PAN_MODE;
		break;
	case 'r':
		cam_mode = ROT_MODE;
		break;
	}
	glutPostRedisplay();
}

void special(int key, int x, int y) {
	switch (key) {
	case GLUT_KEY_UP:
		if (current_camera == 4 && cam_mode == PAN_MODE) {
			camera_pan(current_camera, 3, -1);
		}
		if (current_camera == 4 && cam_mode == ROT_MODE) {
			camera_tilt(current_camera, 3, 1);
		}
		break;
	case GLUT_KEY_DOWN:
		if (current_camera == 4 && cam_mode == PAN_MODE) {
			camera_pan(current_camera, 3, 1);
		}
		if (current_camera == 4 && cam_mode == ROT_MODE) {
			camera_tilt(current_camera, 3, -1);
		}
		break;
	case GLUT_KEY_LEFT:
		if (current_camera != 4) break;
		camera[4].zoom_factor -= CAM_ZOOM_STEP;
		if (camera[4].zoom_factor < CAM_MAX_ZOOM_IN_FACTOR)
			camera[4].zoom_factor = CAM_MAX_ZOOM_IN_FACTOR;
		ProjectionMatrix[4] = glm::perspective(camera[4].zoom_factor * camera[4].fov_y*TO_RADIAN, camera[4].aspect_ratio, camera[4].near_clip, camera[4].far_clip);
		ViewProjectionMatrix[4] = ProjectionMatrix[4] * ViewMatrix[4];
		glutPostRedisplay();
		break;
	case GLUT_KEY_RIGHT:
		if (current_camera != 4) break;
		camera[4].zoom_factor += CAM_ZOOM_STEP;
		if (camera[4].zoom_factor > CAM_MAX_ZOOM_OUT_FACTOR)
			camera[4].zoom_factor = CAM_MAX_ZOOM_OUT_FACTOR;
		ProjectionMatrix[4] = glm::perspective(camera[4].zoom_factor * camera[4].fov_y*TO_RADIAN, camera[4].aspect_ratio, camera[4].near_clip, camera[4].far_clip);
		ViewProjectionMatrix[4] = ProjectionMatrix[4] * ViewMatrix[4];
		glutPostRedisplay();
		break;
	}
}

void camera_pan(int index, int axes, int direction) {
	glm::vec3 vector;
	switch (axes) {
	case 1: // u
		vector = glm::vec3(ViewMatrix[index][0].x, ViewMatrix[index][1].x, ViewMatrix[index][2].x);
		break;
	case 2: // v
		vector = glm::vec3(ViewMatrix[index][0].y, ViewMatrix[index][1].y, ViewMatrix[index][2].y);
		break;
	case 3: // n
		vector = glm::vec3(ViewMatrix[index][0].z, ViewMatrix[index][1].z, ViewMatrix[index][2].z);
		break;
	default:
		return;
	}

	camera[index].prp.x += direction * vector.x * CAM_PAN_STEP;
	camera[index].prp.y += direction * vector.y * CAM_PAN_STEP;
	camera[index].prp.z += direction * vector.z * CAM_PAN_STEP;
	camera[index].vrp.x += direction * vector.x * CAM_PAN_STEP;
	camera[index].vrp.y += direction * vector.y * CAM_PAN_STEP;
	camera[index].vrp.z += direction * vector.z * CAM_PAN_STEP;

	int i = index;
	
	ViewMatrix[index] = glm::lookAt(camera[index].prp, camera[index].vrp, camera[index].vup);
	camera[index].vup = glm::vec3(ViewMatrix[index][0].y, ViewMatrix[index][1].y, ViewMatrix[index][2].y);

	ProjectionMatrix[i] = glm::perspective(camera[i].zoom_factor * camera[i].fov_y * TO_RADIAN, camera[i].aspect_ratio,
		camera[i].near_clip, camera[i].far_clip);
	ViewProjectionMatrix[i] = ProjectionMatrix[i] * ViewMatrix[i];
}

void camera_tilt(int index, int axes, int direction) {
	glm::vec3 vector;
	glm::vec3 new_prp;
	glm::mat4 mat4_tmp;
	glm::vec3 vec3_tmp;
	switch (axes) {
	case 1: // u
		vector = glm::vec3(ViewMatrix[index][0].x, ViewMatrix[index][1].x, ViewMatrix[index][2].x);
		break;
	case 2: // v
		vector = glm::vec3(ViewMatrix[index][0].y, ViewMatrix[index][1].y, ViewMatrix[index][2].y);
		break;
	case 3: // n
		vector = glm::vec3(ViewMatrix[index][0].z, ViewMatrix[index][1].z, ViewMatrix[index][2].z);
		break;
	default:
		return;
	}

	mat4_tmp = glm::translate(glm::mat4(1.0f), camera[index].prp);
	mat4_tmp = glm::rotate(mat4_tmp, CAM_ROT_SENSITIVITY * direction * TO_RADIAN, vector);
	mat4_tmp = glm::translate(mat4_tmp, -camera[index].prp);

	new_prp = glm::vec3(mat4_tmp * glm::vec4(camera[index].prp, 1.0f));
	camera[index].vrp = glm::vec3(mat4_tmp * glm::vec4(camera[index].vrp, 1.0f));
	camera[index].vup = glm::vec3(mat4_tmp * glm::vec4(camera[index].vup, 0.0f));

	ViewMatrix[index] = glm::lookAt(camera[index].prp, camera[index].vrp, camera[index].vup);
	camera[index].vup = glm::vec3(ViewMatrix[index][0].y, ViewMatrix[index][1].y, ViewMatrix[index][2].y);

	ProjectionMatrix[index] = glm::perspective(camera[index].zoom_factor * camera[index].fov_y * TO_RADIAN, camera[index].aspect_ratio,
		camera[index].near_clip, camera[index].far_clip);
	ViewProjectionMatrix[index] = ProjectionMatrix[index] * ViewMatrix[index];
}

void reshape(int width, int height) {
	viewport.x = viewport.y = 0;
	viewport.w = width;
	viewport.h = height;
	for (int i = 0; i < NUMBER_OF_CAMERAS; i++) {
		camera[i].aspect_ratio = (float)width / height;
		ProjectionMatrix[i] = glm::perspective(camera[i].zoom_factor * camera[i].fov_y * TO_RADIAN, camera[i].aspect_ratio,
			camera[i].near_clip, camera[i].far_clip);
		ViewProjectionMatrix[i] = ProjectionMatrix[i] * ViewMatrix[i];
	}
	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &points_VAO);
	glDeleteBuffers(1, &points_VBO);

	glDeleteVertexArrays(1, &axes_VAO);
	glDeleteBuffers(1, &axes_VBO);

	glDeleteVertexArrays(N_OBJECTS, object_VAO);
	glDeleteBuffers(N_OBJECTS, object_VBO);
}
// End of callback function definitions

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutSpecialFunc(special);
	glutMouseFunc(mousepress);
	glutMotionFunc(motion_1);
	glutReshapeFunc(reshape);
	glutTimerFunc(10, timer_scene, 0);
	glutCloseFunc(cleanup);
}

#define PRINT_DEBUG_INFO  
void initialize_OpenGL(void) {

	create_camera(0, glm::vec3(20.0f, 20.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0, -1.0, 9.0f), 30.0, 1.0f);
	create_camera(1, glm::vec3(0.0f, -20.f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 30.0, 2.0f);
	create_camera(2, glm::vec3(-20.f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), 30.0, 2.0f);
	create_camera(3, glm::vec3(0.0f, 0.0f, 20.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), 30.0, 2.0f);
	create_camera(4, glm::vec3(2.0f, 10.0, 5.0f), glm::vec3(-2.5f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 1.0f), 45.0f, 1.0f);

	cur_frame_wolf = 0;
	cur_frame_spider = 0;
	cur_frame_ben = 0;
	cur_frame_tiger = 0;

	glClearColor(10 / 255.0f, 10 / 255.0f, 10 / 255.0f, 1.0f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_DEPTH_TEST);
}

void create_camera(int index, glm::vec3 prp, glm::vec3 vrp, glm:: vec3 vup, float fov, float zoom) {
	camera[index].prp = prp;
	camera[index].vrp = vrp;
	camera[index].vup = vup;

	ViewMatrix[index] = glm::lookAt(camera[index].prp, camera[index].vrp, camera[index].vup);
	camera[index].vup = glm::vec3(ViewMatrix[index][0].y, ViewMatrix[index][1].y, ViewMatrix[index][2].y);

	camera[index].fov_y = fov;
	camera[index].zoom_factor = zoom;

	camera[index].aspect_ratio = 1.0f; // will be set when the viewing window popped up.
	camera[index].near_clip = 0.1f;
	camera[index].far_clip = 50.0f;
}

void prepare_scene(void) {
	prepare_points();
	prepare_axes();
	prepare_square();
	prepare_dragon();
	prepare_godzila();
	prepare_cow();
	prepare_ironman();
	prepare_bike();

	prepare_wolf();
	prepare_ben();
	prepare_spider();
	prepare_tiger();
}

void initialize_renderer(void) {
	register_callbacks();
	prepare_shader_program();
	initialize_OpenGL();
	prepare_scene();
}

void initialize_glew(void) {
	GLenum error;

	glewExperimental = GL_TRUE;

	error = glewInit();
	if (error != GLEW_OK) {
		fprintf(stderr, "Error: %s\n", glewGetErrorString(error));
		exit(-1);
	}
	fprintf(stdout, "*********************************************************\n");
	fprintf(stdout, " - GLEW version supported: %s\n", glewGetString(GLEW_VERSION));
	fprintf(stdout, " - OpenGL renderer: %s\n", glGetString(GL_RENDERER));
	fprintf(stdout, " - OpenGL version supported: %s\n", glGetString(GL_VERSION));
	fprintf(stdout, "*********************************************************\n\n");
}

void print_message(const char * m) {
	fprintf(stdout, "%s\n\n", m);
}

void greetings(char *program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);
	fprintf(stdout, "    This program was coded for CSE4170 students\n");
	fprintf(stdout, "      of Dept. of Comp. Sci. & Eng., Sogang University.\n\n");

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 3
void main(int argc, char *argv[]) {
	char program_name[64] = "Sogang CSE4170 (4.5.3) Simple Camera Transformation - ROTATION";
	char messages[N_MESSAGE_LINES][256] = { "    - Keys used: 'w', 's', 't', 'c', 'o', 'r', '1', '2', '3', '4', 'm', 'ESC'",
		"    - Special key used: KEY-UP/KEY-DOWN, KEY-LEFT/KEY-RIGHT",
		"    - Mouse used: Left Butten Click and Move"
	};

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(1200, 800);
	glutInitContextVersion(4, 0);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}
