#ifndef __RASTERIZER_HPP__
#define __RASTERIZER_HPP__

#include <array>
#include <vector>

#include "rasterizer_vertex.hpp"
#include "pixel_data.hpp"
#include "triangle_edge_equation.hpp"

#include "vertex_shader_base.hpp"
#include "fragment_shader_base.hpp"

#include "miscmath.inl.hpp"

namespace flr {

	const int kBlockSize = 8;

	/// Rasterizer mode.
	enum class TriRasterMode {
		kScanline,
		kEdgeEquation,
		kAdaptive
	};

	/// Rasterizer main class.
	class Rasterizer
	{
	private:
		int min_x_;
		int max_x_;
		int min_y_;
		int max_y_;
		std::vector<std::vector<uint32_t>> frame_buffer_;
		std::vector<std::vector<float>> depth_buffer_;

		TriRasterMode tri_raster_mode_;

		void (Rasterizer::* mfp_point_)(const RasterizerVertex& v) const;
		void (Rasterizer::* mfp_line_)(const RasterizerVertex& v0, const RasterizerVertex& v1) const;
		void (Rasterizer::* mfp_tri_)(const RasterizerVertex& v0, const RasterizerVertex& v1, const RasterizerVertex& v2) const;

	public:
		Rasterizer()
		{
			setTriRasterMode(TriRasterMode::kScanline);
			setScissorRect(0, 0, 0, 0);
			setFragmentShader<DummyFragmentShader>();
		}

		void setTriRasterMode(TriRasterMode mode)noexcept
		{
			tri_raster_mode_ = mode;
		}
		void setScissorRect(int x, int y, int width, int height) noexcept
		{
			min_x_ = x;
			min_y_ = y;
			max_x_ = x + width;
			max_y_ = y + height;
		}
		void ResizeBuffer(int width, int height) {
			frame_buffer_.resize(height, std::vector<uint32_t>(width, 0));
			depth_buffer_.resize(height, std::vector<float>(width, std::numeric_limits<float>::infinity()));
		}

		template<typename FragmentShader>
		void setFragmentShader() 
		{
			mfp_point_ = &Rasterizer::DrawPointTemplate<FragmentShader>;
			mfp_line_ = &Rasterizer::DrawLineTemplate<FragmentShader>;
			mfp_tri_ = &Rasterizer::DrawTriangleModeTemplate<FragmentShader>;
			FragmentShader::p_frame_buffer_ = &frame_buffer_;
			FragmentShader::p_depth_buffer_ = &depth_buffer_;
		}

		void DrawPoint(const RasterizerVertex& v) const 
		{
			(this->*mfp_point_)(v);
		}
		void DrawPointList(const RasterizerVertex* vertices, const int* indices, size_t index_count)const
		{
			for (size_t i = 0; i < index_count; ++i) {
				if (indices[i] < 0)
					continue;
				DrawPoint(vertices[indices[i]]);
			}
		}
		void DrawLine(const RasterizerVertex& v0, const RasterizerVertex& v1) const
		{
			(this->*mfp_line_)(v0, v1);
		}
		void DrawLineList(const RasterizerVertex* vertices, const int* indices, size_t index_count) const
		{
			for (size_t i = 0; i < index_count; i += 2) {
				if (indices[i] < 0 || indices[i + 1] < 0)
					continue;
				DrawLine(vertices[indices[i]], vertices[indices[i + 1]]);
			}
		}
		void DrawTriangle(const RasterizerVertex& v0, const RasterizerVertex& v1, const RasterizerVertex& v2)const
		{
			(this->*mfp_tri_)(v0, v1, v2);
		}
		void DrawTriangleList(const RasterizerVertex* vertices, const int* indices, size_t index_count) const
		{
			for (size_t i = 0; i < index_count; i += 3)
			{
				if (indices[i] < 0 || indices[i + 1] < 0 || indices[i + 2] < 0)
					continue;
				DrawTriangle(vertices[indices[i]], vertices[indices[i + 1]], vertices[indices[i + 2]]);
			}
		}

	private:
		bool ScissorTest(float x, float y)const noexcept
		{
			return (x >= min_x_ && x < max_x_ &&
				y >= min_y_ && y < max_y_);
		}
		PixelData CvtVertex2PixelData(const RasterizerVertex& v, int params_count) const
		{
			PixelData pixel;
			pixel.x_ = v.x;
			pixel.y_ = v.y;
			pixel.w_ = v.w;
			pixel.invw_ = 1 / v.w;
			pixel.z_ = v.z;	
			pixel.zdw_ = pixel.z_ / pixel.w_;
			for (int i = 0; i < params_count; ++i) {
				pixel.params_[i] = v.params_[i];
				pixel.params_dw_[i] = v.params_[i] / pixel.w_;
			}
			return pixel;
		}

