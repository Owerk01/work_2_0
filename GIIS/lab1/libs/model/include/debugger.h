#pragma once
#include "alg.h"
#include <vars.h>

class Debugger {

public:
  Debugger();
  ~Debugger();
  void set_debug(bool debug);
  void set_alg(AlgHandler *alg);
  void reset();
  void begin_debug();

  bool get_debug() const;

private:
  bool debug_mode;
  AlgHandler *alg;
};