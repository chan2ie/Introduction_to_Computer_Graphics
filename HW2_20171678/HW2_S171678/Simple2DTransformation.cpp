#include <stdio.h>
#include <stdlib.h>
#include <GL/glew.h>
#include <GL/freeglut.h>
#include <limits.h>
#include <time.h>

#include "Shaders/LoadShaders.h"
#include "Objects.h"

#include <glm/gtc/matrix_transform.hpp> //translate, rotate, scale, ortho, etc.
glm::mat4 ModelViewProjectionMatrix;
glm::mat4 ViewMatrix, ProjectionMatrix, ViewProjectionMatrix;

typedef struct _parabola {
	int speed;
	int time_counter;
	float angle;
	int negative;
} parabola;

typedef struct _color {
	int r;
	int g;
	int b;
} color;

#define NUM_PARABOLA 30
#define NUM_CARS 10
#define GRAVITY 0.2

int win_width = 0, win_height = 0;
float centerx = 0.0f, centery = 0.0f, rotate_angle = 0.0f;
int clock_count = 0;
float plane_radius = 100.0f;
int plane_y = 0;
float house_x = 0;
float house_y = 0;
int delta_x = 1;
int delta_y = 1;
parabola par_arr[NUM_PARABOLA];
color car_arr[NUM_CARS];

GLfloat axes[4][2];
GLfloat axes_color[3] = { 0.0f, 0.0f, 0.0f };
GLuint VBO_axes, VAO_axes;

void draw_rcs(int);

