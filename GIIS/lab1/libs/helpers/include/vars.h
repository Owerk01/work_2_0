#pragma once

#include <cstdint>
#include <vector>

#define CODE_ERROR -1
#define CELL 16
#define INIT_WIDTH 48 * CELL
#define INIT_HEIGHT 48 * CELL
#define OFFSET int(CELL / 2)
#define CANVAS_SIZE 128 * CELL

#define MIN_WINDOW_WIDTH int((OFFSET + CANVAS_SIZE) / 4)
#define MIN_WINDOW_HEIGHT MIN_WINDOW_WIDTH

#define MAX_WINDOW_WIDTH                                                       \
  CANVAS_SIZE <= MIN_WINDOW_WIDTH ? MIN_WINDOW_WIDTH : CANVAS_SIZE

#define MAX_WINDOW_HEIGHT                                                      \
  CANVAS_SIZE <= MIN_WINDOW_HEIGHT ? MIN_WINDOW_HEIGHT : CANVAS_SIZE

enum class GType {
  CDA = 0,
  Bresenham = 1,
  Wu = 2,
  Circle = 3,
  Elipsis = 4,
  Parabola = 5,
  Hyperbola = 6,
  Hermit = 7,
  Bezier = 8,
  BSpline = 9,
  Cube = 10,
  Tetrahedron = 11
};

struct Point {
  int x = 0;
  int y = 0;
  int z = 0;
  double presize_x = 0;
  double presize_y = 0;
  double presize_z = 0;
  uint8_t r = 0;
  uint8_t g = 0;
  uint8_t b = 0;
  Point() {}
  Point(int x, int y, uint8_t c) : x(x), y(y), r(c), g(c), b(c) {}
  Point(int x, int y, int z, uint8_t c) : x(x), y(y), z(z), r(c), g(c), b(c) {}
  Point(int x, int y, uint8_t r, uint8_t g, uint8_t b)
      : x(x), y(y), z(0), r(r), g(g), b(b) {}
  Point(int x, int y, int z, uint8_t r, uint8_t g, uint8_t b)
      : x(x), y(y), z(z), r(r), g(g), b(b) {}
};

struct Figure {
  int id;
  std::vector<Point> points;
  GType fig_type;
  Figure() {}
  Figure(GType type) : fig_type(type) {}
};