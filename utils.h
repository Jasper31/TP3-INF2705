#ifndef UTILS_H
#define UTILS_H

#include <string>

#define GL_CHECK_ERROR checkGLError(__LINE__)
void checkGLError(int line);

std::string readFile(const char* path);

double rand01();

void getGroupRandomPos(int groupId, const int N_ROWS, float& x, float& z);

#endif // UTILS_H