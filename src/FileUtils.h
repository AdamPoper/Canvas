#pragma once
#include <string>
#include <Windows.h>
#include <commdlg.h>

#include <AP2DGL/Window.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

std::string OpenFile(const char* filter, ap::Window* win);
std::string SaveFile(const char* filter, ap::Window* win);
