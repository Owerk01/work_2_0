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

std::vector<std::tuple<int, int, double>> draw_hyperbola(int c_x, int c_y,
                                                         int a, int b) {
  using namespace std;

  vector<tuple<int, int, double>> points = {};

  if (a < 1) {
    return points;
  }

  int x0 = c_x;
  int y0 = c_y;
  int x = a;
  int y = 0;
  int a2 = a * a;
  int b2 = b * b;

  double d1 = static_cast<double>(b2) - static_cast<double>(a2) * 0.75;

  cout << "\nInit hyperbola drawing. a: " << a << ", center x: " << c_x
       << ", center y: " << c_y << "\n";

  while (static_cast<double>(b2) * x > static_cast<double>(a2) * y) {
    points.emplace_back(x0 + x, y0 + y, 0.0);

    cout << "Hyperbola Region 1: (" << x << ", " << y << ")\n";

    if (d1 < 0) {
      d1 += static_cast<double>(b2) * (2 * y + 3);
    } else {
      d1 += static_cast<double>(b2) * (2 * y + 3) -
            static_cast<double>(a2) * (2 * x - 2);
      x += 1;
    }
    y += 1;
    if (x > 10000 || y > 10000) {
      break;
    }
  }

  double d2 = static_cast<double>(b2) * (x + 0.5) * (x + 0.5) -
              static_cast<double>(a2) * (y + 1) * (y + 1) -
              static_cast<double>(a2) * b2;

  while (x < 500) {
    points.emplace_back(x0 + x, y0 + y, 0.0);

    cout << "Hyperbola Region 2: (" << x << ", " << y << ")\n";

    if (d2 > 0) {
      d2 -= static_cast<double>(a2) * (2 * x - 3);
    } else {
      d2 += static_cast<double>(b2) * (2 * y + 2) -
            static_cast<double>(a2) * (2 * x - 3);
      y += 1;
    }
    x += 1;

    if (x > 10000 || y > 10000) {
      break;
    }
  }

  return points;
}

std::vector<std::tuple<int, int, double>> draw_parabola(int c_x, int c_y,
                                                        int p) {
  using namespace std;

  vector<tuple<int, int, double>> points = {};

  if (p < 1) {
    return points;
  }

  int x0 = c_x;
  int y0 = c_y;
  int x = 0;
  int y = 0;
  double d = 1.0 - 2.0 * static_cast<double>(p);

  cout << "\nInit parabola drawing. p: " << p << ", center x: " << c_x
       << ", center y: " << c_y << "\n";

  while (x * x < 4 * p * y) {
    points.emplace_back(x0 + x, y0 + y, 0.0);

    cout << "Parabola Region 1: (" << x << ", " << y << ")\n";

    if (d < 0) {
      d += 2 * x + 3;
    } else {
      d += 2 * x + 3 - 4 * p;
      y += 1;
    }
    x += 1;
  }

  double d2 = (x + 0.5) * (x + 0.5) - 4.0 * p * (y + 1);
  while (y < 10000) {
    points.emplace_back(x0 + x, y0 + y, 0.0);

    cout << "Parabola Region 2: (" << x << ", " << y << ")\n";

    if (d2 > 0) {
      d2 += -4.0 * p + 2;
    } else {
      d2 += 2 * x - 4.0 * p + 2;
      x += 1;
    }
    y += 1;
  }

  return points;
}