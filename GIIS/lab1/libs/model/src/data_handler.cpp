#include "data_handler.h"
#include "curved_lines_draw_algs.h"
#include "debugger.h"
#include "funcs.h"
#include "line_drawing_algs.h"
#include "obj_3D.h"
#include "polygon.h"
#include "polygon_fill.h"
#include "polygonalisation.h"
#include "vars.h"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <tuple>
#include <utility>
#include <vector>

DataHandler::DataHandler(Debugger *debugger, Figure f)
	: counter(0)
	, debugger(debugger)
	, figures({})
	, fig(f)
	, current_active_idx(CODE_ERROR)
	, control_points(CON_POINTS)
	, prev_active_idx(CODE_ERROR)
{
}

DataHandler::~DataHandler()
{
	std::cout << "Data handler out...\n";
}

int DataHandler::get_current_active_idx() const
{
	return this->current_active_idx;
}
void DataHandler::set_current_active_idx(int idx)
{
	if (idx >= 0 && idx < this->figures.size()) {
		this->current_active_idx = idx;
	}
}
int DataHandler::set_prev_active_idx() const
{
	return this->prev_active_idx;
}
void DataHandler::set_prev_active_idx(int idx)
{
	if (idx >= 0 && idx < this->figures.size()) {
		this->prev_active_idx = idx;
	}
}

void DataHandler::set_control_points(int p)
{
	if (p >= 3 && p <= CON_POINTS) {
		this->control_points = p;
	}
}
int DataHandler::get_control_points() const
{
	return this->control_points;
}
void DataHandler::set_figure(Figure fig)
{
	this->fig = fig;
}
Figure DataHandler::get_figure() const
{
	return this->fig;
}
std::vector<Figure> DataHandler::get_figures() const
{
	return this->figures;
};

void DataHandler::reset()
{
	this->fig.points.clear();
	this->figures.clear();
	this->counter = 0;
	this->current_active_idx = CODE_ERROR;
	this->current_active_idx = CODE_ERROR;
}

void DataHandler::append(std::vector<Point> &parent, std::vector<Point> ch)
{
	for (auto e : ch) {
		parent.push_back(e);
	}
}

