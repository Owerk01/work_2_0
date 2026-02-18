#include "debugger.h"
#include "alg.h"
#include <iostream>
#include <vars.h>

Debugger::Debugger() : debug_mode(false), alg(nullptr) {}
Debugger::~Debugger() {
  delete this->alg;
  std::cout << "Debugger out...\n";
}

void Debugger::set_debug(bool debug) { this->debug_mode = debug; }
void Debugger::set_alg(AlgHandler *alg) { this->alg = alg; }
void Debugger::reset() {
  this->alg = nullptr;
  this->debug_mode = false;
}

void Debugger::begin_debug() {
  if (this->alg != nullptr) {
    if (this->debug_mode == false) {
      while (this->alg->step()) {
      }
      delete this->alg;
      this->alg = nullptr;
    } else {
      if (this->alg->step()) {
      } else {
        delete this->alg;
        this->alg = nullptr;
      }
    }
  }
}

bool Debugger::get_debug() const { return this->debug_mode; }