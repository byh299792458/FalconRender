#ifndef __LINE_CLIPER_HPP__
#define __LINE_CLIPER_HPP__

#include <cstdlib>
#include <algorithm>
#include "vertex_shader_base.hpp"

namespace flr {

	class LineClipper {
	public:
		LineClipper(VertexShaderOutput& v0, VertexShaderOutput& v1)
			:v0_ref_{ v0 }, v1_ref_{ v1 }, t0_{ 0.f }, t1_{ 1.f }, is_fully_clipped_{ false }{}

		void ClipToPlane(float A, float B, float C, float D) 
		{
			float value0 = Plane(v0_ref_, A, B, C, D);
			float value1 = Plane(v1_ref_, A, B, C, D);
			
			if (value0 < 0 && value1 < 0) {
				is_fully_clipped_ = true;
				return;
			}

			float t = -value0 / (value1 - value0);
			if (value0 < 0) {
				t0_ = std::max(t0_, t);
			}
			else {
				t1_= std::min(t1_, t);
			}
		}
			
	private:
		const VertexShaderOutput& v0_ref_;
		const VertexShaderOutput& v1_ref_;

	public:
		bool is_fully_clipped_;
		float t0_;
		float t1_;
	};
}

#endif // !__LINE_CLIPER_HPP__
