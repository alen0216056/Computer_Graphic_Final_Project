#include "parameter.h"

parameter::parameter()
{
	center_num = 1;
	for (int i = 0; i < 10; i++)
	{
		center_pos[i][0] = center_pos[i][1] = center_pos[i][2] = 0.0;
		center_normal[i][0] = center_normal[i][1] = center_normal[i][2] = 0.0;
		radius[i] = 0.0;
	}
	model = "";
	vertex_shader = "";
	fragment_shader = "";
}

parameter::parameter(int argc, char *argv[])
{
	center_num = 1;
	for (int i = 0; i < 10; i++)
	{
		center_pos[i][0] = center_pos[i][1] = center_pos[i][2] = 0.0;
		center_normal[i][0] = center_normal[i][1] = center_normal[i][2] = 0.0;
		radius[i] = 0.0;
	}
	model = "";
	vertex_shader = "";
	fragment_shader = "";
	for (int i = 1; i < argc; i++)
	{
		if (argv[i][0] == '-')
		{
			switch (argv[i][1])
			{
			case 'm':
				model = argv[++i];
				break;
			case 'v':
				vertex_shader = argv[++i];
				break;
			case 'f':
				fragment_shader = argv[++i];
				break;
			case 'n':
				center_num = atoi(argv[++i]);
				center_num = (center_num >= 1 && center_num <= 10) ? center_num : 1;
				break;
			default:
				cout << "usage: ./final_project.exe -m <model file path> -v <vertex shader file path> -f <fragment shader file path> -n <number of the centers>\n";
				exit(EXIT_SUCCESS);
				break;
			}
		}
		else
		{
			cout << "usage: ./final_project.exe -m <model file path> -v <vertex shader file path> -f <fragment shader file path> -n <number of the centers>\n";
			exit(EXIT_SUCCESS);
		}
	}
}

const int parameter::get_center_number()
{
	return center_num;
}

const int parameter::set_center_number(const int n)
{
	center_num = n;
	return center_num;
}

const float * parameter::get_center_position(const int index)
{
	return (0 <= index && index < center_num) ? center_pos[index] : center_pos[0];
}

const float * parameter::set_center_position(const int index, const double value[])
{
	if (0 <= index && index < center_num)
	{
		for (int i = 0; i < 3; i++)
			center_pos[index][i] = (float)value[i];
	}
	return (0 <= index && index < center_num) ? center_pos[index] : center_pos[0];
}

const float * parameter::set_center_position(const int index, const float value[])
{
	if (0 <= index && index < center_num)
	{
		for (int i = 0; i < 3; i++)
			center_pos[index][i] = value[i];
	}
	return (0 <= index && index < center_num) ? center_pos[index] : center_pos[0];
}

const float parameter::get_radius(const int index)
{
	return (0 <= index && index < center_num) ? radius[index] : radius[0];
}

const float parameter::set_radius(const int index, const double pos[])
{
	if (0 <= index && index < center_num)
	{
		radius[index] = sqrt(
			(pos[0] - center_pos[index][0])*(pos[0] - center_pos[index][0])
			+ (pos[1] - center_pos[index][1]) * (pos[1] - center_pos[index][1])
			+ (pos[2] - center_pos[index][2]) * (pos[2] - center_pos[index][2]));
	}
	return 0.0f;
}

const float parameter::set_radius(const int index, const float value)
{
	if (0 <= index && index < center_num)
	{
		radius[index] = value;
	}
	return (0 <= index && index < center_num) ? radius[index] : radius[0];
}

const float * parameter::get_center_normal(const int index)
{
	return (0 <= index && index < center_num) ? center_normal[index] : center_normal[0];
}

const float * parameter::set_center_normal(const int index,const float value[])
{
	if (0 <= index && index < center_num)
	{
		for (int i = 0; i < 3; i++)
			center_normal[index][i] = value[i];
	}
	return (0 <= index && index < center_num) ? center_normal[index] : center_normal[0];
}

const string parameter::model_name(const string str)
{
	if (str != "")
		model = str;
	return model;
}

const string parameter::vertex_shader_name(const string str)
{
	if (str != "")
		vertex_shader = str;
	return vertex_shader;
}

const string parameter::fragment_shader_name(const string str)
{
	if (str != "")
		fragment_shader = str;
	return fragment_shader;
}