std::vector<Point> DataHandler::connect_points(Figure f)
{
	std::vector<Point> connected;

	if (f.points.size() >= 1) {
		switch (f.fig_type) {
		case GType::CDA: {
			this->append(connected,
				     this->transform_to_pts(draw_CDA(
					     f.points[0].x, f.points[0].y,
					     f.points[1].x, f.points[1].y)));
			if (prev_active_idx != CODE_ERROR) {
				if (this->figures[prev_active_idx].fig_type ==
				    GType::ConvexPolygon) {
					point_vector temp;
					for (auto e :
					     this->figures[prev_active_idx]
						     .points) {
						temp.push_back({ e.x, e.y });
					}
					this->append(
						connected,
						this->transform_to_pts(
							get_line_polygon_intersections(
								{ { f.points[0]
									    .x,
								    f.points[0]
									    .y },
								  { f.points[1]
									    .x,
								    f.points[1]
									    .y } },
								temp)));
					for (auto &e : connected) {
						if (int(0.5 * 255) == e.r) {
							e.r = 100;
							e.g = 240;
							e.r = 100;
						}
					}
				}
			}

			break;
		}

		case GType::Bresenham: {
			this->append(connected,
				     this->transform_to_pts(draw_bresenham(
					     f.points[0].x, f.points[0].y,
					     f.points[1].x, f.points[1].y)));
			break;
		}
		case GType::Wu: {
			this->append(connected,
				     this->transform_to_pts(draw_wu(
					     f.points[0].x, f.points[0].y,
					     f.points[1].x, f.points[1].y)));
			break;
		}

		case GType::Circle: {
			int length = std::round((std::sqrt(
				std::pow(f.points[0].x - f.points[1].x, 2) +
				std::pow(f.points[0].y - f.points[1].y, 2))));
			this->append(connected,
				     this->transform_to_pts(
					     draw_circle(length, f.points[0].x,
							 f.points[0].y)));

			break;
		}
		case GType::Elipsis: {
			int length_a = std::round(
				std::abs(f.points[0].x - f.points[1].x));
			int length_b = std::round(
				std::abs(f.points[0].y - f.points[2].y));

			this->append(connected,
				     this->transform_to_pts(draw_elipsis(
					     length_a, length_b, f.points[0].x,
					     f.points[0].y)));
			break;
		}
		case GType::Parabola: {
			int length = std::round(std::sqrt(
				std::pow(f.points[0].x - f.points[1].x, 2) +
				std::pow(f.points[0].y - f.points[1].y, 2)));
			this->append(
				connected,
				this->transform_to_pts(draw_parabola(
					f.points[0].x, f.points[0].y, length)));

			break;
		}
		case GType::Hyperbola: {
			int length_a = std::round(
				std::abs(f.points[0].x - f.points[1].x));
			int length_b = std::round(
				std::abs(f.points[0].y - f.points[2].y));

			this->append(connected,
				     this->transform_to_pts(draw_hyperbola(
					     f.points[0].x, f.points[0].y,
					     length_a, length_b)));

			break;
		}
		case GType::Hermit: {
			this->append(
				connected,
				this->transform_to_pts(draw_hermite(
					{ f.points[0].x, f.points[0].y },
					{ f.points[1].x - f.points[0].x,
					  f.points[1].y - f.points[0].y },
					{ f.points[2].x, f.points[2].y },
					{ f.points[3].x - f.points[2].x,
					  f.points[3].y - f.points[2].y })));

			break;
		}
		case GType::Bezier: {
			this->append(
				connected,
				this->transform_to_pts(draw_bezier(
					{ f.points[0].x, f.points[0].y },
					{ f.points[1].x, f.points[1].y },
					{ f.points[2].x, f.points[2].y },
					{ f.points[3].x, f.points[3].y })));
			break;
		}
		case GType::BSpline: {
			std::vector<std::pair<int, int> > other_pts;
			for (auto e : f.points) {
				other_pts.push_back({ e.x, e.y });
			}
			this->append(
				connected,
				this->transform_to_pts(draw_spline(other_pts)));

			break;
		}
		case GType::Cube: {
			for (int i = 0; i < 4; i++) {
				int idx1 = (i != 3) ? i : 3;
				int idx2 = (i != 3) ? i + 1 : 0;
				this->append(connected,
					     this->transform_to_pts(draw_CDA(
						     f.points[idx1].x,
						     f.points[idx1].y,
						     f.points[idx2].x,
						     f.points[idx2].y)));
			}

			for (int i = 4; i < 8; i++) {
				int idx1 = (i != 7) ? i : 7;
				int idx2 = (i != 7) ? i + 1 : 4;
				this->append(connected,
					     this->transform_to_pts(draw_CDA(
						     f.points[idx1].x,
						     f.points[idx1].y,
						     f.points[idx2].x,
						     f.points[idx2].y)));
			}

			for (int i = 0; i < 4; i++) {
				int idx1 = i;
				int idx2 = i + 4;
				this->append(connected,
					     this->transform_to_pts(draw_CDA(
						     f.points[idx1].x,
						     f.points[idx1].y,
						     f.points[idx2].x,
						     f.points[idx2].y)));
			}

			break;
		}
		case GType::Tetrahedron: {
			for (int i = 0; i < 4; i++) {
				for (int j = i + 1; j < 4; j++) {
					this->append(
						connected,
						this->transform_to_pts(draw_CDA(
							f.points[i].x,
							f.points[i].y,
							f.points[j].x,
							f.points[j].y)));
				}
			}
			break;
		}
		case GType::Polygon: {
			for (int i = 0; i < f.points.size() - 1; i++) {
				this->append(connected,
					     this->transform_to_pts(draw_CDA(
						     f.points[i].x,
						     f.points[i].y,
						     f.points[i + 1].x,
						     f.points[i + 1].y)));
			}
			this->append(connected,
				     this->transform_to_pts(draw_CDA(
					     f.points[f.points.size() - 1].x,
					     f.points[f.points.size() - 1].y,
					     f.points[0].x, f.points[0].y)));
			break;
		}
		case GType::ConvexPolygon: {
			point_vector temp;
			for (auto e : f.points) {
				temp.push_back({ e.x, e.y });
			}

			this->append(connected,
				     this->transform_to_pts(
					     draw_convex_polygon(temp)));

			break;
		}

		case GType::FillOrderedEdges: {
			point_vector temp;
			for (auto e : f.points) {
				temp.push_back({ e.x, e.y });
			}
			this->append(
				connected,
				this->transform_to_pts(
					fill_polygon_scanline_ordered(temp)));
			for (size_t i = 0; i < f.points.size(); i++) {
				size_t next = (i + 1) % f.points.size();
				this->append(connected,
					     this->transform_to_pts(draw_CDA(
						     f.points[i].x,
						     f.points[i].y,
						     f.points[next].x,
						     f.points[next].y)));
			}
			break;
		}
		case GType::FillActiveEdges: {
			point_vector temp;
			for (auto e : f.points) {
				temp.push_back({ e.x, e.y });
			}
			this->append(
				connected,
				this->transform_to_pts(
					fill_polygon_scanline_active(temp)));
			for (size_t i = 0; i < f.points.size(); i++) {
				size_t next = (i + 1) % f.points.size();
				this->append(connected,
					     this->transform_to_pts(draw_CDA(
						     f.points[i].x,
						     f.points[i].y,
						     f.points[next].x,
						     f.points[next].y)));
			}
			break;
		}
		case GType::FillSeedSimple: {
			point_vector temp;
			for (size_t i = 0; i < f.points.size() - 1; i++) {
				temp.push_back(
					{ f.points[i].x, f.points[i].y });
			}
			int seed_x = f.points.back().x;
			int seed_y = f.points.back().y;
			for (int dy = -1; dy <= 1; dy++) {
				for (int dx = -1; dx <= 1; dx++) {
					connected.push_back({ seed_x + dx,
							      seed_y + dy, 0, 0,
							      0 });
				}
			}
			this->append(
				connected,
				this->transform_to_pts(fill_polygon_seed_simple(
					temp, seed_x, seed_y)));
			for (size_t i = 0; i < temp.size(); i++) {
				size_t next = (i + 1) % temp.size();
				this->append(connected,
					     this->transform_to_pts(draw_CDA(
						     temp[i].first,
						     temp[i].second,
						     temp[next].first,
						     temp[next].second)));
			}

			break;
		}
		case GType::FillSeedScanline: {
			point_vector temp;
			for (size_t i = 0; i < f.points.size() - 1; i++) {
				temp.push_back(
					{ f.points[i].x, f.points[i].y });
			}
			int seed_x = f.points.back().x;
			int seed_y = f.points.back().y;
			for (int dy = -1; dy <= 1; dy++) {
				for (int dx = -1; dx <= 1; dx++) {
					connected.push_back({ seed_x + dx,
							      seed_y + dy, 0, 0,
							      0 });
				}
			}
			this->append(connected,
				     this->transform_to_pts(
					     fill_polygon_seed_scanline(
						     temp, seed_x, seed_y)));
			for (size_t i = 0; i < temp.size(); i++) {
				size_t next = (i + 1) % temp.size();
				this->append(connected,
					     this->transform_to_pts(draw_CDA(
						     temp[i].first,
						     temp[i].second,
						     temp[next].first,
						     temp[next].second)));
			}
			break;
		}

		case GType::Dot: {
			if (prev_active_idx != CODE_ERROR) {
				if (this->figures[prev_active_idx].fig_type ==
				    GType::ConvexPolygon) {
					point_vector temp;
					for (auto e :
					     this->figures[prev_active_idx]
						     .points) {
						temp.push_back({ e.x, e.y });
					}

					if (is_point_inside_polygon(
						    { { f.points[0].x,
							f.points[0].y } },
						    temp)) {
						f.points[0].r = 100;
						f.points[0].g = 100;
						f.points[0].b = 240;
					}
				}
			}
			this->append(connected, f.points);

			break;
		}
		case GType::Delone: {
			point_vector temp;
			for (auto e : f.points) {
				temp.push_back({ e.x, e.y });
			}

			this->append(connected,
				     this->transform_to_pts(draw_Delone(temp)));
			break;
		}
		case GType::Voronoi: {
			point_vector temp;
			for (auto e : f.points) {
				temp.push_back({ e.x, e.y });
			}
			this->append(connected, this->transform_to_pts(
							draw_Voronoi(temp)));
			break;
		}
		default: {
			break;
		}
		}
	}
	return connected;
}

