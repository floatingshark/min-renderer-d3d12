#include <iostream>
#include <vector>
#include <string>
#include <windows.h>
#include "GLFW/glfw3.h"

int main()
{
	std::cout << "Begin program" << std::endl;

	const uint32_t WIDTH = 800;
	const uint32_t HEIGHT = 600;

	glfwInit();
	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
	glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
	GLFWwindow *window = glfwCreateWindow(WIDTH, HEIGHT, "Arabesque", nullptr, nullptr);

	while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();
	}

	std::cout << "End program" << std::endl;

	return 0;
}