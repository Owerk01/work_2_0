#include "obj_3D.h"
#include <cmath>
#include <tuple>

tuple_vector rotate_3D(tuple_vector pts, char axis) {
  tuple_vector new_pts;

  float angle = ROTATE_ANGLE * M_PI / 180.0f;
  float cos_a = cos(angle);
  float sin_a = sin(angle);

  for (const auto &pt : pts) {
    int x = std::get<0>(pt);
    int y = std::get<1>(pt);
    int z = std::get<2>(pt);

    int new_x, new_y, new_z;

    switch (axis) {
    case 'x':
      new_x = x;
      new_y = static_cast<int>(round(y * cos_a - z * sin_a));
      new_z = static_cast<int>(round(y * sin_a + z * cos_a));
      break;

    case 'y':
      new_x = static_cast<int>(round(x * cos_a + z * sin_a));
      new_y = y;
      new_z = static_cast<int>(round(-x * sin_a + z * cos_a));
      break;

    case 'z':
      new_x = static_cast<int>(round(x * cos_a - y * sin_a));
      new_y = static_cast<int>(round(x * sin_a + y * cos_a));
      new_z = z;
      break;

    default:
      new_x = x;
      new_y = y;
      new_z = z;
      break;
    }

    new_pts.push_back(std::make_tuple(new_x, new_y, new_z));
  }

  return new_pts;
}