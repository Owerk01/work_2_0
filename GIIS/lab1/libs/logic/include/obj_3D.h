#pragma once

#include <cmath>
#include <tuple>
#include <vector>

#define ROTATE_ANGLE 5.0f

typedef std::vector<std::tuple<int, int, int>> tuple_vector;

tuple_vector rotate_3D(tuple_vector pts, char axis = 'x');