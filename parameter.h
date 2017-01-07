#pragma once
#include <iostream>
#include <string>
#include <string.h>
using namespace std;

#ifndef PARAMETER_H
#define PARAMETER_H

class parameter
{
public:
	parameter();
	/*
	-m <model file name>
	-v <vertex shader file name>
	-f <fragment shader file name>
	-n <number of the all centers, 1<=n<=10 >
	*/
	parameter(int argc, char *argv[]);

	const int get_center_number();
	const int set_center_number(const int n);
	const float* get_center_position(const int index);
	const float* set_center_position(const int index, const double value[]);
	const float* set_center_position(const int index, const float value[]);
	const float get_radius(const int index);
	const float set_radius(const int index, const double pos[]);
	const float set_radius(const int index, float value);
	const float* get_center_normal(const int index);
	const float* set_center_normal(const int index, const float value[]);
	const string model_name(const string str = "");
	const string vertex_shader_name(const string str = "");
	const string fragment_shader_name(const string str = "");
private:
	int center_num;
	float center_pos[10][3];
	float radius[10];
	float center_normal[10][3];
	string model;
	string vertex_shader;
	string fragment_shader;
};

#endif // !PARAMETER_H
