#pragma once
#include <tuple>
#include <utility>
#include <vector>

typedef std::vector<std::pair<int, int>> point_vector;
typedef std::vector<std::tuple<int, int, double>> color_point_vector;

bool is_convex(const point_vector &points);
point_vector graham_scan(const point_vector &points);
color_point_vector draw_convex_polygon(point_vector vertices);
color_point_vector get_convex_hull_points(const point_vector &vertices);
bool is_point_inside_polygon(const point_vector &point,
                             const point_vector &polygon);
color_point_vector get_line_polygon_intersections(const point_vector &line,
                                                  const point_vector &polygon);