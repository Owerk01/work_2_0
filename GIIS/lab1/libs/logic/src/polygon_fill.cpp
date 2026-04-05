#include "polygon_fill.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <stack>

bool is_on_polygon_boundary(int x, int y, const point_vector &polygon)
{
	int n = polygon.size();
	if (n < 3)
		return false;

	for (int i = 0; i < n; i++) {
		int x1 = polygon[i].first;
		int y1 = polygon[i].second;
		int x2 = polygon[(i + 1) % n].first;
		int y2 = polygon[(i + 1) % n].second;

		if (x < std::min(x1, x2) - 1 || x > std::max(x1, x2) + 1 ||
		    y < std::min(y1, y2) - 1 || y > std::max(y1, y2) + 1) {
			continue;
		}

		int cross = (x - x1) * (y2 - y1) - (y - y1) * (x2 - x1);
		if (cross != 0)
			continue;

		int dot = (x - x1) * (x2 - x1) + (y - y1) * (y2 - y1);
		if (dot < 0 ||
		    dot > (x2 - x1) * (x2 - x1) + (y2 - y1) * (y2 - y1))
			continue;

		return true;
	}
	return false;
}

bool is_point_in_polygon(int x, int y, const point_vector &polygon)
{
	int n = polygon.size();
	if (n < 3)
		return false;

	if (is_on_polygon_boundary(x, y, polygon)) {
		return true;
	}

	bool inside = false;
	for (int i = 0, j = n - 1; i < n; j = i++) {
		int xi = polygon[i].first, yi = polygon[i].second;
		int xj = polygon[j].first, yj = polygon[j].second;

		bool intersect =
			((yi > y) != (yj > y)) &&
			(x < (double)(xj - xi) * (y - yi) / (yj - yi) + xi);
		if (intersect)
			inside = !inside;
	}
	return inside;
}

bool is_local_extremum(int i, const point_vector &polygon)
{
	int n = polygon.size();
	int prev = (i + n - 1) % n;
	int next = (i + 1) % n;

	int y_prev = polygon[prev].second;
	int y_curr = polygon[i].second;
	int y_next = polygon[next].second;

	return ((y_curr > y_prev && y_curr >= y_next) ||
		(y_curr < y_prev && y_curr <= y_next));
}

std::vector<double> get_scanline_intersections(int y,
					       const point_vector &polygon)
{
	std::vector<double> intersections;
	int n = polygon.size();

	for (int i = 0; i < n; i++) {
		int x1 = polygon[i].first;
		int y1 = polygon[i].second;
		int x2 = polygon[(i + 1) % n].first;
		int y2 = polygon[(i + 1) % n].second;

		if (y1 == y2)
			continue;

		int edge_y_min = std::min(y1, y2);
		int edge_y_max = std::max(y1, y2);

		bool intersects;

		if (is_local_extremum(i, polygon)) {
			intersects = (y >= edge_y_min && y <= edge_y_max);
		} else {
			intersects = (y >= edge_y_min && y < edge_y_max);
		}

		if (intersects) {
			double x_intersect =
				x1 + (double)(y - y1) * (x2 - x1) / (y2 - y1);
			intersections.push_back(x_intersect);
		}
	}

	std::sort(intersections.begin(), intersections.end());
	return intersections;
}

color_point_vector fill_polygon_scanline_ordered(const point_vector &polygon)
{
	color_point_vector filled_pixels;

	if (polygon.size() < 3)
		return filled_pixels;

	int min_y = polygon[0].second, max_y = polygon[0].second;
	int min_x = polygon[0].first, max_x = polygon[0].first;

	for (const auto &p : polygon) {
		min_y = std::min(min_y, p.second);
		max_y = std::max(max_y, p.second);
		min_x = std::min(min_x, p.first);
		max_x = std::max(max_x, p.first);
	}

	for (int y = min_y; y <= max_y; y++) {
		std::vector<double> intersections =
			get_scanline_intersections(y, polygon);

		for (size_t i = 0; i + 1 < intersections.size(); i += 2) {
			int x_start = std::max(
				min_x, (int)std::ceil(intersections[i]));
			int x_end = std::min(
				max_x, (int)std::floor(intersections[i + 1]));

			for (int x = x_start; x <= x_end; x++) {
				filled_pixels.push_back(
					std::make_tuple(x, y, 0));
			}
		}
	}

	return filled_pixels;
}

