#pragma once
// OpenGL
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
// SDL
#include <SDL.h>
#undef main
#include "SDL_mixer.h"
#include <fftw3.h>
// C++
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
// own headers
#include "Constants.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "VertexBufferLayout.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Camera.h"
#include "AudioSamplier.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


class App
{
private:
	GLFWwindow* window;

	VertexArray* va;
	VertexBuffer* vb;
	IndexBuffer* ib;
	
	Shader* shader;

	MyAudio* audio;

	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 projection;

	glm::vec3 cubePositions[BARS * BARS_LENGTH];

	double prevTimeS;
	double crntTimeS;
	double dt;
	unsigned int counterS;

	bool drawZDiagram;
public:
	App();
	~App();

	void processInput(GLFWwindow* window);

	void updateDt();
	void update();
	void renderGraph();
	void renderSin();
	void render();

	void run();

private:
	void initVariables();
	void initWindow();
	void initBuffers();
	void initMatrices();
	void initShaders();
	void initAudio();
};


