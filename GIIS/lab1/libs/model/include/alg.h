#pragma once
#include "canvas.h"
#include "vars.h"
#include <vector>

class AlgHandler {
public:
  virtual ~AlgHandler() = default;
  virtual bool step() = 0;
};

class LineAlgHandler : public AlgHandler {

public:
  LineAlgHandler(Canvas *canvas);
  ~LineAlgHandler() override;
  virtual bool step() override;

private:
  Canvas *canvas;
  std::vector<Point> points;
  int step_i;
};