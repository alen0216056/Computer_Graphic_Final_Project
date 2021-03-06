#include <stdio.h>
#include <stdlib.h>
#include <stddef.h> /* offsetof */
#include <string>
#include <math.h>
#include <time.h>
#include "../GL/glew.h"
#include "../GL/glut.h""
#include "../shader_lib/shader.h"
#include "glm/glm.h"
#include "parameter.h"
extern "C"
{
	#include "glm_helper.h"
}

void init(void);
void display(void);
void reshape(int width, int height);
void keyboard(unsigned char key, int x, int y);
void mouse(int button, int state, int x, int y);
void idle(void);
bool gluInvertMatrix(const float m[16], float invOut[16]);
void sleepcp(int milliseconds);

/*global variables*/
struct my_vertex
{
	float position[3];
	float normal[3];
	float face_normal[3];
	float texture[2];
};

GLMmodel *model;
GLfloat light_pos[] = { 10.0, 10.0, 0.0 };
float eye_pos[] = { 0.0, 0.0, 3.0 };
bool timer_mode[10], show_center = true;
int timer[10], center_index = 0;
parameter* parameter_ptr;
my_vertex* vertices;
GLuint vertex_shader, fragment_shader, program, vbo_name;

int main(int argc, char *argv[])
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow("Final Project");
	glutReshapeWindow(800, 800);

	glewInit();

	parameter_ptr = new parameter(argc, argv);
	init();

	glutReshapeFunc(reshape);
	glutDisplayFunc(display);
	glutIdleFunc(idle);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);

	glutMainLoop();

	free(vertices);
	glmDelete(model);
	return 0;
}

void init(void) {
	//Initialize model
	model = glmReadOBJ(parameter_ptr->model_name().c_str());
	glmUnitize(model);
	glmFacetNormals(model);
	glmVertexNormals(model, 90.0, GL_FALSE);
	print_model_info(model);
	//Compute the number of all triangles
	int triangle_num = 0;
	for (GLMgroup* group = model->groups; group; group = group->next)
	{
		triangle_num += group->numtriangles;
	}
	//Initialize vertex structure array
	vertices = (my_vertex*)malloc(triangle_num * 3 * sizeof(my_vertex));
	triangle_num = 0;
	for (GLMgroup* group = model->groups; group; group = group->next)
	{
		for (int i = 0; i < group->numtriangles; i++)
		{
			GLMtriangle* triangle_tmp = &model->triangles[group->triangles[i]];
			for (int j = 0; j < 3; j++)
			{
				for (int k = 0; k < 3; k++)
				{
					vertices[3 * (i + triangle_num) + j].position[k] = model->vertices[3 * triangle_tmp->vindices[j] + k];
					vertices[3 * (i + triangle_num) + j].normal[k] = model->normals[3 * triangle_tmp->nindices[j] + k];
					vertices[3 * (i + triangle_num) + j].face_normal[k] = model->facetnorms[3 * triangle_tmp->findex + k];
				}
				for (int k = 0; k < 2; k++)
				{
					vertices[3 * (i + triangle_num) + j].texture[k] = model->texcoords[2 * triangle_tmp->tindices[j] + k];
				}
			}
		}
		triangle_num += group->numtriangles;
	}
	//Generate vertex buffer object and pass data into GPU memory 
	glGenBuffers(1, &vbo_name);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_name);
	glBufferData(GL_ARRAY_BUFFER, triangle_num * 3 * sizeof(my_vertex), vertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);	//0 is GLindex
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(my_vertex), (void*)(offsetof(my_vertex, position)));	//first 0 is GLindex
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(my_vertex), (void*)(offsetof(my_vertex, normal)));
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(my_vertex), (void*)(offsetof(my_vertex, face_normal)));
	glEnableVertexAttribArray(3);
	glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(my_vertex), (void*)(offsetof(my_vertex, texture)));
	//compile vertex shader, fragment shader, and create program
	vertex_shader = createShader(parameter_ptr->vertex_shader_name().c_str(), "vertex");
	fragment_shader = createShader(parameter_ptr->fragment_shader_name().c_str(), "fragment");
	program = createProgram(vertex_shader, fragment_shader);
	//initial timer
	for (int i = 0; i < 10; i++)
	{
		timer[i] = 1;
		timer_mode[i] = true;
	}
		
}

