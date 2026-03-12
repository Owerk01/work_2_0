#include "data_handler.h"
#include "curved_lines_draw_algs.h"
#include "debugger.h"
#include "funcs.h"
#include "line_drawing_algs.h"
#include "obj_3D.h"
#include "vars.h"
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <tuple>
#include <utility>
#include <vector>

DataHandler::DataHandler(Debugger *debugger, Figure f)
    : counter(0), debugger(debugger), figures({}), fig(f) {}

DataHandler::~DataHandler() { std::cout << "Data handler out...\n"; }

void DataHandler::set_figure(Figure fig) { this->fig = fig; }
Figure DataHandler::get_figure() const { return this->fig; }

void DataHandler::reset() {
  this->fig.points.clear();
  this->figures.clear();
  this->counter = 0;
}
void DataHandler::append(std::vector<Point> &parent, std::vector<Point> ch) {
  for (auto e : ch) {
    parent.push_back(e);
  }
}

std::vector<Point> DataHandler::connect_points(Figure f) {
  std::vector<Point> connected;

  if (f.points.size() > 1) {

    switch (f.fig_type) {
    case GType::Cube: {
      if (f.points.size() == 8) {
        for (int i = 0; i < 4; i++) {
          int idx1 = (i != 3) ? i : 3;
          int idx2 = (i != 3) ? i + 1 : 0;
          this->append(connected, this->transform_to_pts(draw_CDA(
                                      f.points[idx1].x, f.points[idx1].y,
                                      f.points[idx2].x, f.points[idx2].y)));
        }

        for (int i = 4; i < 8; i++) {
          int idx1 = (i != 7) ? i : 7;
          int idx2 = (i != 7) ? i + 1 : 4;
          this->append(connected, this->transform_to_pts(draw_CDA(
                                      f.points[idx1].x, f.points[idx1].y,
                                      f.points[idx2].x, f.points[idx2].y)));
        }

        for (int i = 0; i < 4; i++) {
          int idx1 = i;
          int idx2 = i + 4;
          this->append(connected, this->transform_to_pts(draw_CDA(
                                      f.points[idx1].x, f.points[idx1].y,
                                      f.points[idx2].x, f.points[idx2].y)));
        }
      }
      break;
    }
    case GType::Tetrahedron: {
      if (f.points.size() == 4) {
      }
      break;
    }
    default: {
      break;
    }
    }
  }
  return connected;
}

void DataHandler::launch_debugger() {
  this->counter++;
  this->fig.id = this->counter;
  this->debugger->set_figure(this->fig);
  this->debugger->begin_debug();
  this->figures.push_back(this->fig);
  this->fig.points.clear();
}

void DataHandler::launch_3D_debugger() {
  if (this->figures.size() > 0) {
    int idx = this->figures.size() - 1;
    if (this->figures[idx].fig_type == GType::Cube ||
        this->figures[idx].fig_type == GType::Tetrahedron) {

      Figure A;
      A.points = this->connect_points(this->figures[idx]);
      this->debugger->get_canvas()->on_clear();
      this->debugger->set_figure(A);
      this->debugger->begin_debug();
    }
  }
}

std::vector<Point> DataHandler::transform_to_pts(
    std::vector<std::tuple<int, int, double>> raw_pts) {
  std::vector<Point> pts;

  for (auto [x, y, c] : raw_pts) {
    pts.push_back({x, y, static_cast<uint8_t>(c * 255)});
  }

  return pts;
}

std::vector<Point>
DataHandler::transform_to_pts(std::vector<std::tuple<int, int, int>> raw_pts) {
  std::vector<Point> pts;

  for (auto [x, y, z] : raw_pts) {
    pts.push_back({x, y, z, 0});
  }

  return pts;
}

