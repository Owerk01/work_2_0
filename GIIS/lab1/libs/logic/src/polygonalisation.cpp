#include "polygonalisation.h"
#include "line_drawing_algs.h"
#include "polygon.h"
#include <algorithm>
#include <climits>
#include <cmath>
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

    for (const auto &e1 : line) {
      result.push_back(e1);
    }
  }

  return result;
}

p_point_vector get_bounding_box(const p_point &min_xy, const p_point &max_xy) {
  return {min_xy,
          {max_xy.first, min_xy.second},
          max_xy,
          {min_xy.first, max_xy.second}};
}

p_point_vector intersect_half_plane(const p_point_vector &current_region,
                                    const point &p0, const point &p1,
                                    const p_point &min_xy,
                                    const p_point &max_xy) {
  p_point_vector new_region;

  if (current_region.empty()) {
    return new_region;
  }

  p_point mid = {(p0.first + p1.first) / 2.0f, (p0.second + p1.second) / 2.0f};

  float dx = p1.first - p0.first;
  float dy = p1.second - p0.second;

  p_point normal = {static_cast<float>(p0.first - mid.first),
                    static_cast<float>(p0.second - mid.second)};

  float norm_len =
      std::sqrt(normal.first * normal.first + normal.second * normal.second);
  if (norm_len > 1e-6f) {
    normal.first /= norm_len;
    normal.second /= norm_len;
  }

  for (size_t i = 0; i < current_region.size(); ++i) {
    const p_point &p_curr = current_region[i];
    const p_point &p_next = current_region[(i + 1) % current_region.size()];

    float val_curr = (p_curr.first - mid.first) * normal.first +
                     (p_curr.second - mid.second) * normal.second;
    float val_next = (p_next.first - mid.first) * normal.first +
                     (p_next.second - mid.second) * normal.second;

    if (val_curr >= -1e-6f) {
      new_region.push_back(p_curr);
    }

    if ((val_curr > 1e-6f && val_next < -1e-6f) ||
        (val_curr < -1e-6f && val_next > 1e-6f)) {

      float t = -val_curr / (val_next - val_curr);
      p_point intersection = {p_curr.first + t * (p_next.first - p_curr.first),
                              p_curr.second +
                                  t * (p_next.second - p_curr.second)};
      new_region.push_back(intersection);
    }
  }

  return new_region;
}

p_point_vector get_voronoi_cell_for_point(const point &p0,
                                          const point_vector &pts,
                                          const p_point &min_xy,
                                          const p_point &max_xy) {

  p_point_vector region = get_bounding_box(min_xy, max_xy);

  for (const auto &p1 : pts) {
    if (p0 == p1)
      continue;

    region = intersect_half_plane(region, p0, p1, min_xy, max_xy);

    if (region.size() < 3) {
      return {};
    }
  }

  return region;
}

p_point_vector get_Voronoi_cell_vertexes(const point_vector &pts) {
  p_point_vector all_vertices;

  if (pts.size() < 3) {
    return all_vertices;
  }

  float min_x = pts[0].first, min_y = pts[0].second;
  float max_x = pts[0].first, max_y = pts[0].second;

  for (const auto &p : pts) {
    min_x = std::min(min_x, static_cast<float>(p.first));
    min_y = std::min(min_y, static_cast<float>(p.second));
    max_x = std::max(max_x, static_cast<float>(p.first));
    max_y = std::max(max_y, static_cast<float>(p.second));
  }

  float padding = std::max((max_x - min_x) * 0.2f, 5.0f);
  p_point min_xy = {min_x - padding, min_y - padding};
  p_point max_xy = {max_x + padding, max_y + padding};

  for (const auto &p : pts) {
    p_point_vector cell = get_voronoi_cell_for_point(p, pts, min_xy, max_xy);

    for (const auto &vertex : cell) {

      bool duplicate = false;
      for (const auto &existing : all_vertices) {
        if (std::abs(existing.first - vertex.first) < 1e-4f &&
            std::abs(existing.second - vertex.second) < 1e-4f) {
          duplicate = true;
          break;
        }
      }
      if (!duplicate) {
        all_vertices.push_back(vertex);
      }
    }
  }

  return all_vertices;
}

color_point_vector draw_Voronoi(const point_vector &pts) {
  color_point_vector result;

  if (pts.size() < 3) {
    return result;
  }

  p_point_vector voronoi_vertices = get_Voronoi_cell_vertexes(pts);

  if (voronoi_vertices.empty()) {
    return result;
  }

  float min_x = pts[0].first, min_y = pts[0].second;
  float max_x = pts[0].first, max_y = pts[0].second;

  for (const auto &p : pts) {
    min_x = std::min(min_x, static_cast<float>(p.first));
    min_y = std::min(min_y, static_cast<float>(p.second));
    max_x = std::max(max_x, static_cast<float>(p.first));
    max_y = std::max(max_y, static_cast<float>(p.second));
  }

  float padding = std::max((max_x - min_x) * 0.2f, 5.0f);
  p_point min_xy = {min_x - padding, min_y - padding};
  p_point max_xy = {max_x + padding, max_y + padding};

  std::vector<std::pair<p_point, p_point>> all_edges;

  for (const auto &p : pts) {
    p_point_vector cell = get_voronoi_cell_for_point(p, pts, min_xy, max_xy);

    if (cell.size() >= 3) {
      for (size_t i = 0; i < cell.size(); ++i) {
        p_point p1 = cell[i];
        p_point p2 = cell[(i + 1) % cell.size()];

        if (p1.first < p2.first - 1e-6f ||
            (std::abs(p1.first - p2.first) < 1e-6f &&
             p1.second < p2.second - 1e-6f)) {
          all_edges.push_back({p1, p2});
        } else {
          all_edges.push_back({p2, p1});
        }
      }
    }
  }

  std::vector<std::pair<p_point, p_point>> unique_edges;

  for (size_t i = 0; i < all_edges.size(); ++i) {
    bool duplicate = false;
    for (size_t j = i + 1; j < all_edges.size(); ++j) {
      if (std::abs(all_edges[i].first.first - all_edges[j].first.first) <
              1e-4f &&
          std::abs(all_edges[i].first.second - all_edges[j].first.second) <
              1e-4f &&
          std::abs(all_edges[i].second.first - all_edges[j].second.first) <
              1e-4f &&
          std::abs(all_edges[i].second.second - all_edges[j].second.second) <
              1e-4f) {
        duplicate = true;
        break;
      }
    }
    if (!duplicate) {
      unique_edges.push_back(all_edges[i]);
    }
  }

  for (const auto &edge : unique_edges) {
    color_point_vector line =
        draw_CDA(std::round(edge.first.first), std::round(edge.first.second),
                 std::round(edge.second.first), std::round(edge.second.second));

    for (const auto &point : line) {
      result.push_back(point);
    }
  }

  return result;
}