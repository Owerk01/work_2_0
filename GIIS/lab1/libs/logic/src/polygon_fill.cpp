#include "polygon_fill.h"
#include <cmath>
#include <iostream>
#include <algorithm>
#include <set>
#include <stack>
#include <map>
#include <queue>

bool is_on_polygon_boundary(int x, int y, const point_vector& polygon) {
    int n = polygon.size();
    if (n < 3) return false;
    
    for (int i = 0; i < n; i++) {
        int x1 = polygon[i].first;
        int y1 = polygon[i].second;
        int x2 = polygon[(i + 1) % n].first;
        int y2 = polygon[(i + 1) % n].second;
        
        int dx = std::abs(x2 - x1);
        int dy = std::abs(y2 - y1);
        int sx = (x1 < x2) ? 1 : -1;
        int sy = (y1 < y2) ? 1 : -1;
        int err = dx - dy;
        
        int cx = x1;
        int cy = y1;
        
        while (true) {
            if (cx == x && cy == y) {
                return true;
            }
            if (cx == x2 && cy == y2) break;
            int e2 = 2 * err;
            if (e2 > -dy) { err -= dy; cx += sx; }
            if (e2 < dx) { err += dx; cy += sy; }
        }
    }
    return false;
}

bool is_point_in_polygon(int x, int y, const point_vector& polygon) {
    int n = polygon.size();
    if (n < 3) return false;
    
    if (is_on_polygon_boundary(x, y, polygon)) {
        return true;
    }
    
    bool inside = false;
    for (int i = 0, j = n - 1; i < n; j = i++) {
        int xi = polygon[i].first, yi = polygon[i].second;
        int xj = polygon[j].first, yj = polygon[j].second;
        
        bool intersect = ((yi > y) != (yj > y)) &&
            (x < (double)(xj - xi) * (y - yi) / (yj - yi) + xi);
        if (intersect) inside = !inside;
    }
    return inside;
}

std::vector<double> get_scanline_intersections(int y, const point_vector& polygon) {
    std::vector<double> intersections;
    int n = polygon.size();
    
    for (int i = 0; i < n; i++) {
        int x1 = polygon[i].first;
        int y1 = polygon[i].second;
        int x2 = polygon[(i + 1) % n].first;
        int y2 = polygon[(i + 1) % n].second;
        
        if (y1 == y2) continue;
        
        int edge_y_min = std::min(y1, y2);
        int edge_y_max = std::max(y1, y2);

        int prev_y = polygon[(i + n - 1) % n].second;
        bool is_local_max = (y1 > prev_y && y1 >= y2) || (y2 > y1 && y2 >= prev_y);

        bool intersects;
        if (is_local_max && y == edge_y_max) {
            intersects = true;
        } else {

            intersects = (y >= edge_y_min && y < edge_y_max);
        }
        
        if (intersects) {
            double x_intersect = x1 + (double)(y - y1) * (x2 - x1) / (y2 - y1);
            intersections.push_back(x_intersect);
        }
    }
    
    std::sort(intersections.begin(), intersections.end());
    return intersections;
}


color_point_vector fill_polygon_scanline_ordered(const point_vector& polygon) {
    color_point_vector filled_pixels;
    
    if (polygon.size() < 3) return filled_pixels;
    
    int min_y = polygon[0].second, max_y = polygon[0].second;
    int min_x = polygon[0].first, max_x = polygon[0].first;
    
    for (const auto& p : polygon) {
        min_y = std::min(min_y, p.second);
        max_y = std::max(max_y, p.second);
        min_x = std::min(min_x, p.first);
        max_x = std::max(max_x, p.first);
    }
    
    for (int y = min_y; y <= max_y; y++) {
        std::vector<double> intersections = get_scanline_intersections(y, polygon);
        
        for (size_t i = 0; i + 1 < intersections.size(); i += 2) {
            int x_start = std::max(min_x, (int)std::ceil(intersections[i]));
            int x_end = std::min(max_x, (int)std::floor(intersections[i + 1]));
            
            for (int x = x_start; x <= x_end; x++) {
                filled_pixels.push_back(std::make_tuple(x, y, 0));
            }
        }
    }
    
    return filled_pixels;
}


color_point_vector fill_polygon_scanline_active(const point_vector& polygon) {
    color_point_vector filled_pixels;
    
    if (polygon.size() < 3) return filled_pixels;
    
    int min_y = polygon[0].second, max_y = polygon[0].second;
    
    for (const auto& p : polygon) {
        min_y = std::min(min_y, p.second);
        max_y = std::max(max_y, p.second);
    }
    
    std::map<int, std::vector<ActiveEdge>> edge_table;
    
    int n = polygon.size();
    for (int i = 0; i < n; i++) {
        int x1 = polygon[i].first;
        int y1 = polygon[i].second;
        int x2 = polygon[(i + 1) % n].first;
        int y2 = polygon[(i + 1) % n].second;
        
        if (y1 == y2) continue;
        
        int edge_y_min = std::min(y1, y2);
        int edge_y_max = std::max(y1, y2);
        
        double dx = (double)(x2 - x1) / (y2 - y1);
        double x_start = (y1 < y2) ? x1 : x2;
        
        edge_table[edge_y_min].push_back(ActiveEdge(x_start, dx, edge_y_max));
    }
    
    std::vector<ActiveEdge> active_edges;
    
    for (int y = min_y; y <= max_y; y++) {
        if (edge_table.find(y) != edge_table.end()) {
            for (const auto& edge : edge_table[y]) {
                active_edges.push_back(edge);
            }
        }
        
        std::sort(active_edges.begin(), active_edges.end(),
            [](const ActiveEdge& a, const ActiveEdge& b) { return a.x < b.x; });
        
        for (size_t i = 0; i + 1 < active_edges.size(); i += 2) {
            int x_start = (int)std::ceil(active_edges[i].x);
            int x_end = (int)std::floor(active_edges[i + 1].x);
            
            for (int x = x_start; x <= x_end; x++) {
                filled_pixels.push_back(std::make_tuple(x, y, 0));
            }
        }
        
        active_edges.erase(
            std::remove_if(active_edges.begin(), active_edges.end(),
                [y](const ActiveEdge& edge) { return y >= edge.y_max; }),
            active_edges.end()
        );
        
        for (auto& edge : active_edges) {
            edge.x += edge.dx;
        }
    }
    
    return filled_pixels;
}


