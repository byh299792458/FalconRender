#ifndef __TRIANGLE_CLIPPER_HPP__
#define __TRIANGLE_CLIPPER_HPP__

#include <vector>
#include "vertex_shader_base.hpp"

namespace flr {

	class TriangleClipper {
	public:
		TriangleClipper(std::vector<VertexShaderOutput>& output_vertices,
			int idx0, int idx1, int idx2)
			:output_vertices_(output_vertices)
		{
			tri_idx.push_back(idx0);
			tri_idx.push_back(idx1);
			tri_idx.push_back(idx2);
		}
		bool IsFullyClipped(void)const {
			return tri_idx.size() < 3;
		}
		void ClipToPlane(float A, float B, float C, float D)
		{
			if (IsFullyClipped())
				return;

			tri_idx.push_back(tri_idx[0]);
			int pre_idx = tri_idx[0];
			VertexShaderOutput& pre_vertex = output_vertices_[pre_idx];
			float pre_value = A * pre_vertex.x + B * pre_vertex.y + C * pre_vertex.z + D * pre_vertex.w;
						
			std::vector<int> result;
			for (int idx = 1; idx < tri_idx.size(); ++idx) 
			{
				if (pre_value >= 0)
					result.push_back(pre_idx);

				VertexShaderOutput& pre_vertex = output_vertices_[pre_idx];
				VertexShaderOutput& vertex = output_vertices_[idx];
				float value = A * vertex.x + B * vertex.y + C * vertex.z + D * vertex.w;

				if (sgn(pre_value) != sgn(value)) 
				{
					float t = (0 - pre_value) / (value - pre_value);
					auto&& newv = Lerp(t, pre_vertex, vertex);
					output_vertices_.push_back(newv);
					result.push_back(output_vertices_.size() - 1);
				}
				pre_idx = idx;
				pre_value = value;
			}
			using std::swap;
			swap(tri_idx, result);
		}

	public:
		std::vector<int> tri_idx;

	private:
		std::vector<VertexShaderOutput>& output_vertices_;
		template<typename T> 
		int sgn(T val) {
			return (T(0) < val) - (val < T(0));
		}
	};
}


#endif // !__TRIANGLE_CLIPPER_HPP__
