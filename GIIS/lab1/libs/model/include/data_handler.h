#pragma once

#include "debugger.h"
#include "vars.h"
#include <tuple>
#include <vector>

class DataHandler {
public:
  DataHandler(Debugger *debugger);
  ~DataHandler();

  void reset();
  void launch_debugger();
  void add_point(Point pt);
  void set_figure(Figure fig);
  std::vector<Point>
      transform_to_pts(std::vector<std::tuple<int, int, double>>);

private:
  int counter;
  Figure fig;
  std::vector<Figure> figures;
  Debugger *debugger;
};