		using LineStart = RasterizerVertex;
		using LineEnd = RasterizerVertex;
		using CurPoint = RasterizerVertex;
		PixelData LineInterpolate(const LineStart& v0, const LineEnd& v1, const CurPoint& v, float ratio, int params_count) const
		{
			PixelData pixel;
			pixel.x_ = v.x;
			pixel.y_ = v.y;
			pixel.invw_ = Lerp(ratio, 1 / v0.w, 1 / v1.w);
			pixel.w_ = 1 / pixel.invw_;
			pixel.zdw_ = Lerp(ratio, v0.z / v0.w, v1.z / v1.w);
			pixel.z_ = pixel.zdw_ * pixel.w_;
			for (int i = 0; i < params_count; ++i) {
				pixel.params_dw_[i] = Lerp(ratio, v0.params_[i] / v0.w, v1.params_[i] / v1.w);
				pixel.params_[i] = pixel.params_dw_[i] * pixel.w_;
			}
			return pixel;
		}

		template<typename FragmentShader>
		void DrawPointTemplate(const RasterizerVertex& v) const
		{
			if (!ScissorTest(v.x, v.y))
				return;

			PixelData p = CvtVertex2PixelData(v, FragmentShader::params_count_);
			FragmentShader::DrawPixel(p);
		}

		// Bresenham Algorithm
		template<typename FragmentShader>
		void DrawLineTemplate(const RasterizerVertex& v0, const RasterizerVertex& v1)const
		{
			constexpr bool horizontal = true;
			constexpr bool vertical = false;

			int dx = v1.x - v0.x;
			int dy = v1.y - v0.y;
			RasterizerVertex start = v0;
			RasterizerVertex end = v1;
			int absdx = std::abs(dx);
			int absdy = std::abs(dy);
			int steps = 0;
			int pk = 0;
			bool h_v;
			if (absdx > absdy) {
				h_v = horizontal;
				steps = absdx;
				if (absdx < 0) {
					dx = -dx;
					dy = -dy;
					start = v1;
					end = v0;
				}
				pk = 2 * absdy - absdx;
			}
			else {
				h_v = vertical;
				steps = absdy;
				if (dy < 0) {
					dx = -dx;
					dy = -dy;
					start = v1;
					end = v0;
				}
				pk = 2 * absdx - absdy;
			}
			PixelData p = LineInterpolate(start, end, start, 0, FragmentShader::params_count_);
			if (ScissorTest(start.x, start.y))
				FragmentShader::DrawPixel(p);

			auto traveller = start;
			for (int i = 0; i < steps; ++i) {
				if (h_v) 
				{
					traveller.x += 1;
					if (pk > 0) 
					{
						if (dy > 0)
							traveller.y += 1;
						else
							traveller.y -= 1;
						pk += 2 * absdy - 2 * absdx;
					}
					else 
					{
						pk += 2 * absdy;
					}
					PixelData p = LineInterpolate(start, end, traveller, i*1./steps, FragmentShader::params_count_);
					if (ScissorTest(traveller.x, traveller.y))
						FragmentShader::DrawPixel(p);
				}
				else 
				{
					traveller.y += 1;
					if (pk > 0)
					{
						if (dx > 0)
							traveller.x += 1;
						else
							traveller.x -= 1;
						pk += 2 * absdx - 2 * absdy;
					}
					else
					{
						pk += 2 * absdx;
					}
					PixelData p = LineInterpolate(start, end, traveller, i*1./steps, FragmentShader::params_count_);
					if (ScissorTest(traveller.x, traveller.y))
						FragmentShader::DrawPixel(p);
				}
			}
		}

		template<typename FragmentShader>
		void DrawTriangleModeTemplate(const RasterizerVertex& v0, const RasterizerVertex& v1, 
			const RasterizerVertex& v2)const
		{
			switch (tri_raster_mode_)
			{
			case TriRasterMode::kScanline:
				DrawTriangleScanlineTemplate<FragmentShader>(v0, v1, v2);
				break;
			case TriRasterMode::kEdgeEquation:
				DrawTriangleEdgeEquationTemplate<FragmentShader>(v0, v1, v2);
				break;
			case TriRasterMode::kAdaptive:
				DrawTriangleAdaptiveTemplate<FragmentShader>(v0, v1, v2);
				break;
			default:
				throw std::logic_error("wrong triangle rasterization mode!\n");
			}
		}

