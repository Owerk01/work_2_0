#include "curved_lines_draw_algs.h"
#include <cmath>
#include <iostream>
#include <tuple>
#include <vector>

std::vector<std::tuple<int, int, double>> draw_circle(int R, int c_x, int c_y) {
  using namespace std;
  cout << "\nInit circle drawing. R: " << R << ", center x: " << c_x
       << ", center y: " << c_y << "\n";

  vector<tuple<int, int, double>> points = {};

  if (R < 1) {
    return points;
  }

  double lambda = 0;

  int x = 0;
  int y = R;
  double err = 2.0 - 2.0 * R;
  points.emplace_back(0, R, 0.0);

  while (y > 0) {
    if (err > 0) {
      lambda = 2.0 * err - 2 * x - 1;
      if (lambda <= 0) {
        x += 1;
        y -= 1;
        err = err + 2.0 * x - 2 * y + 2;
      } else {
        y -= 1;
        err = err - 2.0 * y + 1;
      }
    } else if (err < 0) {
      lambda = 2.0 * err + 2 * y - 1;
      if (lambda > 0) {
        x += 1;
        y -= 1;
        err = err + 2.0 * x - 2 * y + 2;
      } else {
        x += 1;
        err = err + 2.0 * x + 1;
      }
    } else if (err == 0) {
      x += 1;
      y -= 1;
      err = err + 2.0 * x - 2 * y + 2;
    }
    points.emplace_back(x, y, 0.0);
    cout << "x: " << x << ", y: " << y << ", err (delta): " << err
         << ", lambda: " << lambda << "\n";
  }

  int sz = points.size();
  for (int i = 0; i < sz; i++) {
    auto [x, y, _] = points[i];
    points.emplace_back(y, -x, 0);
  }
  sz = points.size();
  for (int i = 0; i < sz; i++) {
    auto [x, y, _] = points[i];
    points.emplace_back(-x, -y, 0);
  }
  sz = points.size();
  for (int i = 0; i < sz; i++) {
    auto [x, y, _] = points[i];
    points[i] = {x + c_x, y + c_y, 0};
  }

  return points;
}

std::vector<std::tuple<int, int, double>> draw_elipsis(int a, int b, int c_x,
                                                       int c_y) {
  using namespace std;
  cout << "\nInit elipsis drawing. a: " << a << ", b: " << b
       << ", center x: " << c_x << ", center y: " << c_y << "\n";

  vector<tuple<int, int, double>> points = {};

  if (a < 1 || b < 1) {
    return points;
  }

  double lambda = 0;

  int x = 0;
  int y = b;
  double err = double(pow(a, 2) + pow(b, 2) - 2 * b * pow(a, 2));
  points.emplace_back(0, b, 0.0);

  while (y > 0) {
    if (err > 0) {
      lambda = 2.0 * (err - pow(b, 2) * x) - 1;
      if (lambda <= 0) {
        x += 1;
        y -= 1;
        err = err + pow(b, 2) * (2.0 * x + 1) + pow(a, 2) * (1 - 2.0 * y);
      } else {
        y -= 1;
        err = err + pow(a, 2) * (1 - 2.0 * y);
      }
    } else if (err < 0) {
      lambda = 2.0 * err + 2 * y - 1;
      if (lambda > 0) {
        x += 1;
        y -= 1;
        err = err + pow(b, 2) * (2.0 * x + 1) + pow(a, 2) * (1 - 2.0 * y);
      } else {
        x += 1;
        err = err + pow(b, 2) * (2.0 * x + 1);
      }
    } else if (err == 0) {
      x += 1;
      y -= 1;
      err = err + pow(b, 2) * (2.0 * x + 1) + pow(a, 2) * (1 - 2.0 * y);
    }
    points.emplace_back(x, y, 0.0);
    cout << "x: " << x << ", y: " << y << ", err (delta): " << err
         << ", lambda: " << lambda << "\n";
  }

  int sz = points.size();
  for (int i = 0; i < sz; i++) {
    auto [x, y, _] = points[i];
    points.emplace_back(x, -y, 0);
  }
  sz = points.size();
  for (int i = 0; i < sz; i++) {
    auto [x, y, _] = points[i];
    points.emplace_back(-x, y, 0);
  }
  sz = points.size();
  for (int i = 0; i < sz; i++) {
    auto [x, y, _] = points[i];
    points[i] = {x + c_x, y + c_y, 0};
  }

  return points;
}

std::vector<std::tuple<int, int, double>> drawParabola(int p, int y_max) {
  using namespace std;

  vector<tuple<int, int, double>> points;
  if (p <= 0 || y_max < 0)
    return points;

  vector<tuple<int, int, double>> upper;
  int y = 0;
  long long E = 0;

  while (y <= y_max) {
    int x = static_cast<int>((1LL * y * y) / (2LL * p));
    upper.emplace_back(x, y, 0);

    y++;
  }

  for (auto [x, y, _] : upper) {
    points.emplace_back(x, y, 0);
    if (y != 0)
      points.emplace_back(x, -y, 0);
  }

  return points;
}

std::vector<std::tuple<int, int, double>> drawHyperbola(int a, int b,
                                                        int x_max) {
  using namespace std;

  vector<tuple<int, int, double>> points;
  if (a <= 0 || b <= 0 || x_max < a)
    return points;

  vector<tuple<int, int, double>> quadrant;

  int x = a;
  int y = 0;
  long long E = 1LL * b * b * x * x - 1LL * a * a * y * y - 1LL * a * a * b * b;

  while (x <= x_max) {
    quadrant.emplace_back(x, y, 0);

    long long dE_dx = 1LL * b * b * (2 * x + 1);
    long long dE_dy = -1LL * a * a * (2 * y + 1);

    if (E > 0) {
      y++;
      E += dE_dy;
    } else {
      x++;
      E += dE_dx;
    }
  }

  for (auto [x, y, _] : quadrant) {
    points.emplace_back(x, y, 0);
    if (y != 0)
      points.emplace_back(x, -y, 0);
    if (x != 0)
      points.emplace_back(-x, y, 0);
    if (x != 0 && y != 0)
      points.emplace_back(-x, -y, 0);
  }

  return points;
}