color_point_vector fill_polygon_scanline_active(const point_vector &polygon)
{
	color_point_vector filled_pixels;

	if (polygon.size() < 3)
		return filled_pixels;

	int min_y = polygon[0].second, max_y = polygon[0].second;

	for (const auto &p : polygon) {
		min_y = std::min(min_y, p.second);
		max_y = std::max(max_y, p.second);
	}

	std::map<int, std::vector<ActiveEdge> > edge_table;

	int n = polygon.size();
	for (int i = 0; i < n; i++) {
		int x1 = polygon[i].first;
		int y1 = polygon[i].second;
		int x2 = polygon[(i + 1) % n].first;
		int y2 = polygon[(i + 1) % n].second;

		if (y1 == y2)
			continue;

		int y_min, y_max;
		double x_start;

		if (y1 < y2) {
			y_min = y1;
			y_max = y2;
			x_start = x1;
		} else {
			y_min = y2;
			y_max = y1;
			x_start = x2;
		}

		double dx = (double)(x2 - x1) / (y2 - y1);

		int prev = (i + n - 1) % n;
		int next = (i + 1) % n;

		bool is_local_min = (y1 < polygon[prev].second && y1 < y2) ||
				    (y2 < polygon[next].second && y2 < y1);

		if (is_local_min) {
			edge_table[y_min].push_back(
				ActiveEdge(x_start, dx, y_max));
		} else {
			edge_table[y_min].push_back(
				ActiveEdge(x_start, dx, y_max));
		}
	}

	std::vector<ActiveEdge> active_edges;

	for (int y = min_y; y <= max_y; y++) {
		if (edge_table.find(y) != edge_table.end()) {
			for (const auto &edge : edge_table[y]) {
				active_edges.push_back(edge);
			}
		}

		active_edges.erase(
			std::remove_if(active_edges.begin(), active_edges.end(),
				       [y](const ActiveEdge &edge) {
					       return y >= edge.y_max;
				       }),
			active_edges.end());

		std::sort(active_edges.begin(), active_edges.end(),
			  [](const ActiveEdge &a, const ActiveEdge &b) {
				  return a.x < b.x;
			  });

		for (size_t i = 0; i + 1 < active_edges.size(); i += 2) {
			int x_start = (int)std::ceil(active_edges[i].x);
			int x_end = (int)std::floor(active_edges[i + 1].x);

			for (int x = x_start; x <= x_end; x++) {
				filled_pixels.push_back(
					std::make_tuple(x, y, 0));
			}
		}

		for (auto &edge : active_edges) {
			edge.x += edge.dx;
		}
	}

	return filled_pixels;
}

color_point_vector fill_polygon_seed_simple(const point_vector &polygon,
					    int seed_x, int seed_y)
{
	color_point_vector filled_pixels;

	if (polygon.size() < 3)
		return filled_pixels;

	if (!is_point_in_polygon(seed_x, seed_y, polygon)) {
		return filled_pixels;
	}

	int min_y = polygon[0].second, max_y = polygon[0].second;
	int min_x = polygon[0].first, max_x = polygon[0].first;

	for (const auto &p : polygon) {
		min_y = std::min(min_y, p.second);
		max_y = std::max(max_y, p.second);
		min_x = std::min(min_x, p.first);
		max_x = std::max(max_x, p.first);
	}

	min_x -= 5;
	max_x += 5;
	min_y -= 5;
	max_y += 5;

	std::set<std::pair<int, int> > filled_set;

	std::stack<std::pair<int, int> > stack;
	stack.push({ seed_x, seed_y });
	filled_set.insert({ seed_x, seed_y });
	filled_pixels.push_back(std::make_tuple(seed_x, seed_y, 0));

	const int dx[4] = { 1, -1, 0, 0 };
	const int dy[4] = { 0, 0, 1, -1 };

	while (!stack.empty()) {
		auto [x, y] = stack.top();
		stack.pop();

		for (int dir = 0; dir < 4; dir++) {
			int nx = x + dx[dir];
			int ny = y + dy[dir];

			if (nx < min_x || nx > max_x || ny < min_y ||
			    ny > max_y)
				continue;

			if (filled_set.count({ nx, ny }))
				continue;

			if (is_on_polygon_boundary(nx, ny, polygon))
				continue;

			if (!is_point_in_polygon(nx, ny, polygon))
				continue;

			filled_set.insert({ nx, ny });
			filled_pixels.push_back(std::make_tuple(nx, ny, 0));
			stack.push({ nx, ny });
		}
	}

	return filled_pixels;
}

