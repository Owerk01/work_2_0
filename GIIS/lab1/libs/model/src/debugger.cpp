#include "debugger.h"
#include "vars.h"
#include <iostream>

Debugger::Debugger(Canvas *canvas, bool debug_m, std::vector<Point> pts,
                   int step)
    : canvas(canvas), debug_mode(debug_m), points(pts), step_i(step) {}

Debugger::~Debugger() { std::cout << "Debugger out...\n"; }

void Debugger::set_debug(bool debug) { this->debug_mode = debug; }

void Debugger::set_canvas(Canvas *cnvs) { this->canvas = cnvs; }

void Debugger::set_points(std::vector<Point> pts) { this->points = pts; }

void Debugger::reset() {
  this->points.clear();
  this->step_i = 0;
  this->debug_mode = false;
}

bool Debugger::step() {
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

void Debugger::begin_debug() {
  if (!this->points.empty()) {
    if (this->debug_mode == false) {
      while (this->step()) {
      }
      this->points.clear();
      this->step_i = 0;
    } else {
      if (this->step()) {
      } else {
        this->points.clear();
        this->step_i = 0;
      }
    }
  }
}

bool Debugger::get_debug() const { return this->debug_mode; }