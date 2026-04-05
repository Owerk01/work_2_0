#include "obj_3D.h"
#include <cmath>

double to_radians(double degrees)
{
	return degrees * M_PI / 180.0;
}

std::tuple<double, double, double> find_center(const tuple_vector &pts)
{
	if (pts.empty())
		return std::make_tuple(0.0, 0.0, 0.0);

	double sum_x = 0.0, sum_y = 0.0, sum_z = 0.0;
	for (const auto &pt : pts) {
		sum_x += std::get<0>(pt);
		sum_y += std::get<1>(pt);
		sum_z += std::get<2>(pt);
	}

	double count = static_cast<double>(pts.size());
	return std::make_tuple(sum_x / count, sum_y / count, sum_z / count);
}

void multiply_matrices_row_major(double result[4][4], const double a[4][4],
				 const double b[4][4])
{
	double temp[4][4] = { 0 };

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			temp[i][j] = 0;
			for (int k = 0; k < 4; k++) {
				temp[i][j] += a[i][k] * b[k][j];
			}
		}
	}

	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			result[i][j] = temp[i][j];
		}
	}
}

void apply_matrix_to_point_row_major(const double matrix[4][4], double &x,
				     double &y, double &z)
{
	double original_x = x, original_y = y, original_z = z;

	double new_x = original_x * matrix[0][0] + original_y * matrix[1][0] +
		       original_z * matrix[2][0] + 1.0 * matrix[3][0];
	double new_y = original_x * matrix[0][1] + original_y * matrix[1][1] +
		       original_z * matrix[2][1] + 1.0 * matrix[3][1];
	double new_z = original_x * matrix[0][2] + original_y * matrix[1][2] +
		       original_z * matrix[2][2] + 1.0 * matrix[3][2];

	x = new_x;
	y = new_y;
	z = new_z;
}

void translation_matrix_row_major(double matrix[4][4], double dx, double dy,
				  double dz)
{
	matrix[0][0] = 1.0;
	matrix[0][1] = 0.0;
	matrix[0][2] = 0.0;
	matrix[0][3] = 0.0;
	matrix[1][0] = 0.0;
	matrix[1][1] = 1.0;
	matrix[1][2] = 0.0;
	matrix[1][3] = 0.0;
	matrix[2][0] = 0.0;
	matrix[2][1] = 0.0;
	matrix[2][2] = 1.0;
	matrix[2][3] = 0.0;
	matrix[3][0] = dx;
	matrix[3][1] = dy;
	matrix[3][2] = dz;
	matrix[3][3] = 1.0;
}

void rotation_x_matrix_row_major(double matrix[4][4], double angle_deg)
{
	double rad = to_radians(angle_deg);
	double cos_a = cos(rad);
	double sin_a = sin(rad);

	matrix[0][0] = 1.0;
	matrix[0][1] = 0.0;
	matrix[0][2] = 0.0;
	matrix[0][3] = 0.0;
	matrix[1][0] = 0.0;
	matrix[1][1] = cos_a;
	matrix[1][2] = sin_a;
	matrix[1][3] = 0.0;
	matrix[2][0] = 0.0;
	matrix[2][1] = -sin_a;
	matrix[2][2] = cos_a;
	matrix[2][3] = 0.0;
	matrix[3][0] = 0.0;
	matrix[3][1] = 0.0;
	matrix[3][2] = 0.0;
	matrix[3][3] = 1.0;
}

void rotation_y_matrix_row_major(double matrix[4][4], double angle_deg)
{
	double rad = to_radians(angle_deg);
	double cos_a = cos(rad);
	double sin_a = sin(rad);

	matrix[0][0] = cos_a;
	matrix[0][1] = 0.0;
	matrix[0][2] = -sin_a;
	matrix[0][3] = 0.0;
	matrix[1][0] = 0.0;
	matrix[1][1] = 1.0;
	matrix[1][2] = 0.0;
	matrix[1][3] = 0.0;
	matrix[2][0] = sin_a;
	matrix[2][1] = 0.0;
	matrix[2][2] = cos_a;
	matrix[2][3] = 0.0;
	matrix[3][0] = 0.0;
	matrix[3][1] = 0.0;
	matrix[3][2] = 0.0;
	matrix[3][3] = 1.0;
}

void rotation_z_matrix_row_major(double matrix[4][4], double angle_deg)
{
	double rad = to_radians(angle_deg);
	double cos_a = cos(rad);
	double sin_a = sin(rad);

	matrix[0][0] = cos_a;
	matrix[0][1] = sin_a;
	matrix[0][2] = 0.0;
	matrix[0][3] = 0.0;
	matrix[1][0] = -sin_a;
	matrix[1][1] = cos_a;
	matrix[1][2] = 0.0;
	matrix[1][3] = 0.0;
	matrix[2][0] = 0.0;
	matrix[2][1] = 0.0;
	matrix[2][2] = 1.0;
	matrix[2][3] = 0.0;
	matrix[3][0] = 0.0;
	matrix[3][1] = 0.0;
	matrix[3][2] = 0.0;
	matrix[3][3] = 1.0;
}

tuple_vector rotate_3D_around_center(tuple_vector pts, double center_x,
				     double center_y, double center_z,
				     char axis, double angle_deg)
{
	if (pts.empty())
		return pts;

	double T1[4][4];
	double R[4][4];
	double T2[4][4];
	double temp[4][4];
	double M[4][4];

	translation_matrix_row_major(T1, -center_x, -center_y, -center_z);

	switch (axis) {
	case 'x':
		rotation_x_matrix_row_major(R, angle_deg);
		break;
	case 'y':
		rotation_y_matrix_row_major(R, angle_deg);
		break;
	case 'z':
		rotation_z_matrix_row_major(R, angle_deg);
		break;
	default:
		rotation_z_matrix_row_major(R, angle_deg);
	}

	translation_matrix_row_major(T2, center_x, center_y, center_z);

	multiply_matrices_row_major(temp, T1, R);
	multiply_matrices_row_major(M, temp, T2);

	tuple_vector result;
	for (const auto &pt : pts) {
		double x = std::get<0>(pt);
		double y = std::get<1>(pt);
		double z = std::get<2>(pt);

		apply_matrix_to_point_row_major(M, x, y, z);

		result.push_back(std::make_tuple(x, y, z));
	}

	return result;
}

