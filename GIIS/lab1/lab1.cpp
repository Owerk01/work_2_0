#include "lab1.h"
#include <vector>
#include <tuple>
#include <cmath>
#include <iostream>

int sign(int a)
{
    return (0 < a) - (a < 0);
}

std::vector<std::pair<int, int>> CDA(int x1, int y1, int x2, int y2) 
{
    using namespace std;
    vector<pair<int, int>> points;
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = max(abs(dx), abs(dy));

    if (steps == 0) 
    {
        points.emplace_back(x1, y1);
        return points;
    }

    for (int i = 0; i <= steps; ++i) 
    {
        double t = static_cast<double>(i) / steps;
        double x = x1 + t * dx;
        double y = y1 + t * dy;
        points.emplace_back(
            static_cast<int>(round(x)),
            static_cast<int>(round(y))
        );
    }

    points.back() = {x2, y2};

    cout << "Points of CDA:\n";
    for (auto pair: points)
    {
        cout << "(" << pair.first << ", " << pair.second << ")\n";
    }

    return points;
}

std::vector<std::pair<int, int>> Bresenham(int x1, int y1, int x2, int y2) 
{
    using namespace std;
    vector<pair<int, int>> points;

    int dx = abs(x2 - x1);
    int dy = abs(y2 - y1);
    int sx = sign(x2 - x1);
    int sy = sign(y2 - y1);

    bool steep = (dy > dx);
    if (steep) 
    {
        swap(x1, y1);
        swap(x2, y2);
        swap(dx, dy);
        swap(sx, sy);
    }

    int err = dx / 2;
    int x = x1, y = y1;

    for (int i = 0; i <= dx; i++) 
    {
        if (steep)
            points.emplace_back(y, x);
        else
            points.emplace_back(x, y);

        x += sx;
        err -= dy;
        if (err < 0) 
        {
            y += sy;
            err += dx;
        }
    }

    cout << "\nPoints of Brezenham:\n";
    for (auto pair: points)
    {
        cout << "(" << pair.first << ", " << pair.second << ")\n";
    }

    return points;
}

std::vector<std::tuple<int, int, double>> Wu(int x1, int y1, int x2, int y2) 
{
    using namespace std;

    vector<tuple<int, int, double>> points;

    int dx = x2 - x1;
    int dy = y2 - y1;

    if (dx == 0 && dy == 0) 
    {
        points.emplace_back(x1, y1, 1);
        return points;
    }

    if (dx == 0 || dy == 0 || abs(dx) == abs(dy)) 
    {
        auto simple = Bresenham(x1, y1, x2, y2);
        for (auto [x, y] : simple)
            points.emplace_back(x, y, 1.0);
        return points;
    }

    bool steep = abs(dy) > abs(dx);

    if (steep) 
    {
        swap(x1, y1);
        swap(x2, y2);
        swap(dx, dy);
    }

    if (x1 > x2) 
    {
        swap(x1, x2);
        swap(y1, y2);
        dx = x2 - x1;
        dy = y2 - y1;
    }

    double gradient = static_cast<double>(dy) / dx;

    double y = y1;
    for (int x = x1; x <= x2; x++) 
    {
        int y_floor = static_cast<int>(floor(y));
        double frac = y - y_floor;
        if (steep) 
        {
            points.emplace_back(y_floor, x, 1.0 - frac);
            points.emplace_back(y_floor + 1, x, frac);
        } 
        else 
        {
            points.emplace_back(x, y_floor, 1.0 - frac);
            points.emplace_back(x, y_floor + 1, frac);
        }
        y += gradient;
    }

    points.back() = {x2, y2, 1};

    cout << "\nPoints of Wu:\n";
    for (auto trio: points)
    {
        cout << "(" << get<0>(trio) << ", " << get<1>(trio) << ", " << get<2>(trio) << ")\n";
    } 

    return points;
}

int main()
{
    using namespace std;
    int x1 = 1;
    int y1 = 2;
    int x2 = 8;
    int y2 = 6;
    
    CDA(x1, y1, x2, y2);
    Bresenham(x1, y1, x2, y2);
    Wu(x1, y1, x2, y2);  
}