		template <class FragmentShader>
		void DrawTriangleScanlineTemplate(const RasterizerVertex& v0, const RasterizerVertex& v1, const RasterizerVertex& v2) const
		{
			// Compute triangle equations.
			TriangleEquation eqn(v0, v1, v2, FragmentShader::params_count_);

			// Check if triangle is backfacing.
			if (eqn.area_twifold_ <= 0)
				return;

			const RasterizerVertex* top = &v0;
			const RasterizerVertex* middle = &v1;
			const RasterizerVertex* bottom = &v2;

			//// Sort vertices from top to bottom.
			// x-coordinate from left to right 
			// y-coordinate from top to bottom
			if (top->y < middle->y) std::swap(top, middle);
			if (middle->y < bottom->y) std::swap(middle, bottom);
			if (top->y < middle->y) std::swap(top, middle);

			float dy = (bottom->y - top->y);
			float iy = (middle->y - top->y);

			if (middle->y == top->y)
			{
				const RasterizerVertex* left = middle, * right = top;
				if (left->x > right->x) std::swap(left, right);
				DrawTopFlatTriangle<FragmentShader>(eqn, *left, *right, *bottom);
			}
			else if (middle->y == bottom->y)
			{
				const RasterizerVertex* left = middle, * right = bottom;
				if (left->x > right->x) std::swap(left, right);
				DrawBottomFlatTriangle<FragmentShader>(eqn, *top, *left, *right);
			}
			else
			{
				RasterizerVertex v4;
				v4.y = middle->y;
				v4.x = top->x + (bottom->x - top->x) / dy * iy;

				float invw = 1.f / top->w + (1.f / bottom->w - 1.f / top->w) / dy * iy;
				v4.w = 1.f / invw;
				float zdw = top->z / top->w + (bottom->z / bottom->w - top->z / top->w) / dy * iy;
				v4.z = zdw * v4.w;

				for (int i = 0; i < FragmentShader::params_count_; ++i) {
					float pdw = top->params_[i] / top->w
						+ (bottom->params_[i] / bottom->w - top->params_[i] / top->w) / dy * iy;
					v4.params_[i] = pdw * v4.w;
				}

				const RasterizerVertex* left = middle, * right = &v4;
				if (left->x > right->x) std::swap(left, right);

				DrawBottomFlatTriangle<FragmentShader>(eqn, *top, *left, *right);
				DrawTopFlatTriangle<FragmentShader>(eqn, *left, *right, *bottom);
			}
		}

		template <class FragmentShader>
		void DrawBottomFlatTriangle(const TriangleEquation& tri, const RasterizerVertex& v0, const RasterizerVertex& v1, const RasterizerVertex& v2) const
		{
			float invslope1 = (v1.x - v0.x) / (v1.y - v0.y);
			float invslope2 = (v2.x - v0.x) / (v2.y - v0.y);

			#pragma omp parallel for
			for (int scanline_y = int(v0.y - 0.5f); scanline_y > int(v1.y - 0.5f); --scanline_y)
			{
				float dy = (scanline_y - v0.y) + 0.5f;
				float curx1 = v0.x + invslope1 * dy + 0.5f;
				float curx2 = v0.x + invslope2 * dy + 0.5f;

				// Clip to scissor rect
				int left_x = math::clamp(min_x_, max_x_, (int)curx1);
				int right_x = math::clamp(min_x_, max_x_, (int)curx2);

				FragmentShader::DrawSpan(tri, left_x, scanline_y, right_x);
			}
		}

		template <class FragmentShader>
		void DrawTopFlatTriangle(const TriangleEquation& eqn, const RasterizerVertex& v0, const RasterizerVertex& v1, const RasterizerVertex& v2) const
		{
			float invslope1 = (v2.x - v0.x) / (v2.y - v0.y);
			float invslope2 = (v2.x - v1.x) / (v2.y - v1.y);

			#pragma omp parallel for
			for (int scanline_y = int(v2.y + 0.5f); scanline_y < int(v0.y + 0.5f); ++scanline_y)
			{
				float dy = (scanline_y - v2.y) + 0.5f;
				float curx1 = v2.x + invslope1 * dy + 0.5f;
				float curx2 = v2.x + invslope2 * dy + 0.5f;

				// Clip to scissor rect
				int left_x = math::clamp(min_x_, max_x_, (int)curx1);
				int right_x = math::clamp(min_x_, max_x_, (int)curx2);

				FragmentShader::DrawSpan(eqn, left_x, scanline_y, right_x);
			}
		}

