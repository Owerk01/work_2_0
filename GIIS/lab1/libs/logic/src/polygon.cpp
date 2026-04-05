#include "polygon.h"
#include "line_drawing_algs.h"
#include <algorithm>
#include <cmath>

bool is_convex(const point_vector &points)
{
	if (points.size() < 3)
		return false;

	int n = points.size();
	int sign = 0;

	for (int i = 0; i < n; i++) {
		int x1 = points[i].first;
		int y1 = points[i].second;
		int x2 = points[(i + 1) % n].first;
		int y2 = points[(i + 1) % n].second;
		int x3 = points[(i + 2) % n].first;
		int y3 = points[(i + 2) % n].second;

		int v1x = x2 - x1;
		int v1y = y2 - y1;
		int v2x = x3 - x2;
		int v2y = y3 - y2;

		int cross_product = v1x * v2y - v1y * v2x;

		int current_sign = 0;
		if (cross_product > 0)
			current_sign = 1;
		else if (cross_product < 0)
			current_sign = -1;

		if (i == 0) {
			sign = current_sign;
		} else if (current_sign != 0 && current_sign != sign) {
			return false;
		}
	}

	return true;
}

double calculate_polar_angle(int base_x, int base_y, int x, int y)
{
	int dx = x - base_x;
	int dy = y - base_y;

	if (dx == 0 && dy == 0)
		return 0;

	double angle = atan2(dy, dx);
	if (angle < 0)
		angle += 2 * M_PI;

	return angle;
}

point_vector jarvis_march(const point_vector &points)
{
	if (points.size() < 3)
		return points;

	int leftmost_idx = 0;
	for (size_t i = 1; i < points.size(); i++) {
		if (points[i].first < points[leftmost_idx].first) {
			leftmost_idx = i;
		}
	}

	point_vector hull;
	int current_idx = leftmost_idx;

	do {
		hull.push_back(points[current_idx]);

		int next_idx = (current_idx + 1) % points.size();

		for (size_t i = 0; i < points.size(); i++) {
			const auto &p1 = points[current_idx];
			const auto &p2 = points[next_idx];
			const auto &p3 = points[i];

			int v1x = p2.first - p1.first;
			int v1y = p2.second - p1.second;
			int v2x = p3.first - p1.first;
			int v2y = p3.second - p1.second;

			int cross = v1x * v2y - v1y * v2x;

			if (cross < 0) {
				next_idx = i;
			} else if (cross == 0) {
				int dist_to_next =
					(p2.first - p1.first) *
						(p2.first - p1.first) +
					(p2.second - p1.second) *
						(p2.second - p1.second);
				int dist_to_candidate =
					(p3.first - p1.first) *
						(p3.first - p1.first) +
					(p3.second - p1.second) *
						(p3.second - p1.second);
				if (dist_to_candidate > dist_to_next) {
					next_idx = i;
				}
			}
		}

		current_idx = next_idx;

	} while (current_idx != leftmost_idx);

	return hull;
}

point_vector graham_scan(const point_vector &points)
{
	if (points.size() < 3)
		return points;

	int min_y_idx = 0;
	for (size_t i = 1; i < points.size(); i++) {
		if (points[i].second < points[min_y_idx].second ||
		    (points[i].second == points[min_y_idx].second &&
		     points[i].first < points[min_y_idx].first)) {
			min_y_idx = i;
		}
	}

	point_vector sorted_points;
	sorted_points.push_back(points[min_y_idx]);
	for (size_t i = 0; i < points.size(); i++) {
		if (i != min_y_idx) {
			sorted_points.push_back(points[i]);
		}
	}

	auto p0 = sorted_points[0];
	std::sort(sorted_points.begin() + 1, sorted_points.end(),
		  [&p0](const std::pair<int, int> &a,
			const std::pair<int, int> &b) {
			  double angle_a = calculate_polar_angle(
				  p0.first, p0.second, a.first, a.second);
			  double angle_b = calculate_polar_angle(
				  p0.first, p0.second, b.first, b.second);

			  if (angle_a == angle_b) {
				  int dist_a = (a.first - p0.first) *
						       (a.first - p0.first) +
					       (a.second - p0.second) *
						       (a.second - p0.second);
				  int dist_b = (b.first - p0.first) *
						       (b.first - p0.first) +
					       (b.second - p0.second) *
						       (b.second - p0.second);
				  return dist_a < dist_b;
			  }
			  return angle_a < angle_b;
		  });

	point_vector hull;
	hull.push_back(sorted_points[0]);
	hull.push_back(sorted_points[1]);

	for (size_t i = 2; i < sorted_points.size(); i++) {
		while (hull.size() >= 2) {
			const auto &p1 = hull[hull.size() - 2];
			const auto &p2 = hull[hull.size() - 1];
			const auto &p3 = sorted_points[i];

			int v1x = p2.first - p1.first;
			int v1y = p2.second - p1.second;
			int v2x = p3.first - p2.first;
			int v2y = p3.second - p2.second;

			int cross = v1x * v2y - v1y * v2x;

			if (cross <= 0) {
				hull.pop_back();
			} else {
				break;
			}
		}
		hull.push_back(sorted_points[i]);
	}

	return hull;
}

