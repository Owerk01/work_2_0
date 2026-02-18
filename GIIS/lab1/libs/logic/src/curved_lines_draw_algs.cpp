#include "curved_lines_draw_algs.h"
#include <vector>
#include <tuple>
#include <cmath>

std::vector<std::tuple<int, int, double>> drawParabola(int p, int y_max) 
{
    using namespace std;
    
    vector<tuple<int, int, double>> points;
    if (p <= 0 || y_max < 0) 
        return points;

    vector<tuple<int, int, double>> upper;
    int y = 0;
    long long E = 0;

    while (y <= y_max) 
    {
        int x = static_cast<int>((1LL * y * y) / (2LL * p));
        upper.emplace_back(x, y, 0);

        y++;
    }

    for (auto [x, y] : upper) {
        points.emplace_back(x, y, 0);    
        if (y != 0)
            points.emplace_back(x, -y, 0); 
    }

    return points;
}

std::vector<std::tuple<int, int, double>> drawHyperbola(int a, int b, int x_max) 
{
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

    for (auto [x, y] : quadrant) {
        points.emplace_back(x, y, 0);
        if (y != 0) points.emplace_back(x, -y, 0);
        if (x != 0) points.emplace_back(-x, y, 0);
        if (x != 0 && y != 0) points.emplace_back(-x, -y, 0);
    }

    return points;
}