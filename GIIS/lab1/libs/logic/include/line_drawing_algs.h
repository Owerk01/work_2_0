#pragma once
#include <cmath>
#include <tuple>
#include <vector>

int sign(int a);
std::vector<std::tuple<int, int, double>> draw_CDA(int x1, int y1, int x2,
                                                   int y2);
std::vector<std::tuple<int, int, double>> draw_bresenham(int x1, int y1, int x2,
                                                         int y2);
std::vector<std::tuple<int, int, double>> draw_wu(int x1, int y1, int x2,
                                                  int y2);