color_point_vector get_convex_hull_points(const point_vector &vertices)
{
	color_point_vector hull_points;

	if (vertices.size() < 3) {
		return hull_points;
	}

	point_vector hull = jarvis_march(vertices);

	for (const auto &point : hull) {
		hull_points.push_back(
			std::make_tuple(point.first, point.second, 0.0));
	}

	return hull_points;
}

color_point_vector draw_convex_polygon(point_vector vertices)
{
	color_point_vector all_points;

	if (vertices.size() < 3) {
		return all_points;
	}

	if (!is_convex(vertices)) {
		// std::cout << "Polygon is not convex! Elimiting uncovex points...\n";

		vertices = graham_scan(vertices);
	}

	for (size_t i = 0; i < vertices.size(); i++) {
		const auto &current = vertices[i];
		const auto &next = vertices[(i + 1) % vertices.size()];

		std::vector<std::tuple<int, int, double> > line_points =
			draw_CDA(current.first, current.second, next.first,
				 next.second);

		all_points.insert(all_points.end(), line_points.begin(),
				  line_points.end());
	}

	return all_points;
}

bool is_point_inside_polygon(const point_vector &point,
			     const point_vector &polygon)
{
	if (point.size() != 1 || polygon.size() < 3)
		return false;

	int x = point[0].first;
	int y = point[0].second;

	bool inside = false;
	int n = polygon.size();

	for (int i = 0, j = n - 1; i < n; j = i++) {
		int xi = polygon[i].first;
		int yi = polygon[i].second;
		int xj = polygon[j].first;
		int yj = polygon[j].second;

		bool intersect =
			((yi > y) != (yj > y)) &&
			(x < (xj - xi) * (y - yi) / (double)(yj - yi) + xi);

		if (intersect)
			inside = !inside;
	}

	return inside;
}

color_point_vector get_line_polygon_intersections(const point_vector &line,
						  const point_vector &polygon)
{
	color_point_vector intersections;

	if (line.size() != 2 || polygon.size() < 3)
		return intersections;

	int x1 = line[0].first;
	int y1 = line[0].second;
	int x2 = line[1].first;
	int y2 = line[1].second;

	color_point_vector line_points = draw_CDA(x1, y1, x2, y2);
	intersections = line_points;

	int n = polygon.size();

	for (size_t k = 0; k < line_points.size(); k++) {
		int x = std::get<0>(line_points[k]);
		int y = std::get<1>(line_points[k]);

		for (int i = 0; i < n; i++) {
			int j = (i + 1) % n;

			int x3 = polygon[i].first;
			int y3 = polygon[i].second;
			int x4 = polygon[j].first;
			int y4 = polygon[j].second;

			double denom =
				(x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4);

			if (denom == 0)
				continue;

			double t = ((x1 - x3) * (y3 - y4) -
				    (y1 - y3) * (x3 - x4)) /
				   denom;
			double u = -((x1 - x2) * (y1 - y3) -
				     (y1 - y2) * (x1 - x3)) /
				   denom;

			if (t >= 0 && t <= 1 && u >= 0 && u <= 1) {
				int ix = x1 + t * (x2 - x1);
				int iy = y1 + t * (y2 - y1);

				if (std::abs(ix - x) < 2 &&
				    std::abs(iy - y) < 2) {
					intersections[k] =
						std::make_tuple(ix, iy, 0.5);
					break;
				}
			}
		}
	}

	return intersections;
}