void DataHandler::update_figure()
{
	this->counter++;
	this->fig.id = this->counter;
	this->figures.push_back(this->fig);
	this->current_active_idx = this->figures.size() - 1;

	this->fig.points.clear();
}

void DataHandler::launch_debugger()
{
	this->debugger->get_canvas()->on_clear();
	bool d = this->debugger->get_debug();

	this->debugger->set_debug(false);
	Figure A;

	for (int i = 0; i < this->figures.size(); i++) {
		if (i != this->current_active_idx) {
			A.points = this->connect_points(this->figures[i]);
			this->debugger->set_figure(A);
			this->debugger->begin_debug();
		}
	}

	this->debugger->set_debug(d);

	A.points =
		this->connect_points(this->figures[this->current_active_idx]);
	this->debugger->set_figure(A);
	this->debugger->begin_debug();
}

std::vector<Point> DataHandler::transform_to_pts(
	std::vector<std::tuple<int, int, double> > raw_pts)
{
	std::vector<Point> pts;

	for (auto [x, y, c] : raw_pts) {
		pts.push_back({ x, y, static_cast<uint8_t>(c * 255) });
	}

	return pts;
}

std::vector<Point> DataHandler::transform_to_pts(tuple_vector raw_pts)
{
	std::vector<Point> pts;

	for (auto [x, y, z] : raw_pts) {
		Point p = { static_cast<int>(x), static_cast<int>(y),
			    static_cast<int>(z), 0 };
		p.presize_x = x;
		p.presize_y = y;
		p.presize_z = z;
		pts.push_back(p);
	}

	return pts;
}

