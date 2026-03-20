#include "polygonalisation.h"
#include "line_drawing_algs.h"
#include "polygon.h"
#include <algorithm>
#include <climits>
#include <cmath>
#include <iostream>
#include <utility>

edge get_start_edge(const point_vector &pts) {

  int leftmost_idx = 0;
  for (size_t i = 1; i < pts.size(); i++) {
    if (pts[i].first < pts[leftmost_idx].first) {
      leftmost_idx = i;
    }
  }

  point_vector hull;
  int current_idx = leftmost_idx;

  do {
    hull.push_back(pts[current_idx]);

    int next_idx = (current_idx + 1) % pts.size();

    if (hull.size() == 2) {
      break;
    }

    for (size_t i = 0; i < pts.size(); i++) {
      const auto &p1 = pts[current_idx];
      const auto &p2 = pts[next_idx];
      const auto &p3 = pts[i];

      int v1x = p2.first - p1.first;
      int v1y = p2.second - p1.second;
      int v2x = p3.first - p1.first;
      int v2y = p3.second - p1.second;

      int cross = v1x * v2y - v1y * v2x;

      if (cross < 0) {
        next_idx = i;
      } else if (cross == 0) {
        int dist_to_next = (p2.first - p1.first) * (p2.first - p1.first) +
                           (p2.second - p1.second) * (p2.second - p1.second);
        int dist_to_candidate =
            (p3.first - p1.first) * (p3.first - p1.first) +
            (p3.second - p1.second) * (p3.second - p1.second);
        if (dist_to_candidate > dist_to_next) {
          next_idx = i;
        }
      }
    }

    current_idx = next_idx;

  } while (current_idx != leftmost_idx && hull.size() < 2);

  return {hull[0], hull[1]};
}

bool circle_from_3_points(const point &p1, const point &p2, const point &p3,
                          std::pair<float, float> &center, float &radius) {

  float x1 = p1.first, y1 = p1.second;
  float x2 = p2.first, y2 = p2.second;
  float x3 = p3.first, y3 = p3.second;

  float area2 = (x2 - x1) * (y3 - y1) - (x3 - x1) * (y2 - y1);
  if (fabs(area2) < 1e-6f) {
    return false;
  }

  float A1 = x2 - x1;
  float B1 = y2 - y1;
  float C1 = (x2 * x2 - x1 * x1 + y2 * y2 - y1 * y1) / 2.0f;

  float A2 = x3 - x1;
  float B2 = y3 - y1;
  float C2 = (x3 * x3 - x1 * x1 + y3 * y3 - y1 * y1) / 2.0f;

  float det = A1 * B2 - A2 * B1;

  if (fabs(det) < 1e-6f) {
    return false;
  }

  center.first = (C1 * B2 - C2 * B1) / det;
  center.second = (A1 * C2 - A2 * C1) / det;

  float dx = center.first - x1;
  float dy = center.second - y1;
  radius = std::sqrt(dx * dx + dy * dy);

  return true;
}

point_vector get_oncircle_points(const edge &e, const point_vector &pts) {

  point_vector a;

  for (const auto &p : pts) {
    if (p != e.first && p != e.second) {

      std::pair<float, float> c;
      float R;
      bool success = circle_from_3_points(e.first, e.second, p, c, R);

      if (success) {
        bool is_ok = true;
        for (const auto &p2 : pts) {

          if (p2 != p && p2 != e.first && p2 != e.second) {
            float length = std::sqrt(std::pow(c.first - p2.first, 2) +
                                     std::pow(c.second - p2.second, 2));
            if (length < R) {
              is_ok = false;
              break;
            }
          }
        }
        if (is_ok) {

          a.push_back(p);
        }
      }
    }
  }
  return a;
}

bool is_alive(const edge_vector &alive, const edge &e) {

  edge rev_e = {e.second, e.first};
  auto it = std::find(alive.begin(), alive.end(), e);
  auto rev_it = std::find(alive.begin(), alive.end(), rev_e);

  if (it != alive.end() || rev_it != alive.end()) {
    return true;
  }
  return false;
}

void update_alive_dead(edge_vector &alive, edge_vector &dead, const edge &e) {
  edge rev_e = {e.second, e.first};

  auto it = std::find(alive.begin(), alive.end(), e);
  auto it2 = std::find(dead.begin(), dead.end(), e);

  auto rev_it = std::find(alive.begin(), alive.end(), rev_e);
  auto rev_it2 = std::find(dead.begin(), dead.end(), rev_e);

  if (it != alive.end() || rev_it != alive.end()) {
    alive.erase((it == alive.end()) ? rev_it : it);
    dead.push_back(e);
  } else if (it2 == dead.end() && rev_it2 == dead.end()) {
    alive.push_back(e);
  }
}

color_point_vector draw_Delone(const point_vector &pts) {

  color_point_vector result;

  if (pts.size() < 3) {
    return result;
  }

  edge_vector alive_edges, dead_edges;

  alive_edges.push_back(get_start_edge(pts));

  while (alive_edges.size() > 0) {

    edge last_edge = alive_edges[alive_edges.size() - 1];
    update_alive_dead(alive_edges, dead_edges, last_edge);

    point_vector near_points = get_oncircle_points(last_edge, pts);

    for (const auto &p : near_points) {
      edge edge_1 = {last_edge.first, p};
      edge edge_2 = {last_edge.second, p};
      if (!is_alive(alive_edges, edge_1))
        update_alive_dead(alive_edges, dead_edges, edge_1);
      if (!is_alive(alive_edges, edge_2))
        update_alive_dead(alive_edges, dead_edges, edge_2);
    }
  }

  for (const auto &e : dead_edges) {
    color_point_vector line = draw_CDA(e.first.first, e.first.second,
                                       e.second.first, e.second.second);

    for (const auto &e : line) {
      result.push_back(e);
    }
  }

  return result;
}