		template <typename FragmentShader>
		void DrawTriangleAdaptiveTemplate(const RasterizerVertex& v0, const RasterizerVertex& v1, const RasterizerVertex& v2) const
		{
			// Compute triangle bounding box.
			float box_min_x = (float)std::min(std::min(v0.x, v1.x), v2.x);
			float box_max_x = (float)std::max(std::max(v0.x, v1.x), v2.x);
			float box_min_y = (float)std::min(std::min(v0.y, v1.y), v2.y);
			float box_max_y = (float)std::max(std::max(v0.y, v1.y), v2.y);

			float orient = (box_max_x - box_min_x) / (box_max_y - box_min_y);

			if (orient > 0.4 && orient < 1.6)
				DrawTriangleEdgeEquationTemplate<FragmentShader>(v0, v1, v2);
			else
				DrawTriangleScanlineTemplate<FragmentShader>(v0, v1, v2);
		}

		template <class FragmentShader>
		void DrawTriangleEdgeEquationTemplate(const RasterizerVertex& v0, const RasterizerVertex& v1, const RasterizerVertex& v2) const
		{
			// Compute triangle equations.
			TriangleEquation tri(v0, v1, v2, FragmentShader::params_count_);

			// Check if triangle is backfacing.
			if (tri.area_twifold_ <= 0)
				return;

			// Compute triangle bounding box.
			int box_min_x = (int)std::min(std::min(v0.x, v1.x), v2.x);
			int box_max_x = (int)std::max(std::max(v0.x, v1.x), v2.x);
			int box_min_y = (int)std::min(std::min(v0.y, v1.y), v2.y);
			int box_max_y = (int)std::max(std::max(v0.y, v1.y), v2.y);

			// Clip to scissor rect.
			box_min_x = math::clamp(min_x_, max_x_, box_min_x);
			box_max_x = math::clamp(min_x_, max_x_, box_max_x);
			box_min_y = math::clamp(min_x_, max_x_, box_min_y);
			box_max_y = math::clamp(min_x_, max_x_, box_max_y);

			// Round to block grid.
			box_min_x = box_min_x & ~(kBlockSize - 1);
			box_max_x = box_max_x & ~(kBlockSize - 1);
			box_min_y = box_min_y & ~(kBlockSize - 1);
			box_max_y = box_max_y & ~(kBlockSize - 1);

			float s = kBlockSize - 1;

			int steps_x = (box_max_x - box_min_x) / kBlockSize + 1;
			int steps_y = (box_max_y - box_min_y) / kBlockSize + 1;

			#pragma omp parallel for
			for (int i = 0; i < steps_x * steps_y; ++i)
			{
				int sx = i % steps_x;
				int sy = i / steps_x;

				// Add 0.5 to sample at pixel centers.
				int x = box_min_x + sx * kBlockSize;
				int y = box_min_y + sy * kBlockSize;

				float xf = x + 0.5f;
				float yf = y + 0.5f;

				// Test if block is inside or outside triangle or touches it.
				TriEdgeEvalData eval_00; 
				eval_00.Initialize(tri, xf, yf);
				bool is_in_triangle_00 = eval_00.IsInTriangle();

				TriEdgeEvalData eval_01 = eval_00;
				eval_01.StepY(s);
				bool is_in_triangle_01 = eval_01.IsInTriangle();

				TriEdgeEvalData eval_10 = eval_00; 
				eval_10.StepX(s);
				bool is_in_triangle_10 = eval_10.IsInTriangle();

				TriEdgeEvalData eval_11 = eval_01; 
				eval_11.StepX(s);
				bool is_in_triangle_11 = eval_11.IsInTriangle();

				int result = is_in_triangle_00 + is_in_triangle_01 +
					is_in_triangle_10 + is_in_triangle_11;

				if (result == 4)
				{
					// Fully Covered.
					FragmentShader::template DrawBlockInTriangle<false>(tri, x, y);
				}
				else
				{
					// Partially Covered or Potentially all out.
					FragmentShader::template DrawBlockInTriangle<true>(tri, x, y);
				}
			}
		}
	};

} // end namespace fl


#endif // !__RASTERIZER_HPP__
