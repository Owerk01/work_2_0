#include <tuple>
#include <vector>

std::vector<std::tuple<int, int, double>>
drawHermite(std::pair<int, int> P1, std::pair<int, int> Force1,
            std::pair<int, int> P2, std::pair<int, int> Force2,
            int accuracy = 1000);

std::vector<std::tuple<int, int, double>>
drawBezier(std::pair<int, int> P1, std::pair<int, int> P2,
           std::pair<int, int> P3, std::pair<int, int> P4, int accuracy = 1000);

std::vector<std::tuple<int, int, double>>
drawBSplineSegment(std::pair<int, int> P_im1, std::pair<int, int> P_i,
                   std::pair<int, int> P_ip1, std::pair<int, int> P_ip2,
                   int accuracy);

std::vector<std::tuple<int, int, double>>
drawBSpline(std::vector<std::pair<int, int>> controlPoints,
            int accuracy = 1000);