void DataHandler::add_point(Point pt)
{
	if (!pt.is_temp) {
		this->fig.points.push_back(pt);
	}
	int sz = this->fig.points.size();

	switch (this->fig.fig_type) {
	case GType::CDA: {
		if (sz == 2) {
			this->update_figure();
			this->launch_debugger();
		}
		break;
	}

	case GType::Bresenham: {
		if (sz == 2) {
			this->update_figure();
			this->launch_debugger();
		}
		break;
	}
	case GType::Wu: {
		if (sz == 2) {
			this->update_figure();
			this->launch_debugger();
		}
		break;
	}
	case GType::Circle: {
		if (sz == 2) {
			this->update_figure();
			this->launch_debugger();
		}
		break;
	}
	case GType::Elipsis: {
		if (sz == 3) {
			this->update_figure();
			this->launch_debugger();
		}
		break;
	}
	case GType::Parabola: {
		if (sz == 2) {
			this->update_figure();
			this->launch_debugger();
		}
		break;
	}
	case GType::Hyperbola: {
		if (sz == 3) {
			this->update_figure();
			this->launch_debugger();
		}
		break;
	}
	case GType::Hermit: {
		if (sz == 4) {
			this->update_figure();
			this->launch_debugger();
		}
		break;
	}
	case GType::Bezier: {
		if (sz == 4) {
			this->update_figure();
			this->launch_debugger();
		}
		break;
	}
	case GType::BSpline: {
		if ((sz == this->control_points && this->control_points > 3) ||
		    ((sz == 4 && this->control_points == 3))) {
			this->update_figure();
			this->launch_debugger();
		}

		break;
	}
	case GType::Cube: {
		if (sz == 2) {
			int x0 = this->fig.points[0].x;
			int y0 = this->fig.points[0].y;

			// edge length
			int l = std::round(std::sqrt(
				std::pow(x0 - this->fig.points[1].x, 2) +
				std::pow(y0 - this->fig.points[1].y, 2)));

			this->fig.points.clear();

			this->fig.points.push_back({ x0, y0, 0, 0 });

			this->fig.points.push_back({ x0, y0, l, 0 });

			this->fig.points.push_back({ x0, y0 + l, l, 0 });

			this->fig.points.push_back({ x0, y0 + l, 0, 0 });

			this->fig.points.push_back({ x0 + l, y0, 0, 0 });

			this->fig.points.push_back({ x0 + l, y0, l, 0 });

			this->fig.points.push_back({ x0 + l, y0 + l, l, 0 });

			this->fig.points.push_back({ x0 + l, y0 + l, 0, 0 });

			this->update_figure();
			this->launch_debugger();
		}

		break;
	}
	case GType::Tetrahedron: {
		if (sz == 2) {
			int x0 = this->fig.points[0].x;
			int y0 = this->fig.points[0].y;

			// edge length
			int l = std::round(std::sqrt(
				std::pow(x0 - this->fig.points[1].x, 2) +
				std::pow(y0 - this->fig.points[1].y, 2)));

			this->fig.points.clear();

			double h = l * sqrt(2.0 / 3.0);
			double r_base = l / sqrt(3.0);
			double r_center = l / (2.0 * sqrt(3.0));

			this->fig.points.push_back({ x0, y0, 0, 0 });

			this->fig.points.push_back({ x0 + l, y0, 0, 0 });

			this->fig.points.push_back(
				{ static_cast<int>(x0 + l / 2.0),
				  static_cast<int>(y0 + l * sqrt(3.0) / 2.0), 0,
				  0 });

			double center_x =
				(x0 + (x0 + l) + (x0 + l / 2.0)) / 3.0;
			double center_y =
				(y0 + y0 + (y0 + l * sqrt(3.0) / 2.0)) / 3.0;

			this->fig.points.push_back({ static_cast<int>(center_x),
						     static_cast<int>(center_y),
						     static_cast<int>(h), 0 });

			this->update_figure();
			this->launch_debugger();
		}
		break;
	}
	case GType::Polygon: {
		if (sz == this->control_points && this->control_points > 2) {
			this->update_figure();
			this->launch_debugger();
		}

		break;
	}
	case GType::ConvexPolygon: {
		if (sz == this->control_points && this->control_points > 2) {
			point_vector temp;
			for (auto e : this->fig.points) {
				temp.push_back({ e.x, e.y });
			}
			auto convex_points = this->transform_to_pts(
				get_convex_hull_points(temp));
			this->fig.points = convex_points;
			this->update_figure();
			this->launch_debugger();
		}

		break;
	}

	case GType::FillOrderedEdges: {
		if (sz == this->control_points && this->control_points > 2) {
			this->update_figure();
			this->launch_debugger();
		}
		break;
	}

	case GType::FillActiveEdges: {
		if (sz == this->control_points && this->control_points > 2) {
			this->update_figure();
			this->launch_debugger();
		}
		break;
	}

	case GType::FillSeedSimple: {
		if (sz == this->control_points && this->control_points > 3) {
			this->update_figure();
			this->launch_debugger();
		}
		break;
	}

	case GType::FillSeedScanline: {
		if (sz == this->control_points && this->control_points > 3) {
			this->update_figure();
			this->launch_debugger();
		}
		break;
	}

	case GType::Dot: {
		if (sz == 1) {
			this->fig.points[0].r = 0;
			this->fig.points[0].g = 0;
			this->fig.points[0].b = 0;
			this->update_figure();
			this->launch_debugger();
		}
		break;
	}
	case GType::Delone: {
		if (sz == this->control_points && this->control_points > 2) {
			this->update_figure();
			this->launch_debugger();
		}
		break;
	}
	case GType::Voronoi: {
		if (sz == this->control_points && this->control_points > 2) {
			this->update_figure();
			this->launch_debugger();
		}
		break;
	}
	default: {
		break;
	}
	}
}