tuple_vector rotate_3D(tuple_vector pts, char axis, bool use_center,
		       std::tuple<double, double, double> center)
{
	if (pts.empty())
		return pts;

	double center_x, center_y, center_z;
	if (use_center) {
		auto [x1, y1, z1] = center;
		center_x = x1;
		center_y = y1;
		center_z = z1;
	} else {
		auto [x1, y1, z1] = find_center(pts);
		center_x = x1;
		center_y = y1;
		center_z = z1;
	}

	return rotate_3D_around_center(pts, center_x, center_y, center_z, axis,
				       ROTATE_ANGLE);
}

void scale_matrix_row_major(double matrix[4][4], double sx, double sy,
			    double sz)
{
	matrix[0][0] = sx;
	matrix[0][1] = 0.0;
	matrix[0][2] = 0.0;
	matrix[0][3] = 0.0;
	matrix[1][0] = 0.0;
	matrix[1][1] = sy;
	matrix[1][2] = 0.0;
	matrix[1][3] = 0.0;
	matrix[2][0] = 0.0;
	matrix[2][1] = 0.0;
	matrix[2][2] = sz;
	matrix[2][3] = 0.0;
	matrix[3][0] = 0.0;
	matrix[3][1] = 0.0;
	matrix[3][2] = 0.0;
	matrix[3][3] = 1.0;
}

tuple_vector scale_around_center(tuple_vector pts, double center_x,
				 double center_y, double center_z, double sx,
				 double sy, double sz)
{
	if (pts.empty())
		return pts;

	double T1[4][4];
	double S[4][4];
	double T2[4][4];
	double temp[4][4];
	double M[4][4];

	translation_matrix_row_major(T1, -center_x, -center_y, -center_z);

	scale_matrix_row_major(S, sx, sy, sz);

	translation_matrix_row_major(T2, center_x, center_y, center_z);

	multiply_matrices_row_major(temp, T1, S);
	multiply_matrices_row_major(M, temp, T2);

	tuple_vector result;
	for (const auto &pt : pts) {
		double x = std::get<0>(pt);
		double y = std::get<1>(pt);
		double z = std::get<2>(pt);

		apply_matrix_to_point_row_major(M, x, y, z);

		result.push_back(std::make_tuple(x, y, z));
	}

	return result;
}

tuple_vector scale(tuple_vector pts, char method, bool use_center,
		   std::tuple<double, double, double> center)
{
	if (pts.empty())
		return pts;

	double center_x, center_y, center_z;
	if (use_center) {
		auto [x1, y1, z1] = center;
		center_x = x1;
		center_y = y1;
		center_z = z1;
	} else {
		auto [x1, y1, z1] = find_center(pts);
		center_x = x1;
		center_y = y1;
		center_z = z1;
	}

	double sx, sy, sz;

	switch (method) {
	case 'u':
		sx = UPSCALE_F;
		sy = UPSCALE_F;
		sz = UPSCALE_F;
		break;
	case 'd':
		sx = DOWNSCALE_F;
		sy = DOWNSCALE_F;
		sz = DOWNSCALE_F;
		break;
	default:
		sx = 1.0;
		sy = 1.0;
		sz = 1.0;
		break;
	}

	return scale_around_center(pts, center_x, center_y, center_z, sx, sy,
				   sz);
}

tuple_vector perspective(tuple_vector pts, bool use_center,
			 std::tuple<double, double, double> center)
{
	if (pts.empty())
		return pts;

	double center_x, center_y, center_z;
	if (use_center) {
		auto [x1, y1, z1] = center;
		center_x = x1;
		center_y = y1;
		center_z = z1;
	} else {
		auto [x1, y1, z1] = find_center(pts);
		center_x = x1;
		center_y = y1;
		center_z = z1;
	}

	tuple_vector result;

	for (const auto &pt : pts) {
		double x = std::get<0>(pt);
		double y = std::get<1>(pt);
		double z = std::get<2>(pt);

		double tx = x - center_x;
		double ty = y - center_y;
		double tz = z - center_z;

		double denominator = 1.0 + tz / PERSPECTIVE_FACTOR;

		if (fabs(denominator) < 0.1) {
			denominator = (denominator >= 0) ? 0.1 : -0.1;
		}

		double px = tx / denominator;
		double py = ty / denominator;
		double pz = tz;
		result.push_back(std::make_tuple(px + center_x, py + center_y,
						 pz + center_z));
	}

	return result;
}

tuple_vector move_fig(tuple_vector pts, char direction)
{
	if (pts.empty())
		return pts;

	double dx = 0.0, dy = 0.0;

	switch (direction) {
	case 'l':
		dx = -MOVE_STEP;
		break;
	case 'r':
		dx = MOVE_STEP;
		break;
	case 'd':
		dy = -MOVE_STEP;
		break;
	case 'u':
		dy = MOVE_STEP;
		break;
	default:

		return pts;
	}

	tuple_vector result;
	for (const auto &pt : pts) {
		double x = std::get<0>(pt) + dx;
		double y = std::get<1>(pt) + dy;
		double z = std::get<2>(pt);

		result.push_back(std::make_tuple(x, y, z));
	}

	return result;
}