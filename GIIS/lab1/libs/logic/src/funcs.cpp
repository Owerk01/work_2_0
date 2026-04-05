#include "funcs.h"
#include <cmath>
#include <tuple>
#include <vector>

std::vector<std::tuple<int, int, double> >
draw_hermite(std::pair<int, int> P1, std::pair<int, int> Force1,
	     std::pair<int, int> P2, std::pair<int, int> Force2, int accuracy)
{
	std::vector<std::tuple<int, int, double> > points;
	double dt = 1.0 / accuracy;

	for (int i = 0; i <= accuracy; i++) {
		double t = i * dt;
		double t2 = t * t;
		double t3 = t2 * t;

		double h00 = 2 * t3 - 3 * t2 + 1;
		double h10 = t3 - 2 * t2 + t;
		double h01 = -2 * t3 + 3 * t2;
		double h11 = t3 - t2;

		double x = h00 * P1.first + h10 * Force1.first +
			   h01 * P2.first + h11 * Force2.first;
		double y = h00 * P1.second + h10 * Force1.second +
			   h01 * P2.second + h11 * Force2.second;

		points.emplace_back(std::round(x), std::round(y), 0.0);
	}
	return points;
}

std::vector<std::tuple<int, int, double> >
draw_bezier(std::pair<int, int> P1, std::pair<int, int> P2,
	    std::pair<int, int> P3, std::pair<int, int> P4, int accuracy)
{
	return draw_hermite(
		P1, { 3 * (P2.first - P1.first), 3 * (P2.second - P1.second) },
		P4, { 3 * (P4.first - P3.first), 3 * (P4.second - P3.second) },
		accuracy);
}

std::vector<std::tuple<int, int, double> >
draw_b_spline_segment(std::pair<int, int> P_im1, std::pair<int, int> P_i,
		      std::pair<int, int> P_ip1, std::pair<int, int> P_ip2,
		      int accuracy)
{
	std::vector<std::tuple<int, int, double> > segment;
	double dt = 1.0 / accuracy;

	for (int k = 0; k <= accuracy; k++) {
		double t = k * dt;
		double t2 = t * t;
		double t3 = t2 * t;

		double b0 = (-t3 + 3 * t2 - 3 * t + 1) / 6.0;
		double b1 = (3 * t3 - 6 * t2 + 4) / 6.0;
		double b2 = (-3 * t3 + 3 * t2 + 3 * t + 1) / 6.0;
		double b3 = (t3) / 6.0;

		double x = b0 * std::get<0>(P_im1) + b1 * std::get<0>(P_i) +
			   b2 * std::get<0>(P_ip1) + b3 * std::get<0>(P_ip2);
		double y = b0 * std::get<1>(P_im1) + b1 * std::get<1>(P_i) +
			   b2 * std::get<1>(P_ip1) + b3 * std::get<1>(P_ip2);

		segment.emplace_back(std::round(x), std::round(y), 0.0);
	}
	return segment;
}

std::vector<std::tuple<int, int, double> >
draw_spline(std::vector<std::pair<int, int> > controlPoints, int accuracy)
{
	std::vector<std::tuple<int, int, double> > curve;
	if (controlPoints.size() < 4)
		return curve;

	for (size_t i = 1; i <= controlPoints.size() - 3; i++) {
		auto seg = draw_b_spline_segment(
			controlPoints[i - 1], controlPoints[i],
			controlPoints[i + 1], controlPoints[i + 2], accuracy);
		curve.insert(curve.end(), seg.begin(), seg.end());
	}
	return curve;
}