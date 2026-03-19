#ifndef POLYGON_FILL_H
#define POLYGON_FILL_H

#include <vector>
#include <tuple>

using point_vector = std::vector<std::pair<int, int>>;
using color_point_vector = std::vector<std::tuple<int, int, double>>;

struct ActiveEdge {
    double x;
    double dx;
    int y_max;
    
    ActiveEdge(double _x, double _dx, int _y_max) 
        : x(_x), dx(_dx), y_max(_y_max) {}
};

color_point_vector fill_polygon_scanline_ordered(const point_vector& polygon);
color_point_vector fill_polygon_scanline_active(const point_vector& polygon);
color_point_vector fill_polygon_seed_simple(const point_vector& polygon, int seed_x, int seed_y);
color_point_vector fill_polygon_seed_scanline(const point_vector& polygon, int seed_x, int seed_y);

bool is_on_polygon_boundary(int x, int y, const point_vector& polygon);
bool is_point_in_polygon(int x, int y, const point_vector& polygon);

#endif