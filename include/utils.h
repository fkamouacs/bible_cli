#ifndef UTILS_H
#define UTILS_H
#include <curses.h>
#include <vector>
#include <string>
#include <iostream>
#include <sstream>

std::vector<std::string> reorderVector(const std::vector<std::string> &matrix, int rows, int cols);

void printMultilineAscii(int start_y, int start_x, const std::string &asciiArt);

#endif