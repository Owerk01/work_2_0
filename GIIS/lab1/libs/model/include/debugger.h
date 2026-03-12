#pragma once
#include "canvas.h"
#include "vars.h"

class Debugger {

public:
  Debugger(Canvas *canvas = nullptr, bool debug_m = false, int step = 0);
  ~Debugger();

  void set_debug(bool debug);
  void set_canvas(Canvas *cnvs);
  void set_figure(Figure fig);

  bool get_debug() const;
  Canvas *get_canvas();

  bool step();
  void reset();
  void begin_debug();

private:
  bool debug_mode;
  Canvas *canvas;
  Figure fig;
  int step_i;
};