color_point_vector fill_polygon_seed_scanline(const point_vector &polygon,
					      int seed_x, int seed_y)
{
	color_point_vector filled_pixels;

	if (polygon.size() < 3)
		return filled_pixels;

	if (!is_point_in_polygon(seed_x, seed_y, polygon) &&
	    !is_on_polygon_boundary(seed_x, seed_y, polygon)) {
		return filled_pixels;
	}

	int min_y = polygon[0].second, max_y = polygon[0].second;
	int min_x = polygon[0].first, max_x = polygon[0].first;

	for (const auto &p : polygon) {
		min_y = std::min(min_y, p.second);
		max_y = std::max(max_y, p.second);
		min_x = std::min(min_x, p.first);
		max_x = std::max(max_x, p.first);
	}

	std::set<std::pair<int, int> > filled_set;

	std::stack<std::pair<int, int> > seed_stack;
	seed_stack.push({ seed_x, seed_y });
	filled_set.insert({ seed_x, seed_y });
	filled_pixels.push_back(std::make_tuple(seed_x, seed_y, 0));

	while (!seed_stack.empty()) {
		auto [x, y] = seed_stack.top();
		seed_stack.pop();

		int x_left = x;
		while (x_left >= min_x) {
			if (is_on_polygon_boundary(x_left, y, polygon)) {
				if (!filled_set.count({ x_left, y })) {
					filled_set.insert({ x_left, y });
					filled_pixels.push_back(
						std::make_tuple(x_left, y, 0));
				}
				break;
			}
			x_left--;
		}

		int x_right = x;
		while (x_right <= max_x) {
			if (is_on_polygon_boundary(x_right, y, polygon)) {
				if (!filled_set.count({ x_right, y })) {
					filled_set.insert({ x_right, y });
					filled_pixels.push_back(
						std::make_tuple(x_right, y, 0));
				}
				break;
			}
			x_right++;
		}

		for (int fill_x = x_left + 1; fill_x < x_right; fill_x++) {
			if (!filled_set.count({ fill_x, y }) &&
			    is_point_in_polygon(fill_x, y, polygon)) {
				filled_set.insert({ fill_x, y });
				filled_pixels.push_back(
					std::make_tuple(fill_x, y, 0));
			}
		}

		int check_y = y - 1;
		if (check_y >= min_y) {
			bool in_span = false;
			int span_start = x_left;

			for (int check_x = x_left; check_x <= x_right;
			     check_x++) {
				bool is_in_polygon =
					is_point_in_polygon(check_x, check_y,
							    polygon) ||
					is_on_polygon_boundary(check_x, check_y,
							       polygon);
				bool is_filled =
					filled_set.count({ check_x, check_y });

				if (is_in_polygon && !is_filled) {
					if (!in_span) {
						in_span = true;
						span_start = check_x;
					}
				} else {
					if (in_span) {
						int seed_x_new = (span_start +
								  check_x - 1) /
								 2;
						if (!filled_set.count(
							    { seed_x_new,
							      check_y })) {
							seed_stack.push(
								{ seed_x_new,
								  check_y });
						}
						in_span = false;
					}
				}
			}

			if (in_span) {
				int seed_x_new = (span_start + x_right) / 2;
				if (!filled_set.count(
					    { seed_x_new, check_y })) {
					seed_stack.push(
						{ seed_x_new, check_y });
				}
			}
		}

		check_y = y + 1;
		if (check_y <= max_y) {
			bool in_span = false;
			int span_start = x_left;

			for (int check_x = x_left; check_x <= x_right;
			     check_x++) {
				bool is_in_polygon =
					is_point_in_polygon(check_x, check_y,
							    polygon) ||
					is_on_polygon_boundary(check_x, check_y,
							       polygon);
				bool is_filled =
					filled_set.count({ check_x, check_y });

				if (is_in_polygon && !is_filled) {
					if (!in_span) {
						in_span = true;
						span_start = check_x;
					}
				} else {
					if (in_span) {
						int seed_x_new = (span_start +
								  check_x - 1) /
								 2;
						if (!filled_set.count(
							    { seed_x_new,
							      check_y })) {
							seed_stack.push(
								{ seed_x_new,
								  check_y });
						}
						in_span = false;
					}
				}
			}

			if (in_span) {
				int seed_x_new = (span_start + x_right) / 2;
				if (!filled_set.count(
					    { seed_x_new, check_y })) {
					seed_stack.push(
						{ seed_x_new, check_y });
				}
			}
		}
	}

	return filled_pixels;
}