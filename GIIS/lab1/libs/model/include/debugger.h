#pragma once
#include "canvas.h"
#include "vars.h"
#include <vector>

class Debugger {

public:
  Debugger(Canvas *canvas = nullptr, bool debug_m = false,
           std::vector<Point> pts = {}, int step = 0);
  ~Debugger();

  void set_debug(bool debug);
  void set_canvas(Canvas *cnvs);
  void set_points(std::vector<Point> pts);

  bool get_debug() const;

  bool step();
  void reset();
  void begin_debug();

private:
  bool debug_mode;
  Canvas *canvas;
  std::vector<Point> points;
  int step_i;
};