void DataHandler::add_point(Point pt) {
  this->fig.points.push_back(pt);
  int sz = this->fig.points.size();

  switch (this->fig.fig_type) {
  case GType::CDA: {
    if (sz == 2) {
      auto raw_pts = draw_CDA(this->fig.points[0].x, this->fig.points[0].y,
                              this->fig.points[1].x, this->fig.points[1].y);
      auto pts = this->transform_to_pts(raw_pts);
      this->fig.points = pts;

      this->launch_debugger();
    }
    break;
  }

  case GType::Bresenham: {
    if (sz == 2) {
      auto raw_pts =
          draw_bresenham(this->fig.points[0].x, this->fig.points[0].y,
                         this->fig.points[1].x, this->fig.points[1].y);
      auto pts = this->transform_to_pts(raw_pts);
      this->fig.points = pts;

      this->launch_debugger();
    }
    break;
  }
  case GType::Wu: {
    if (sz == 2) {
      auto raw_pts = draw_wu(this->fig.points[0].x, this->fig.points[0].y,
                             this->fig.points[1].x, this->fig.points[1].y);
      auto pts = this->transform_to_pts(raw_pts);
      this->fig.points = pts;

      this->launch_debugger();
    }
    break;
  }
  case GType::Circle: {
    if (sz == 2) {
      int length = std::round((std::sqrt(
          std::pow(this->fig.points[0].x - this->fig.points[1].x, 2) +
          std::pow(this->fig.points[0].y - this->fig.points[1].y, 2))));

      auto raw_pts =
          draw_circle(length, this->fig.points[0].x, this->fig.points[0].y);
      auto pts = this->transform_to_pts(raw_pts);
      this->fig.points = pts;

      this->launch_debugger();
    }
    break;
  }
  case GType::Elipsis: {
    if (sz == 3) {
      int length_a =
          std::round(std::abs(this->fig.points[0].x - this->fig.points[1].x));
      int length_b =
          std::round(std::abs(this->fig.points[0].y - this->fig.points[2].y));

      auto raw_pts = draw_elipsis(length_a, length_b, this->fig.points[0].x,
                                  this->fig.points[0].y);
      auto pts = this->transform_to_pts(raw_pts);
      this->fig.points = pts;

      this->launch_debugger();
    }
    break;
  }
  case GType::Parabola: {
    if (sz == 2) {
      int length = std::round(std::sqrt(
          std::pow(this->fig.points[0].x - this->fig.points[1].x, 2) +
          std::pow(this->fig.points[0].y - this->fig.points[1].y, 2)));

      auto raw_pts =
          draw_parabola(this->fig.points[0].x, this->fig.points[0].y, length);

      auto pts = this->transform_to_pts(raw_pts);
      this->fig.points = pts;

      this->launch_debugger();
    }
    break;
  }
  case GType::Hyperbola: {
    if (sz == 3) {
      int length_a =
          std::round(std::abs(this->fig.points[0].x - this->fig.points[1].x));
      int length_b =
          std::round(std::abs(this->fig.points[0].y - this->fig.points[2].y));

      auto raw_pts = draw_hyperbola(this->fig.points[0].x,
                                    this->fig.points[0].y, length_a, length_b);

      auto pts = this->transform_to_pts(raw_pts);
      this->fig.points = pts;

      this->launch_debugger();
    }
    break;
  }
  case GType::Hermit: {
    if (sz == 4) {
      auto raw_pts =
          draw_hermite({this->fig.points[0].x, this->fig.points[0].y},
                       {this->fig.points[1].x - this->fig.points[0].x,
                        this->fig.points[1].y - this->fig.points[0].y},
                       {this->fig.points[2].x, this->fig.points[2].y},
                       {this->fig.points[3].x - this->fig.points[2].x,
                        this->fig.points[3].y - this->fig.points[2].y});

      auto pts = this->transform_to_pts(raw_pts);
      this->fig.points = pts;

      this->launch_debugger();
    }
    break;
  }
  case GType::Bezier: {
    if (sz == 4) {
      auto raw_pts =
          draw_bezier({this->fig.points[0].x, this->fig.points[0].y},
                      {this->fig.points[1].x, this->fig.points[1].y},
                      {this->fig.points[2].x, this->fig.points[2].y},
                      {this->fig.points[3].x, this->fig.points[3].y});

      auto pts = this->transform_to_pts(raw_pts);
      this->fig.points = pts;

      this->launch_debugger();
    }
    break;
  }
  case GType::BSpline: {
    if (sz == 8) {

      std::vector<std::pair<int, int>> other_pts;
      for (auto e : this->fig.points) {
        other_pts.push_back({e.x, e.y});
      }
      auto raw_pts = draw_spline(other_pts);

      auto pts = this->transform_to_pts(raw_pts);

      this->fig.points = pts;

      this->launch_debugger();
    }

    break;
  }
  case GType::Cube: {
    if (sz == 2) {
      int x0 = this->fig.points[0].x;
      int y0 = this->fig.points[0].y;

      // edge length
      int l = std::round(std::sqrt(std::pow(x0 - this->fig.points[1].x, 2) +
                                   std::pow(y0 - this->fig.points[1].y, 2)));
      this->fig.points.clear();
      this->fig.points.push_back({x0, y0, 0, 0});
      this->fig.points.push_back({x0, y0, l, 0});
      this->fig.points.push_back({x0, y0 + l, l, 0});
      this->fig.points.push_back({x0, y0 + l, 0, 0});
      this->fig.points.push_back({x0 + l, y0, 0, 0});
      this->fig.points.push_back({x0 + l, y0, l, 0});
      this->fig.points.push_back({x0 + l, y0 + l, l, 0});
      this->fig.points.push_back({x0 + l, y0 + l, 0, 0});

      this->counter++;
      this->fig.id = this->counter;
      this->figures.push_back(this->fig);
      this->fig.points.clear();

      this->launch_3D_debugger();
    }

    break;
  }
  default: {
    break;
  }
  }
}

void DataHandler::rotate_last_3D(char axis) {
  if (this->figures.size() > 0 &&
      (this->figures[this->figures.size() - 1].fig_type == GType::Cube ||
       this->figures[this->figures.size() - 1].fig_type ==
           GType::Tetrahedron)) {
    int idx = this->figures.size() - 1;
    auto old_pts = this->figures[idx].points;
    std::vector<std::tuple<int, int, int>> tmp_pts;
    for (auto e : old_pts) {
      tmp_pts.push_back({e.x, e.y, e.z});
    }
    auto new_pts = rotate_3D(tmp_pts, axis);
    this->figures[idx].points.clear();
    this->figures[idx].points = this->transform_to_pts(new_pts);
    this->launch_3D_debugger();
  }
}