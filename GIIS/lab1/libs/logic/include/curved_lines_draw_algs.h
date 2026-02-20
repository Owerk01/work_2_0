#pragma once
#include <tuple>
#include <vector>

std::vector<std::tuple<int, int, double>> draw_circle(int R, int c_x, int c_y);
std::vector<std::tuple<int, int, double>> draw_elipsis(int a, int b, int c_x,
                                                       int c_y);
std::vector<std::tuple<int, int, double>> draw_parabola(int c_x, int c_y,
                                                        int p);
std::vector<std::tuple<int, int, double>> draw_hyperbola(int c_x, int c_y,
                                                         int a, int b);