void display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0, 1.0, 1e-2, 1e2);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eye_pos[0], eye_pos[1], eye_pos[2],
				0.0, 0.0, 0.0,
				0.0, 1.0, 0.0);

	//	Write your code here
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	
	GLfloat projection_matrix[16];
	glGetFloatv(GL_PROJECTION_MATRIX, projection_matrix);
	GLfloat modelview_matrix[16];
	glGetFloatv(GL_MODELVIEW_MATRIX, modelview_matrix);
	GLfloat inverse_modelview_matrix[16];
	gluInvertMatrix(modelview_matrix, inverse_modelview_matrix);

	GLint texture_location = glGetUniformLocation(program, "texture");
	GLint ambient_location = glGetUniformLocation(program, "ambient");
	GLint diffuse_location = glGetUniformLocation(program, "diffuse");
	GLint specular_location = glGetUniformLocation(program, "specular");
	GLint shininess_location = glGetUniformLocation(program, "shininess");
	GLint use_texture_location = glGetUniformLocation(program, "use_texture");
	glUseProgram(program);
		glUniformMatrix4fv(glGetUniformLocation(program, "Projection"), 1, GL_FALSE, projection_matrix);
		glUniformMatrix4fv(glGetUniformLocation(program, "Modelview"), 1, GL_FALSE, modelview_matrix);
		glUniformMatrix4fv(glGetUniformLocation(program, "Inverse_Modelview"), 1, GL_FALSE, inverse_modelview_matrix);
		glUniform3f(glGetUniformLocation(program, "eye_pos"), eye_pos[0], eye_pos[1], 3.0);
		glUniform3fv(glGetUniformLocation(program, "light_pos"), 1, light_pos);
		glUniform1iv(glGetUniformLocation(program, "timer"), 10, timer);
		glUniform1i(glGetUniformLocation(program, "center_num"), parameter_ptr->get_center_number()); 
		for (int i = 0; i < 10; i++)
		{
			string str = "center[" + to_string(i) + "]";
			glUniform3fv(glGetUniformLocation(program, str.c_str()), 1, parameter_ptr->get_center_position(i));
			str = "center_normal[" + to_string(i) + "]";
			glUniform3fv(glGetUniformLocation(program, str.c_str()), 1, parameter_ptr->get_center_normal(i));
			str = "radius[" + to_string(i) + "]";
			glUniform1f(glGetUniformLocation(program, str.c_str()), parameter_ptr->get_radius(i));
		}
		
		int triangle_num = 0;
		for (GLMgroup* group = model->groups; group; group = group->next)
		{
			glUniform4fv(ambient_location, 1, model->materials[group->material].ambient);
			glUniform4fv(diffuse_location, 1, model->materials[group->material].diffuse);
			glUniform4fv(specular_location, 1, model->materials[group->material].specular);
			glUniform1f(shininess_location, model->materials[group->material].shininess);

			if (model->materials[group->material].map_diffuse != GLuint(-1))
			{
				glActiveTexture(GL_TEXTURE0 + 0);
				glBindTexture(GL_TEXTURE_2D, model->textures[model->materials[group->material].map_diffuse].id);
				glUniform1i(texture_location, 0);
				glUniform1i(use_texture_location, 1);
			}
			else
			{
				glUniform1i(use_texture_location, 0);
			}
			glDrawArrays(GL_TRIANGLES, triangle_num * 3, group->numtriangles * 3);

			glBindTexture(GL_TEXTURE_2D, NULL);
			triangle_num += group->numtriangles;
		}
	glUseProgram(0);
	
	if (show_center)
	{
		glColor3f(1, 1, 1);
		glPointSize(10);
		glBegin(GL_POINTS);
		for (int i = 0; i<parameter_ptr->get_center_number(); i++)
			glVertex3fv(parameter_ptr->get_center_position(i));
		glEnd();
	}
	
	for (int i = 0; i < 10; i++)
	{
		if (timer_mode[i])
		{
			timer[i]++;
			if (timer[i] == 120)
				timer_mode[i] = false;
		}
		else
		{
			timer[i]--;
			if (timer[i] == 1)
				timer_mode[i] = true;
		}
	}
	
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_DEPTH_TEST);
	glutSwapBuffers();

	sleepcp(10);
}

