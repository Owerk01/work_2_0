#pragma once
#include "vars.h"
#include <vector>

class DataHandler {
public:
  DataHandler();
  ~DataHandler();
  std::vector<Point> get_points() const;

protected:
  std::vector<Point> points;
};

// first rank line data handler (simple, straight lines)
class FRLDataHandler : public DataHandler {

public:
  FRLDataHandler();
  ~FRLDataHandler();
};

// second rank line data handler (parabolas, hyperbolas, circle, elipsis)
class SRLDataHandler : public DataHandler {

public:
  SRLDataHandler(int type);
  ~SRLDataHandler();
};

// any rank line data handler (spline, interpolation)
class ARDataHandler : public DataHandler {

public:
  ARDataHandler(int type);
  ~ARDataHandler();
};