void DataHandler::rotate_last_3D(char axis)
{
	if (this->figures.size() > 0) {
		tuple_vector old_pts;

		for (auto e : this->figures[this->current_active_idx].points) {
			old_pts.push_back(
				{ e.presize_x, e.presize_y, e.presize_z });
		}

		tuple_vector raw_pts;

		if (this->figures[this->current_active_idx].fig_type ==
			    GType::Circle ||
		    this->figures[this->current_active_idx].fig_type ==
			    GType::Elipsis ||
		    this->figures[this->current_active_idx].fig_type ==
			    GType::Parabola ||
		    this->figures[this->current_active_idx].fig_type ==
			    GType::Hyperbola) {
			raw_pts = rotate_3D(
				old_pts, axis, true,
				{ this->figures[this->current_active_idx]
					  .points[0]
					  .presize_x,
				  this->figures[this->current_active_idx]
					  .points[0]
					  .presize_y,
				  this->figures[this->current_active_idx]
					  .points[0]
					  .presize_z });
		} else {
			raw_pts = rotate_3D(old_pts, axis);
		}
		this->figures[this->current_active_idx].points =
			this->transform_to_pts(raw_pts);

		this->launch_debugger();
	}
}

void DataHandler::scale_last_3D(char method)
{
	if (this->figures.size() > 0) {
		tuple_vector old_pts;

		for (auto e : this->figures[this->current_active_idx].points) {
			old_pts.push_back(
				{ e.presize_x, e.presize_y, e.presize_z });
		}

		tuple_vector raw_pts;

		if (this->figures[this->current_active_idx].fig_type ==
			    GType::Circle ||
		    this->figures[this->current_active_idx].fig_type ==
			    GType::Elipsis ||
		    this->figures[this->current_active_idx].fig_type ==
			    GType::Parabola ||
		    this->figures[this->current_active_idx].fig_type ==
			    GType::Hyperbola) {
			raw_pts =
				scale(old_pts, method, true,
				      { this->figures[this->current_active_idx]
						.points[0]
						.presize_x,
					this->figures[this->current_active_idx]
						.points[0]
						.presize_y,
					this->figures[this->current_active_idx]
						.points[0]
						.presize_z });
		} else {
			raw_pts = scale(old_pts, method);
		}

		this->figures[this->current_active_idx].points =
			this->transform_to_pts(raw_pts);

		this->launch_debugger();
	}
}

