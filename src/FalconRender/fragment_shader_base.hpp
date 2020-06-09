#ifndef __PIXELSHADERBASE_HPP__
#define __PIXELSHADERBASE_HPP__

#include <vector>
#include "pixel_data.hpp"
#include "triangle_edge_equation.hpp"

namespace flr {

	template<typename Derived>
	class FragmentShaderBase {
	public:
		static std::vector<std::vector<uint32_t>>* p_frame_buffer_;
		static std::vector<std::vector<float>>* p_depth_buffer_;

		static const int params_count_ = 0;

		static void DrawPixel(PixelData& p){}

		static void DrawSpan(const TriangleEquation& tri, int x1, int y1, int x2)
		{
			float xf = x1 + 0.5f;
			float yf = y1 + 0.5f;

			PixelData p;
			p.y_ = y1;
			p.Initialize(tri, xf, yf, Derived::params_count_);
			
			while (x1 < x2) 
			{
				p.x_ = x1;
				Derived::DrawPixel(p);
				p.StepX(Derived::params_count_);
				x1++;
			}
		}

		template<bool is_test_edge>
		static void DrawBlockInTriangle(const TriangleEquation& tri, int x, int y)
		{
			float xf = x + 0.5;
			float yf = y + 0.5;

			PixelData pixel;
			pixel.Initialize(tri, xf, yf, Derived::params_count_);

			TriEdgeEvalData eval_data;
			if (is_test_edge)
				eval_data.Initialize(tri, xf, yf);

			for (int i = y; i < y + kBlockSize; ++i)
			{
				auto temp_pixel = pixel;
				
				TriEdgeEvalData temp_eval_data;
				if (is_test_edge)
					temp_eval_data = eval_data;

				for (int j = x; j < x + kBlockSize; ++j)
				{
					if (!is_test_edge || temp_eval_data.IsInTriangle())
					{
						temp_pixel.x_ = j;
						temp_pixel.y_ = i;
						Derived::DrawPixel(temp_pixel);
					}

					temp_pixel.StepX(Derived::params_count_);
					if (is_test_edge)
						temp_eval_data.StepX(1);
				}

				pixel.StepY(Derived::params_count_);
				if (is_test_edge)
					eval_data.StepY(1);
			}
		}
	};

	template<typename Derived>
	std::vector<std::vector<uint32_t>>* FragmentShaderBase<Derived>::p_frame_buffer_ = nullptr;
	template<typename Derived>
	std::vector<std::vector<float>>* FragmentShaderBase<Derived>::p_depth_buffer_ = nullptr;


	class DummyFragmentShader : public FragmentShaderBase<DummyFragmentShader> {};

} // end namespace flr

#endif // !__PIXELSHADERBASE_HPP__
