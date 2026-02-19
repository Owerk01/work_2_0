#include "data_handler.h"
#include "curved_lines_draw_algs.h"
#include "dynamic_dia.h"
#include "line_drawing_algs.h"
#include "vars.h"
#include <iostream>
#include <string>

DataHandler::DataHandler() : points({}) {}
DataHandler::~DataHandler() { std::cout << "Data handler out...\n"; }
std::vector<Point> DataHandler::get_points() const { return this->points; }

FRLDataHandler::FRLDataHandler() {
  DynamicDialogue dia(nullptr, "FRLine dialogue", {"x1", "y1", "x2", "y2"},
                      "Algorithm", {"CDA", "Bresenham", "Wu"});
  dia.exec();

  int x1 = dia.get_spin_by_name("x1");
  int y1 = dia.get_spin_by_name("y1");
  int x2 = dia.get_spin_by_name("x2");
  int y2 = dia.get_spin_by_name("y2");
  std::string alg_type = dia.get_combo_box();

  if (!(x1 == CODE_ERROR && y1 == CODE_ERROR && x2 == CODE_ERROR &&
        y2 == CODE_ERROR)) {
    std::vector<std::tuple<int, int, double>> pts;

    if (alg_type == "CDA") {
      pts = CDA(x1, y1, x2, y2);

    } else if (alg_type == "Bresenham") {
      pts = Bresenham(x1, y1, x2, y2);

    } else {
      pts = Wu(x1, y1, x2, y2);
    }

    for (auto [x, y, c] : pts) {
      this->points.push_back({x, y, int(255 * c)});
    }
  } else {
    this->points = {};
  }
}
FRLDataHandler::~FRLDataHandler() { std::cout << "FRLine handler out...\n"; }

SRLDataHandler::SRLDataHandler(int type) {

  std::vector<std::tuple<int, int, double>> pts = {};

  switch (type) {
  case 0: {
    DynamicDialogue dia(nullptr, "Circle dialogue", {"R", "x", "y"});
    dia.exec();
    int R = dia.get_spin_by_name("R");
    int x = dia.get_spin_by_name("x");
    int y = dia.get_spin_by_name("y");
    pts = draw_circle(R, x, y);

    break;
  }
  case 1: {
    DynamicDialogue dia(nullptr, "Elipsis dialogue", {"a", "b", "x", "y"});
    dia.exec();
    int a = dia.get_spin_by_name("a");
    int b = dia.get_spin_by_name("b");
    int x = dia.get_spin_by_name("x");
    int y = dia.get_spin_by_name("y");
    pts = draw_elipsis(a, b, x, y);
    break;
  }
  case 2: {
    DynamicDialogue dia(nullptr, "Parabola dialogue", {"p", "y_max"});
    dia.exec();
    int p = dia.get_spin_by_name("p");
    int y_max = dia.get_spin_by_name("y_max");
    pts = drawParabola(p, y_max);
    break;
  }
  case 3: {
    DynamicDialogue dia(nullptr, "Hyperbola dialogue", {"a", "b", "x_max"});
    dia.exec();
    int a = dia.get_spin_by_name("a");
    int b = dia.get_spin_by_name("b");
    int x_max = dia.get_spin_by_name("x_max");
    pts = drawHyperbola(a, b, x_max);
    break;
  }
  }
  for (auto [x, y, c] : pts) {
    this->points.push_back({x, y, int(255 * c)});
  }
}
SRLDataHandler::~SRLDataHandler() { std::cout << "SRLine handler out...\n"; }