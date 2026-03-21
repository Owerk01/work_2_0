#pragma once
#include "polygon.h"
#include <tuple>
#include <vector>

typedef std::pair<int, int> point;
typedef std::pair<point, point> edge;
typedef std::vector<point> point_vector;
typedef std::vector<edge> edge_vector;

typedef std::pair<float, float> p_point;
typedef std::pair<p_point, p_point> p_edge;
typedef std::vector<p_point> p_point_vector;
typedef std::vector<p_edge> p_edge_vector;

typedef std::vector<std::tuple<int, int, double>> color_point_vector;

edge get_start_edge(const point_vector &pts);
bool circle_from_3_points(const point &p1, const point &p2, const point &p3,
                          std::pair<float, float> &center, float &radius);
point_vector get_oncircle_points(const edge &e, const point_vector &pts);
void update_alive_dead(edge_vector &alive, edge_vector &dead, const edge &a);
color_point_vector draw_Delone(const point_vector &pts);

color_point_vector draw_Voronoi(const point_vector &pts);