void reshape(int width, int height)
{
	glViewport(0, 0, width, height);
}

void keyboard(unsigned char key, int x, int y) {
	switch (key) 
	{
	case 27://ESC
		exit(0);
		break;
	case 'd':
		eye_pos[0] += 0.1;
		break;
	case 'a':
		eye_pos[0] -= 0.1;
		break;
	case 'w':
		eye_pos[1] += 0.1;
		break;
	case 's':
		eye_pos[1]-= 0.1;
		break;
	case 9:	//tab
		show_center = !show_center;
		break;
	default:
		if ('0' <= key && key <= '9')
		{
			if (key - '0' < parameter_ptr->get_center_number())
			{
				center_index = key - '0';
				printf("center index %d\n", center_index);
			}
		}
		break;
	}
}

void mouse(int button, int state, int x, int y)
{
	switch (button)
	{
	case GLUT_LEFT_BUTTON:
		if (state == GLUT_DOWN)
		{
			GLdouble modelview[16];
			glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
			GLdouble projection[16];
			glGetDoublev(GL_PROJECTION_MATRIX, projection);
			GLint viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);
			GLfloat z;
			y = viewport[3] - y - 1;
			glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
			//printf("x: %d y: %d z: %f\n", x, y, z);
			double center[3];
			gluUnProject(x, y, z, modelview, projection, viewport, &center[0], &center[1], &center[2]);
			parameter_ptr->set_center_position(center_index, center);
			//printf("%lf %lf %lf\n", parameter_ptr->get_center_position(0)[0], parameter_ptr->get_center_position(0)[1], parameter_ptr->get_center_position(0)[2]);

			double distance, min_distance = 0xffffffff;
			for (int i = 0; i < model->numvertices; i++)
			{
				distance = pow((model->vertices[i * 3] - center[0]) * (model->vertices[i * 3] - center[0])
					+ (model->vertices[i * 3 + 1] - center[1]) * (model->vertices[i * 3 + 1] - center[1])
					+ (model->vertices[i * 3 + 2] - center[2]) * (model->vertices[i * 3 + 2] - center[2]), 0.5);
				if (distance < min_distance)
				{
					parameter_ptr->set_center_normal(center_index, &model->normals[i * 3]);
					min_distance = distance;
				}
			}
			timer[center_index] = 1;
			timer_mode[center_index] = true;
			parameter_ptr->set_radius(center_index, 0.0f);
			printf("center normal:%f %f %f\n", parameter_ptr->get_center_normal(center_index)[0], parameter_ptr->get_center_normal(center_index)[1], parameter_ptr->get_center_normal(center_index)[2]);
		}
		break;
	case GLUT_RIGHT_BUTTON:
		if (state == GLUT_DOWN)
		{
			GLdouble modelview[16];
			glGetDoublev(GL_MODELVIEW_MATRIX, modelview);
			GLdouble projection[16];
			glGetDoublev(GL_PROJECTION_MATRIX, projection);
			GLint viewport[4];
			glGetIntegerv(GL_VIEWPORT, viewport);
			GLfloat z;
			y = viewport[3] - y - 1;
			glReadPixels(x, y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &z);
			//printf("x: %d y: %d z: %f\n", x, y, z);
			double center[3];
			gluUnProject(x, y, z, modelview, projection, viewport, &center[0], &center[1], &center[2]);
			parameter_ptr->set_radius(center_index, center);
			printf("radius %f\n", parameter_ptr->get_radius(center_index));
		}
		break;
	case GLUT_MIDDLE_BUTTON:
		if (state == GLUT_DOWN)
		{

		}
		break;
	default:

		break;
	}
}

void idle(void)
{
	glutPostRedisplay();
}

