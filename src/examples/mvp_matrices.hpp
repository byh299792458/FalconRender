#ifndef __MVP_MATRICES_HPP__
#define __MVP_MATRICES_HPP__

#include "Eigen/Eigen"
#include "miscmath.inl.hpp"

using vec4f = Eigen::Vector4f;
using vec3f = Eigen::Vector3f;
using vec2f = Eigen::Vector2f;

Eigen::Matrix4f LookAt(const vec3f& eye_pos, const vec3f& center, const vec3f& up)
{
	auto view_z = (eye_pos - center).normalized();
	auto view_x = up.cross(view_z).normalized();
	auto view_y = view_z.cross(view_x).normalized();
	auto t = eye_pos;

	Eigen::Matrix4f view = Eigen::Matrix4f::Identity();

	view(0, 0) = view_x.x(); view(0, 1) = view_x.y(); view(0, 2) = view_x.z(); view(0, 3) = -view_x.dot(t);
	view(1, 0) = view_y.x(); view(1, 1) = view_y.y(); view(1, 2) = view_y.z(); view(1, 3) = -view_y.dot(t);
	view(2, 0) = view_z.x(); view(2, 1) = view_z.y(); view(2, 2) = view_z.z(); view(2, 3) = -view_z.dot(t);

	return view;
}

Eigen::Matrix4f Projection(float fovy, float aspect_radio, float z_near, float z_far)
{
	Eigen::Matrix4f projection = Eigen::Matrix4f::Identity();

	float n = std::abs(z_near);
	float f = std::abs(z_far);
	float t = n * std::tan(flr::math::toRadians(fovy / 2));
	float r = t * std::abs(aspect_radio);
	projection(0, 0) = n / r;
	projection(1, 1) = n / t;
	projection(2, 2) = -(f + n) / (f - n);
	projection(2, 3) = -2 * f * n / (f - n);
	projection(3, 2) = -1;
	projection(3, 3) = 0;

	return projection;
}
#endif // !__MVP_MATRICES_HPP__
