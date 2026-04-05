#include "curved_lines_draw_algs.h"
#include <algorithm>
#include <cmath>
#include <tuple>
#include <vector>

std::vector<std::tuple<int, int, double> > draw_circle(int R, int c_x, int c_y)
{
	using namespace std;

	vector<tuple<int, int, double> > points = {};

	if (R < 1) {
		return points;
	}

	double lambda = 0;

	int x = 0;
	int y = R;
	double err = 2.0 - 2.0 * R;
	points.emplace_back(0, R, 0.0);

	while (y > 0) {
		if (err > 0) {
			lambda = 2.0 * err - 2 * x - 1;
			if (lambda <= 0) {
				x += 1;
				y -= 1;
				err = err + 2.0 * x - 2 * y + 2;
			} else {
				y -= 1;
				err = err - 2.0 * y + 1;
			}
		} else if (err < 0) {
			lambda = 2.0 * err + 2 * y - 1;
			if (lambda > 0) {
				x += 1;
				y -= 1;
				err = err + 2.0 * x - 2 * y + 2;
			} else {
				x += 1;
				err = err + 2.0 * x + 1;
			}
		} else if (err == 0) {
			x += 1;
			y -= 1;
			err = err + 2.0 * x - 2 * y + 2;
		}
		points.emplace_back(x, y, 0.0);
	}

	int sz = points.size();
	for (int i = 0; i < sz; i++) {
		auto [x, y, _] = points[i];
		points.emplace_back(y, -x, 0);
	}
	sz = points.size();
	for (int i = 0; i < sz; i++) {
		auto [x, y, _] = points[i];
		points.emplace_back(-x, -y, 0);
	}
	sz = points.size();
	for (int i = 0; i < sz; i++) {
		auto [x, y, _] = points[i];
		points[i] = { x + c_x, y + c_y, 0 };
	}

	return points;
}

std::vector<std::tuple<int, int, double> > draw_elipsis(int a, int b, int c_x,
							int c_y)
{
	using namespace std;

	vector<tuple<int, int, double> > points = {};

	if (a < 1 || b < 1) {
		return points;
	}

	double lambda = 0;

	int x = 0;
	int y = b;
	double err = double(pow(a, 2) + pow(b, 2) - 2 * b * pow(a, 2));
	points.emplace_back(0, b, 0.0);

	while (y > 0) {
		if (err > 0) {
			lambda = 2.0 * (err - pow(b, 2) * x) - 1;
			if (lambda <= 0) {
				x += 1;
				y -= 1;
				err = err + pow(b, 2) * (2.0 * x + 1) +
				      pow(a, 2) * (1 - 2.0 * y);
			} else {
				y -= 1;
				err = err + pow(a, 2) * (1 - 2.0 * y);
			}
		} else if (err < 0) {
			lambda = 2.0 * err + 2 * y - 1;
			if (lambda > 0) {
				x += 1;
				y -= 1;
				err = err + pow(b, 2) * (2.0 * x + 1) +
				      pow(a, 2) * (1 - 2.0 * y);
			} else {
				x += 1;
				err = err + pow(b, 2) * (2.0 * x + 1);
			}
		} else if (err == 0) {
			x += 1;
			y -= 1;
			err = err + pow(b, 2) * (2.0 * x + 1) +
			      pow(a, 2) * (1 - 2.0 * y);
		}
		points.emplace_back(x, y, 0.0);
	}

	int sz = points.size();
	for (int i = 0; i < sz; i++) {
		auto [x, y, _] = points[i];
		points.emplace_back(x, -y, 0);
	}
	sz = points.size();
	for (int i = 0; i < sz; i++) {
		auto [x, y, _] = points[i];
		points.emplace_back(-x, y, 0);
	}
	sz = points.size();
	for (int i = 0; i < sz; i++) {
		auto [x, y, _] = points[i];
		points[i] = { x + c_x, y + c_y, 0 };
	}

	return points;
}

