#pragma once
#include <GL/glew.h>
#include <iostream>

#define APP_NAME "Oscillator3D"

#define PATH_TO_SHADER "res/shaders/Basic.shader"

#define MUSIC_PATH "res/loop.wav"
#define SAMPLE_RATE 44100
#define SAMPLES 1024
#define CHANELS 2

#define BARS 64
#define BARS_LENGTH 100
#define MARGIN 2


const unsigned int SCR_WIDTH = 1280;
const unsigned int SCR_HEIGHT = 960;

#define ASSERT(x) if (!(x)) __debugbreak();
#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))

void GLClearError();
bool GLLogCall(const char* function, const char* file, int line);