bool gluInvertMatrix(const float m[16], float invOut[16])
{
	double inv[16], det;
	int i;

	inv[0] = m[5] * m[10] * m[15] -
		m[5] * m[11] * m[14] -
		m[9] * m[6] * m[15] +
		m[9] * m[7] * m[14] +
		m[13] * m[6] * m[11] -
		m[13] * m[7] * m[10];

	inv[4] = -m[4] * m[10] * m[15] +
		m[4] * m[11] * m[14] +
		m[8] * m[6] * m[15] -
		m[8] * m[7] * m[14] -
		m[12] * m[6] * m[11] +
		m[12] * m[7] * m[10];

	inv[8] = m[4] * m[9] * m[15] -
		m[4] * m[11] * m[13] -
		m[8] * m[5] * m[15] +
		m[8] * m[7] * m[13] +
		m[12] * m[5] * m[11] -
		m[12] * m[7] * m[9];

	inv[12] = -m[4] * m[9] * m[14] +
		m[4] * m[10] * m[13] +
		m[8] * m[5] * m[14] -
		m[8] * m[6] * m[13] -
		m[12] * m[5] * m[10] +
		m[12] * m[6] * m[9];

	inv[1] = -m[1] * m[10] * m[15] +
		m[1] * m[11] * m[14] +
		m[9] * m[2] * m[15] -
		m[9] * m[3] * m[14] -
		m[13] * m[2] * m[11] +
		m[13] * m[3] * m[10];

	inv[5] = m[0] * m[10] * m[15] -
		m[0] * m[11] * m[14] -
		m[8] * m[2] * m[15] +
		m[8] * m[3] * m[14] +
		m[12] * m[2] * m[11] -
		m[12] * m[3] * m[10];

	inv[9] = -m[0] * m[9] * m[15] +
		m[0] * m[11] * m[13] +
		m[8] * m[1] * m[15] -
		m[8] * m[3] * m[13] -
		m[12] * m[1] * m[11] +
		m[12] * m[3] * m[9];

	inv[13] = m[0] * m[9] * m[14] -
		m[0] * m[10] * m[13] -
		m[8] * m[1] * m[14] +
		m[8] * m[2] * m[13] +
		m[12] * m[1] * m[10] -
		m[12] * m[2] * m[9];

	inv[2] = m[1] * m[6] * m[15] -
		m[1] * m[7] * m[14] -
		m[5] * m[2] * m[15] +
		m[5] * m[3] * m[14] +
		m[13] * m[2] * m[7] -
		m[13] * m[3] * m[6];

	inv[6] = -m[0] * m[6] * m[15] +
		m[0] * m[7] * m[14] +
		m[4] * m[2] * m[15] -
		m[4] * m[3] * m[14] -
		m[12] * m[2] * m[7] +
		m[12] * m[3] * m[6];

	inv[10] = m[0] * m[5] * m[15] -
		m[0] * m[7] * m[13] -
		m[4] * m[1] * m[15] +
		m[4] * m[3] * m[13] +
		m[12] * m[1] * m[7] -
		m[12] * m[3] * m[5];

	inv[14] = -m[0] * m[5] * m[14] +
		m[0] * m[6] * m[13] +
		m[4] * m[1] * m[14] -
		m[4] * m[2] * m[13] -
		m[12] * m[1] * m[6] +
		m[12] * m[2] * m[5];

	inv[3] = -m[1] * m[6] * m[11] +
		m[1] * m[7] * m[10] +
		m[5] * m[2] * m[11] -
		m[5] * m[3] * m[10] -
		m[9] * m[2] * m[7] +
		m[9] * m[3] * m[6];

	inv[7] = m[0] * m[6] * m[11] -
		m[0] * m[7] * m[10] -
		m[4] * m[2] * m[11] +
		m[4] * m[3] * m[10] +
		m[8] * m[2] * m[7] -
		m[8] * m[3] * m[6];

	inv[11] = -m[0] * m[5] * m[11] +
		m[0] * m[7] * m[9] +
		m[4] * m[1] * m[11] -
		m[4] * m[3] * m[9] -
		m[8] * m[1] * m[7] +
		m[8] * m[3] * m[5];

	inv[15] = m[0] * m[5] * m[10] -
		m[0] * m[6] * m[9] -
		m[4] * m[1] * m[10] +
		m[4] * m[2] * m[9] +
		m[8] * m[1] * m[6] -
		m[8] * m[2] * m[5];

	det = m[0] * inv[0] + m[1] * inv[4] + m[2] * inv[8] + m[3] * inv[12];

	if (det == 0)
		return false;

	det = 1.0 / det;

	for (i = 0; i < 16; i++)
		invOut[i] = inv[i] * det;

	return true;
}

void sleepcp(int milliseconds)	//sleep 10^-3 second
{
	clock_t time_end;
	time_end = clock() + milliseconds * CLOCKS_PER_SEC / 1000;
	while (clock() < time_end)
	{
	}
}