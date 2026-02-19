#pragma once
#include <tuple>
#include <vector>

std::vector<std::tuple<int, int, double>> draw_circle(int R, int c_x, int c_y);
std::vector<std::tuple<int, int, double>> draw_elipsis(int a, int b, int c_x,
                                                       int c_y);
std::vector<std::tuple<int, int, double>> drawParabola(int p, int y_max);
std::vector<std::tuple<int, int, double>> drawHyperbola(int a, int b,
                                                        int x_max);