#pragma once

#include "debugger.h"
#include "obj_3D.h"
#include "vars.h"
#include <tuple>
#include <vector>

class DataHandler {
public:
  DataHandler(Debugger *debugger, Figure f);
  ~DataHandler();

  void append(std::vector<Point> &parent, std::vector<Point> ch);
  void reset();
  void add_point(Point pt);
  void set_figure(Figure fig);

  Figure get_figure() const;

  std::vector<Point>
  transform_to_pts(std::vector<std::tuple<int, int, double>> pts);
  std::vector<Point> transform_to_pts(tuple_vector pts);

  std::vector<Point> connect_points(Figure f);

  void launch_debugger();
  void launch_3D_debugger();
  void rotate_last_3D(char axis);
  void scale_last_3D(char method);
  void perspective_last_3D();
  void move_last_3D(char direction);

private:
  int counter;
  Figure fig;
  std::vector<Figure> figures;
  Debugger *debugger;
};