std::vector<std::tuple<int, int, double> > draw_hyperbola(int c_x, int c_y,
							  int a, int b)
{
	using namespace std;

	vector<tuple<int, int, double> > points = {};

	if (a < 1 || b < 1) {
		return points;
	}

	const int MAX_Y_OFFSET = 100;

	int x0 = c_x;
	int y0 = c_y;

	int x = a;

	for (int y = 1; y <= MAX_Y_OFFSET; ++y) {
		double x_exact = a * sqrt(1.0 + (y * y) / (double)(b * b));
		int x_int = static_cast<int>(round(x_exact));

		if (x_int > x) {
			for (int xi = x + 1; xi <= x_int; ++xi) {
				points.emplace_back(x0 + xi, y0 + y, 0.0);
			}
		} else if (x_int < x) {
			for (int xi = x - 1; xi >= x_int; --xi) {
				points.emplace_back(x0 + xi, y0 + y, 0.0);
			}
		} else {
			points.emplace_back(x0 + x_int, y0 + y, 0.0);
		}
		x = x_int;
	}

	x = a;
	for (int y = -1; y >= -MAX_Y_OFFSET; --y) {
		double x_exact = a * sqrt(1.0 + (y * y) / (double)(b * b));
		int x_int = static_cast<int>(round(x_exact));

		if (x_int > x) {
			for (int xi = x + 1; xi <= x_int; ++xi) {
				points.emplace_back(x0 + xi, y0 + y, 0.0);
			}
		} else if (x_int < x) {
			for (int xi = x - 1; xi >= x_int; --xi) {
				points.emplace_back(x0 + xi, y0 + y, 0.0);
			}
		} else {
			points.emplace_back(x0 + x_int, y0 + y, 0.0);
		}
		x = x_int;
	}

	x = -a;
	for (int y = 1; y <= MAX_Y_OFFSET; ++y) {
		double x_exact = -a * sqrt(1.0 + (y * y) / (double)(b * b));
		int x_int = static_cast<int>(round(x_exact));

		if (x_int < x) {
			for (int xi = x - 1; xi >= x_int; --xi) {
				points.emplace_back(x0 + xi, y0 + y, 0.0);
			}
		} else if (x_int > x) {
			for (int xi = x + 1; xi <= x_int; ++xi) {
				points.emplace_back(x0 + xi, y0 + y, 0.0);
			}
		} else {
			points.emplace_back(x0 + x_int, y0 + y, 0.0);
		}
		x = x_int;
	}

	x = -a;
	for (int y = -1; y >= -MAX_Y_OFFSET; --y) {
		double x_exact = -a * sqrt(1.0 + (y * y) / (double)(b * b));
		int x_int = static_cast<int>(round(x_exact));

		if (x_int < x) {
			for (int xi = x - 1; xi >= x_int; --xi) {
				points.emplace_back(x0 + xi, y0 + y, 0.0);
			}
		} else if (x_int > x) {
			for (int xi = x + 1; xi <= x_int; ++xi) {
				points.emplace_back(x0 + xi, y0 + y, 0.0);
			}
		} else {
			points.emplace_back(x0 + x_int, y0 + y, 0.0);
		}
		x = x_int;
	}

	points.emplace_back(x0 + a, y0, 0.0);
	points.emplace_back(x0 - a, y0, 0.0);

	sort(points.begin(), points.end(),
	     [x0, y0](const auto &p1, const auto &p2) {
		     double dx1 = get<0>(p1) - x0;
		     double dy1 = get<1>(p1) - y0;
		     double dx2 = get<0>(p2) - x0;
		     double dy2 = get<1>(p2) - y0;

		     double angle1 = atan2(dy1, dx1);
		     double angle2 = atan2(dy2, dx2);

		     if (angle1 < 0)
			     angle1 += 2 * M_PI;
		     if (angle2 < 0)
			     angle2 += 2 * M_PI;

		     return angle1 < angle2;
	     });

	return points;
}

std::vector<std::tuple<int, int, double> > draw_parabola(int c_x, int c_y,
							 int p)
{
	using namespace std;

	vector<tuple<int, int, double> > points = {};

	if (p < 1) {
		return points;
	}

	const int MAX_Y_OFFSET = 100;

	int x0 = c_x;
	int y0 = c_y;

	int prev_x = 0;
	for (int y = 1; y <= MAX_Y_OFFSET; ++y) {
		double x_exact = (y * y) / (4.0 * p);
		int x_int = static_cast<int>(round(x_exact));

		if (x_int > prev_x) {
			for (int xi = prev_x + 1; xi <= x_int; ++xi) {
				points.emplace_back(x0 + xi, y0 + y, 0.0);
			}
		} else if (x_int < prev_x) {
			for (int xi = prev_x - 1; xi >= x_int; --xi) {
				points.emplace_back(x0 + xi, y0 + y, 0.0);
			}
		} else {
			points.emplace_back(x0 + x_int, y0 + y, 0.0);
		}
		prev_x = x_int;
	}

	prev_x = 0;
	for (int y = -1; y >= -MAX_Y_OFFSET; --y) {
		double x_exact = (y * y) / (4.0 * p);
		int x_int = static_cast<int>(round(x_exact));

		if (x_int > prev_x) {
			for (int xi = prev_x + 1; xi <= x_int; ++xi) {
				points.emplace_back(x0 + xi, y0 + y, 0.0);
			}
		} else if (x_int < prev_x) {
			for (int xi = prev_x - 1; xi >= x_int; --xi) {
				points.emplace_back(x0 + xi, y0 + y, 0.0);
			}
		} else {
			points.emplace_back(x0 + x_int, y0 + y, 0.0);
		}
		prev_x = x_int;
	}

	points.emplace_back(x0, y0, 0.0);

	sort(points.begin(), points.end(),
	     [x0, y0](const auto &p1, const auto &p2) {
		     int x1 = get<0>(p1) - x0;
		     int y1 = get<1>(p1) - y0;
		     int x2 = get<0>(p2) - x0;
		     int y2 = get<1>(p2) - y0;

		     double angle1 = atan2(y1, x1);
		     double angle2 = atan2(y2, x2);

		     if (angle1 < 0)
			     angle1 += 2 * M_PI;
		     if (angle2 < 0)
			     angle2 += 2 * M_PI;

		     return angle1 < angle2;
	     });

	return points;
}