color_point_vector fill_polygon_seed_simple(const point_vector& polygon, int seed_x, int seed_y) {
    color_point_vector filled_pixels;
    
    if (polygon.size() < 3) return filled_pixels;
    
    int min_y = polygon[0].second, max_y = polygon[0].second;
    int min_x = polygon[0].first, max_x = polygon[0].first;
    
    for (const auto& p : polygon) {
        min_y = std::min(min_y, p.second);
        max_y = std::max(max_y, p.second);
        min_x = std::min(min_x, p.first);
        max_x = std::max(max_x, p.first);
    }
    min_x -= 2; max_x += 2;
    min_y -= 2; max_y += 2;

    if (seed_x < min_x || seed_x > max_x || seed_y < min_y || seed_y > max_y) {
        return filled_pixels;
    }
    if (is_on_polygon_boundary(seed_x, seed_y, polygon)) {
        return filled_pixels;
    }

    std::queue<std::pair<int, int>> q;
    q.push({seed_x, seed_y});
    
    std::set<std::pair<int, int>> visited;
    visited.insert({seed_x, seed_y});
    
    const int dx[4] = {0, 0, -1, 1};
    const int dy[4] = {-1, 1, 0, 0};
    
    while (!q.empty()) {
        auto [x, y] = q.front();
        q.pop();

        filled_pixels.push_back(std::make_tuple(x, y, 0));
        
        for (int dir = 0; dir < 4; dir++) {
            int nx = x + dx[dir];
            int ny = y + dy[dir];
            
            if (nx < min_x || nx > max_x || ny < min_y || ny > max_y) continue;

            if (visited.count({nx, ny})) continue;
            
            if (is_on_polygon_boundary(nx, ny, polygon)) continue;
            
            visited.insert({nx, ny});
            q.push({nx, ny});
        }
    }
    
    return filled_pixels;
}


color_point_vector fill_polygon_seed_scanline(const point_vector& polygon, int seed_x, int seed_y) {
    color_point_vector filled_pixels;
    
    if (polygon.size() < 3) return filled_pixels;
    
    int min_y = polygon[0].second, max_y = polygon[0].second;
    int min_x = polygon[0].first, max_x = polygon[0].first;
    
    for (const auto& p : polygon) {
        min_y = std::min(min_y, p.second);
        max_y = std::max(max_y, p.second);
        min_x = std::min(min_x, p.first);
        max_x = std::max(max_x, p.first);
    }
    min_x -= 2; max_x += 2;
    min_y -= 2; max_y += 2;

    if (seed_x < min_x || seed_x > max_x || seed_y < min_y || seed_y > max_y) {
        return filled_pixels;
    }
    if (is_on_polygon_boundary(seed_x, seed_y, polygon)) {
        return filled_pixels;
    }
    
    std::stack<std::pair<int, int>> seed_stack;
    seed_stack.push({seed_x, seed_y});
    
    std::set<std::pair<int, int>> filled_set;
    filled_set.insert({seed_x, seed_y});
    
    while (!seed_stack.empty()) {
        auto [x, y] = seed_stack.top();
        seed_stack.pop();

        int x_left = x;
        while (x_left >= min_x) {
            if (filled_set.find({x_left, y}) != filled_set.end()) {
                break;
            }
            if (is_on_polygon_boundary(x_left, y, polygon)) {
                break;  
            }
            filled_pixels.push_back(std::make_tuple(x_left, y, 0));
            filled_set.insert({x_left, y});
            x_left--;
        }
        x_left++;

        int x_right = x + 1;
        while (x_right <= max_x) {
            if (is_on_polygon_boundary(x_right, y, polygon) ||
                filled_set.find({x_right, y}) != filled_set.end()) {
                break;
            }
            filled_pixels.push_back(std::make_tuple(x_right, y, 0));
            filled_set.insert({x_right, y});
            x_right++;
        }
        x_right--;


        for (int check_y : {y - 1, y + 1}) {
            if (check_y < min_y || check_y > max_y) continue;
            
            int span_start = -1;
            for (int check_x = x_left; check_x <= x_right; check_x++) {
                bool is_bound = is_on_polygon_boundary(check_x, check_y, polygon);
                bool is_filled = filled_set.find({check_x, check_y}) != filled_set.end();
                
                if (!is_bound && !is_filled) {
                    if (span_start == -1) {
                        span_start = check_x;
                    }
                } else {
                    if (span_start != -1) {
                        seed_stack.push({check_x - 1, check_y});
                        span_start = -1;
                    }
                }
            }
            if (span_start != -1) {
                seed_stack.push({x_right, check_y});
            }
        }
    }
    
    return filled_pixels;
}