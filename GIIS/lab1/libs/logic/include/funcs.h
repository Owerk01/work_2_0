#include <tuple>
#include <vector>

std::vector<std::tuple<int, int, double>>
draw_hermite(std::pair<int, int> P1, std::pair<int, int> Force1,
             std::pair<int, int> P2, std::pair<int, int> Force2,
             int accuracy = 1000);

std::vector<std::tuple<int, int, double>> draw_bezier(std::pair<int, int> P1,
                                                      std::pair<int, int> P2,
                                                      std::pair<int, int> P3,
                                                      std::pair<int, int> P4,
                                                      int accuracy = 1000);

std::vector<std::tuple<int, int, double>>
draw_b_spline_segment(std::pair<int, int> P_im1, std::pair<int, int> P_i,
                      std::pair<int, int> P_ip1, std::pair<int, int> P_ip2,
                      int accuracy);

std::vector<std::tuple<int, int, double>>
draw_spline(std::vector<std::pair<int, int>> controlPoints,
            int accuracy = 1000);