void DataHandler::perspective_last_3D()
{
	if (this->figures.size() > 0) {
		tuple_vector old_pts;

		for (auto e : this->figures[this->current_active_idx].points) {
			old_pts.push_back(
				{ e.presize_x, e.presize_y, e.presize_z });
		}

		std::vector<Point> pts;

		tuple_vector raw_pts;

		if (this->figures[this->current_active_idx].fig_type ==
			    GType::Circle ||
		    this->figures[this->current_active_idx].fig_type ==
			    GType::Elipsis ||
		    this->figures[this->current_active_idx].fig_type ==
			    GType::Parabola ||
		    this->figures[this->current_active_idx].fig_type ==
			    GType::Hyperbola) {
			raw_pts = perspective(
				old_pts, true,
				{ this->figures[this->current_active_idx]
					  .points[0]
					  .presize_x,
				  this->figures[this->current_active_idx]
					  .points[0]
					  .presize_y,
				  this->figures[this->current_active_idx]
					  .points[0]
					  .presize_z });
		} else {
			raw_pts = perspective(old_pts);
		}

		for (int i = 0; i < old_pts.size(); i++) {
			auto [x, y, z] = raw_pts[i];
			auto [x1, y1, z1] = old_pts[i];
			Point p = { static_cast<int>(x), static_cast<int>(y),
				    static_cast<int>(z), 0 };
			p.presize_x = x1;
			p.presize_y = y1;
			p.presize_z = z1;
			pts.push_back(p);
		}

		this->figures[this->current_active_idx].points = pts;

		this->launch_debugger();
	}
}

void DataHandler::move_last_3D(char direction)
{
	if (this->figures.size() > 0) {
		tuple_vector old_pts;

		for (auto e : this->figures[this->current_active_idx].points) {
			old_pts.push_back(
				{ e.presize_x, e.presize_y, e.presize_z });
		}

		this->figures[this->current_active_idx].points =
			this->transform_to_pts(move_fig(old_pts, direction));

		this->launch_debugger();
	}
}