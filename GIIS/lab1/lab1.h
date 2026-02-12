#pragma once
#include <vector>
#include <tuple>
#include <cmath>

int sign(int a);
std::vector<std::pair<int, int>> CDA(int x1, int y1, int x2, int y2);
std::vector<std::pair<int, int>> Bresenham(int x1, int y1, int x2, int y2);
std::vector<std::tuple<int, int, double>> Wu(int x1, int y1, int x2, int y2);