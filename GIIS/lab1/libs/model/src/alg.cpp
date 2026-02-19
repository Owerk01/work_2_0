#include "alg.h"
#include "canvas.h"
#include "dynamic_dia.h"
#include "line_drawing_algs.h"
#include "vars.h"
#include <iostream>
#include <string>

LineAlgHandler::LineAlgHandler(Canvas *canv) : canvas(canv), step_i(0) {
  DynamicDialogue dia(nullptr, "Line dialogue", {"x1", "y1", "x2", "y2"},
                      "Algorithm", {"CDA", "Bresenham", "Wu"});
  dia.exec();

  int x1 = dia.get_spin_by_name("x1");
  int y1 = dia.get_spin_by_name("y1");
  int x2 = dia.get_spin_by_name("x2");
  int y2 = dia.get_spin_by_name("y2");
  std::string alg_type = dia.get_combo_box();

  if (!(x1 == CODE_ERROR && y1 == CODE_ERROR && x2 == CODE_ERROR &&
        y2 == CODE_ERROR)) {
    std::vector<std::tuple<int, int, double>> points;

    std::cout << alg_type << " " << x1 << " " << x2 << " " << y1 << " " << y2
              << "...\n";
    if (alg_type == "CDA") {
      points = CDA(x1, y1, x2, y2);

    } else if (alg_type == "Bresenham") {
      points = Bresenham(x1, y1, x2, y2);

    } else {
      points = Wu(x1, y1, x2, y2);
    }

    for (auto [x, y, c] : points) {
      this->points.push_back({x, y, int(255 * c)});
    }
  } else {
    this->points = {};
  }
}

LineAlgHandler::~LineAlgHandler() { std::cout << "Line handler out...\n"; }

// Returns: false - algoritm is finshed, true - may continue
bool LineAlgHandler::step() {
  if (this->points.size() > 1) {
    this->canvas->set_pixel(this->points[step_i]);
    this->step_i++;
    if (this->step_i == this->points.size() - 1) {
      return false;
    } else {
      return true;
    }
  } else {
    return false;
  }
}