void prepare_axes(void) { // Draw axes in their MC.
	axes[0][0] = -win_width / 2.5f; axes[0][1] = 0.0f;
	axes[1][0] = win_width / 2.5f; axes[1][1] = 0.0f;
	axes[2][0] = 0.0f; axes[2][1] = -win_height / 2.5f;
	axes[3][0] = 0.0f; axes[3][1] = win_height / 2.5f;

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_axes);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_axes);
	glBindVertexArray(VAO_axes);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void update_axes(void) {
	axes[0][0] = -win_width / 2.25f; axes[1][0] = win_width / 2.25f;
	axes[2][1] = -win_height / 2.25f;
	axes[3][1] = win_height / 2.25f;

	glBindBuffer(GL_ARRAY_BUFFER, VBO_axes);
	glBufferData(GL_ARRAY_BUFFER, sizeof(axes), axes, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw_axes(void) {
	glUniform3fv(loc_primitive_color, 1, axes_color);
	glBindVertexArray(VAO_axes);
	glDrawArrays(GL_LINES, 0, 4);
	glBindVertexArray(0);
}

GLfloat line[2][2];
GLfloat line_color[3] = { 1.0f, 0.0f, 0.0f };
GLuint VBO_line, VAO_line;

void prepare_line(void) { 	// y = x - win_height/4
	line[0][0] = (1.0f / 4.0f - 1.0f / 2.5f) * win_height;
	line[0][1] = (1.0f / 4.0f - 1.0f / 2.5f) * win_height - win_height / 4.0f;
	line[1][0] = win_width / 2.5f;
	line[1][1] = win_width / 2.5f - win_height / 4.0f;

	// Initialize vertex buffer object.
	glGenBuffers(1, &VBO_line);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_line);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);

	// Initialize vertex array object.
	glGenVertexArrays(1, &VAO_line);
	glBindVertexArray(VAO_line);

	glBindBuffer(GL_ARRAY_BUFFER, VBO_line);
	glVertexAttribPointer(LOC_VERTEX, 2, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void update_line(void) { 	// y = x - win_height/4
	line[0][0] = (1.0f / 4.0f - 1.0f / 2.5f) * win_height;
	line[0][1] = (1.0f / 4.0f - 1.0f / 2.5f) * win_height - win_height / 4.0f;
	line[1][0] = win_width / 2.5f;
	line[1][1] = win_width / 2.5f - win_height / 4.0f;

	glBindBuffer(GL_ARRAY_BUFFER, VBO_line);
	glBufferData(GL_ARRAY_BUFFER, sizeof(line), line, GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void draw_line(void) { // Draw line in its MC.
					   // y = x - win_height/4
	glUniform3fv(loc_primitive_color, 1, line_color);
	glBindVertexArray(VAO_line);
	glDrawArrays(GL_LINES, 0, 2);
	glBindVertexArray(0);
}

void display(void) {
	glm::mat4 ModelMatrix;

	glClear(GL_COLOR_BUFFER_BIT);

	house_x += delta_x * 2;
	house_y += delta_y * 3;
	if (house_x * delta_x > win_width / 2) {
		delta_x *= -1;
	}
	if (house_y * delta_y > win_height / 2) {
		delta_y *= -1;
	}

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(house_x * 1.0f, house_y * 1.0f, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, clock_count * 3 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(3*sin(clock_count * TO_RADIAN), 3*cos(clock_count * TO_RADIAN), 1.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_house();
	
	float cocktail_x, cocktail_y, cocktail_angle;
	for (int i = 0; i < NUM_PARABOLA; i++) {
		cocktail_x = par_arr[i].negative * par_arr[i].speed * cos(par_arr[i].angle) * par_arr[i].time_counter;
		cocktail_y = 200 + par_arr[i].speed * sin(par_arr[i].angle) * par_arr[i].time_counter - 0.5 * GRAVITY * pow(par_arr[i].time_counter, 2);
		cocktail_angle = par_arr[i].speed * par_arr[i].time_counter * TO_RADIAN;

		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(cocktail_x * 1.0f, cocktail_y * 1.0f, 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, cocktail_angle, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_cocktail();

		if (cocktail_y < -win_height / 2) {
			par_arr[i].speed = rand() % 10 + 3;
			par_arr[i].time_counter = 0;
			par_arr[i].angle = (20 + rand() % 70) * TO_RADIAN;
			par_arr[i].negative = 1 - rand() % 3;
		}
		else {
			par_arr[i].time_counter += 1;
		}
	}

	float shirt_angle;
	float shirt_x, shirt_y;
	float shirt_scale = 0.3f * cos(clock_count * 10 * TO_RADIAN) + 1.2f;
	for (int j = 0; j < 20; j++) {
		shirt_angle = (clock_count + 18 * j) % 360 * TO_RADIAN;
		shirt_x = shirt_scale * 8 * 16 * pow(sin(shirt_angle) , 3);
		shirt_y = shirt_scale *8 * (13 * cos(shirt_angle) - 5 * cos(2 * shirt_angle) - 2 * cos(3 * shirt_angle) - cos(4 * shirt_angle));
		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(shirt_x * 1.0f, shirt_y * 1.0f, 0.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(shirt_scale, shirt_scale, 1.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_shirt();
	}

	for (int j = 1; j < 5; j++) {
		int num_hat = 9 * j;
		float hat_angle;
		float hat_radius = 140 * j + 45 * j * sin(clock_count * 1.7 * TO_RADIAN);
		for (int i = 0; i < num_hat; i++) {
			hat_angle = pow(-1, j) * (clock_count + 360 / num_hat * i) * TO_RADIAN;

			ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(hat_radius * cos(hat_angle), hat_radius * sin(hat_angle), 0.0f));
			ModelMatrix = glm::rotate(ModelMatrix, hat_angle - 90 * TO_RADIAN, glm::vec3(0.0f, 0.0f, 1.0f));
			ModelMatrix = glm::scale(ModelMatrix, glm::vec3((5 - j) / 3.0, (5 - j) / 3.0, 0.0f));
			ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
			glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
			draw_hat();
		}
	}

	float plane_x, plane_angle;
	
	// plane_radius += (rand() % 21 - 10) / 20.0f;
	for (int i = 0; i < 15; i++) {
		plane_x = (clock_count*2 + 45 * i) % (win_width + 360) - (win_width + 360) / 2;
		plane_angle = (90 - (clock_count*2+ 45 * i)) * TO_RADIAN;
	
		ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(plane_x + plane_radius * cos(plane_angle), 
			plane_y + plane_radius * sin(plane_angle), 0.0f));
		ModelMatrix = glm::rotate(ModelMatrix, plane_angle, glm::vec3(0.0f, 0.0f, 1.0f));
		ModelMatrix = glm::scale(ModelMatrix, glm::vec3(0.3 + i * 0.15, 0.3 + i * 0.15, 0.0f));
		ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
		glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
		draw_airplane();
	}
	
	for (int i = 0; i < NUM_CARS; i++) {
		draw_rcs(i);
	}

	glFlush();
}

void draw_rcs(int index) {
	int rc_clock = clock_count * 2 + 54 * (NUM_CARS  - index);
	int rc_count = rc_clock % 720;
	float rc_angle, rc_x, rc_y;
	int rc_radius = 300;
	glm::mat4 ModelMatrix;

	if (rc_count < 360) {
		rc_angle = rc_count * TO_RADIAN;
		rc_x = rc_radius * cos(rc_angle) - rc_radius;
		rc_y = 0.5 * rc_radius * sin(rc_angle);
	}
	else {
		rc_angle = (180 - rc_count) * TO_RADIAN;
		rc_x = rc_radius * cos(rc_angle) + rc_radius;
		rc_y = 0.5 * rc_radius * sin(rc_angle);
	}

	ModelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(rc_x, rc_y, 0.0f));
	ModelMatrix = glm::rotate(ModelMatrix, rc_count / 2 * TO_RADIAN, glm::vec3(0.0f, 1.0f, 0.0f));
	ModelViewProjectionMatrix = ViewProjectionMatrix * ModelMatrix;
	glUniformMatrix4fv(loc_ModelViewProjectionMatrix, 1, GL_FALSE, &ModelViewProjectionMatrix[0][0]);
	draw_rc(car_arr[index].r, car_arr[index].g, car_arr[index].b);
}

void timer(int value) {
	clock_count = (clock_count + 1) % INT_MAX;
	glutPostRedisplay();
	glutTimerFunc(10, timer, 0);
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) {
	case 27: // ESC key
		glutLeaveMainLoop(); // Incur destuction callback for cleanups.
		break;
	}
}

void reshape(int width, int height) {
	win_width = width, win_height = height;

	glViewport(0, 0, win_width, win_height);
	ProjectionMatrix = glm::ortho(-win_width / 2.0, win_width / 2.0,
		-win_height / 2.0, win_height / 2.0, -1000.0, 1000.0);
	ViewProjectionMatrix = ProjectionMatrix * ViewMatrix;

	update_axes();
	update_line();

	glutPostRedisplay();
}

void cleanup(void) {
	glDeleteVertexArrays(1, &VAO_axes);
	glDeleteBuffers(1, &VBO_axes);

	glDeleteVertexArrays(1, &VAO_line);
	glDeleteBuffers(1, &VBO_line);

	glDeleteVertexArrays(1, &VAO_airplane);
	glDeleteBuffers(1, &VBO_airplane);

	// Delete others here too!!!
}

void register_callbacks(void) {
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutReshapeFunc(reshape);
	glutCloseFunc(cleanup);
	glutTimerFunc(10, timer, 0);
}

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

void initialize_OpenGL(void) {
	glEnable(GL_MULTISAMPLE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

	glClearColor(250 / 255.0f, 128 / 255.0f, 114 / 255.0f, 1.0f);
	ViewMatrix = glm::mat4(1.0f);
}

void prepare_scene(void) {
	prepare_axes();
	prepare_line();
	prepare_airplane();
	prepare_shirt();
	prepare_house();
	prepare_car();
	prepare_cocktail();
	prepare_car2();
	prepare_hat();
	prepare_cake();
	prepare_sword();
	prepare_rc();
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
}

void greetings(char* program_name, char messages[][256], int n_message_lines) {
	fprintf(stdout, "**************************************************************\n\n");
	fprintf(stdout, "  PROGRAM NAME: %s\n\n", program_name);

	for (int i = 0; i < n_message_lines; i++)
		fprintf(stdout, "%s\n", messages[i]);
	fprintf(stdout, "\n**************************************************************\n\n");

	initialize_glew();
}

#define N_MESSAGE_LINES 1
void main(int argc, char* argv[]) {
	srand(time(NULL));

	char program_name[64] = "Sogang CSE4170 HW1";
	char messages[N_MESSAGE_LINES][256] = {
		"    - Keys used: 'ESC' "
	};

	for (int i = 0; i < NUM_PARABOLA; i++) {
		par_arr[i].speed = rand() % 10 + 3;
		par_arr[i].time_counter = rand() % 60;
		par_arr[i].angle = (20 + rand() % 70) * TO_RADIAN;
		par_arr[i].negative = 1 - rand() % 3;
	}

	car_arr[0].r = 255;
	car_arr[0].g = 80;
	car_arr[0].b = 80;

	for (int i = 1; i < NUM_CARS; i++) {
		car_arr[i].r = rand() % 150 + 55;
		car_arr[i].g = rand() % 150 + 55;
		car_arr[i].b = rand() % 150 + 55;
	}

	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_MULTISAMPLE);
	glutInitWindowSize(1440, 900);
	glutInitContextVersion(3, 3);
	glutInitContextProfile(GLUT_CORE_PROFILE);
	glutCreateWindow(program_name);

	greetings(program_name, messages, N_MESSAGE_LINES);
	initialize_renderer();

	glutSetOption(GLUT_ACTION_ON_WINDOW_CLOSE, GLUT_ACTION_GLUTMAINLOOP_RETURNS);
	glutMainLoop();
}


