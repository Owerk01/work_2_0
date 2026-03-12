#pragma once

#include <tuple>
#include <vector>

#define ROTATE_ANGLE 3.0f

#define UPSCALE_F 1.2f
#define DOWNSCALE_F 0.8f

#define PERSPECTIVE_FACTOR 1000.0f

#define MOVE_STEP 1.0f

typedef std::vector<std::tuple<double, double, double>> tuple_vector;

double to_radians(double degrees);

std::tuple<double, double, double> find_center(const tuple_vector &pts);

void multiply_matrices_row_major(double result[4][4], const double a[4][4],
                                 const double b[4][4]);
void apply_matrix_to_point_row_major(const double matrix[4][4], double &x,
                                     double &y, double &z);

void translation_matrix_row_major(double matrix[4][4], double dx, double dy,
                                  double dz);
void rotation_x_matrix_row_major(double matrix[4][4], double angle);
void rotation_y_matrix_row_major(double matrix[4][4], double angle);
void rotation_z_matrix_row_major(double matrix[4][4], double angle);

tuple_vector rotate_3D(tuple_vector pts, char axis = 'x');

tuple_vector rotate_3D_around_center(tuple_vector pts, double center_x,
                                     double center_y, double center_z,
                                     char axis,
                                     double angle_deg = ROTATE_ANGLE);

void scale_matrix_row_major(double matrix[4][4], double sx, double sy,
                            double sz);

tuple_vector scale(tuple_vector pts, char method);

tuple_vector scale_around_center(tuple_vector pts, double center_x,
                                 double center_y, double center_z, double sx,
                                 double sy, double sz);

tuple_vector perspective(tuple_vector pts);

tuple_vector move_fig(tuple_vector pts, char direction);
