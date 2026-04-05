#include "line_drawing_algs.h"
#include <cmath>
#include <tuple>
#include <vector>

int sign(int a)
{
	return (0 < a) - (a < 0);
}

std::vector<std::tuple<int, int, double> > draw_CDA(int x1, int y1, int x2,
						    int y2)
{
	using namespace std;
	vector<tuple<int, int, double> > points;
	int dx = x2 - x1;
	int dy = y2 - y1;
	int steps = max(abs(dx), abs(dy));

	if (steps == 0) {
		points.emplace_back(x1, y1, 0);
		return points;
	}

	for (int i = 0; i <= steps; ++i) {
		double t = static_cast<double>(i) / steps;
		double x = x1 + t * dx;
		double y = y1 + t * dy;
		points.emplace_back(static_cast<int>(round(x)),
				    static_cast<int>(round(y)), 0);
	}

	points.back() = { x2, y2, 0 };

	return points;
}

std::vector<std::tuple<int, int, double> > draw_bresenham(int x1, int y1,
							  int x2, int y2)
{
	using namespace std;
	vector<tuple<int, int, double> > points;
	int dx = abs(x2 - x1);
	int dy = abs(y2 - y1);
	int sx = sign(x2 - x1);
	int sy = sign(y2 - y1);

	bool steep = (dy > dx);
	if (steep) {
		swap(x1, y1);
		swap(x2, y2);
		swap(dx, dy);
		swap(sx, sy);
	}

	double err = (static_cast<double>(dy) / dx) - 0.5;
	int x = x1, y = y1;

	for (int i = 0; i <= dx; i++) {
		if (steep)
			points.emplace_back(y, x, 0);
		else
			points.emplace_back(x, y, 0);

		x += sx;
		err -= dy;
		if (err < 0) {
			y += sy;
			err += dx;
		}
	}

	return points;
}

std::vector<std::tuple<int, int, double> > draw_wu(int x1, int y1, int x2,
						   int y2)
{
	using namespace std;

	vector<tuple<int, int, double> > points;

	int dx = x2 - x1;
	int dy = y2 - y1;

	if (dx == 0 && dy == 0) {
		points.emplace_back(x1, y1, 0.0);
		return points;
	}

	if (dx == 0 || dy == 0 || abs(dx) == abs(dy)) {
		auto simple = draw_bresenham(x1, y1, x2, y2);
		for (auto [x, y, _] : simple)
			points.emplace_back(x, y, 0.0);
		return points;
	}

	bool steep = abs(dy) > abs(dx);

	if (steep) {
		swap(x1, y1);
		swap(x2, y2);
		swap(dx, dy);
	}

	if (x1 > x2) {
		swap(x1, x2);
		swap(y1, y2);
		dx = x2 - x1;
		dy = y2 - y1;
	}

	double gradient = static_cast<double>(dy) / dx;

	double y = y1;
	for (int x = x1; x <= x2; x++) {
		int y_floor = static_cast<int>(floor(y));
		double frac = y - y_floor;
		if (steep) {
			points.emplace_back(y_floor, x, frac);
			points.emplace_back(y_floor + 1, x, 1.0 - frac);
		} else {
			points.emplace_back(x, y_floor, frac);
			points.emplace_back(x, y_floor + 1, 1.0 - frac);
		}
		y += gradient;
	}

	points.back() = { x2, y2, 0 };

	return points;
}