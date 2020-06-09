#ifndef __EDGE_EQUATION_HPP__
#define __EDGE_EQUATION_HPP__

#include <array>
#include "rasterizer_vertex.hpp"

namespace flr {

	class EdgeEquation 
	{
	public:

		float a_;
		float b_;
		float c_;
		bool tie_;

		void 
		Initialize(const RasterizerVertex& v0,
			const RasterizerVertex& v1)
		{
			a_ = v0.y - v1.y;
			b_ = v1.x - v0.x;
			c_ = -(a_ * (v0.x + v1.x) + b_ * (v0.y + v1.y)) / 2;
			tie_ = a_ != 0 ? a_ > 0:b_ < 0;
		}
		float Evaluate(float x, float y) const
		{
			return a_ * x + b_ * y + c_;
		}
		float StepX(float value, float step_size = 1.f)const
		{
			return value + a_ * step_size;
		}
		float StepY(float value, float step_size = 1.f) const
		{
			return value + b_ * step_size;
		}

		bool Test(float x, float y) const
		{
			return Test(Evaluate(x, y));
		}
		bool Test(float v)const
		{
			return (v > 0 || (v == 0 && tie_));
		}

	};
	
	class ParameterEquation {
	public:
		void
			Initialize(float param0,
				float param1,
				float param2,
				const EdgeEquation& e0,
				const EdgeEquation& e1,
				const EdgeEquation& e2,
				float factor)
		{
			a_ = factor * (param0 * e0.a_ + param1 * e1.a_ + param2 * e2.a_);
			b_ = factor * (param0 * e0.b_ + param1 * e1.b_ + param2 * e2.b_);
			c_ = factor * (param0 * e0.c_ + param1 * e1.c_ + param2 * e2.c_);
		}
		float Evaluate(float x, float y) const noexcept
		{
			return a_ * x + b_ * y + c_;
		}
		float StepX(float value, float step_size = 1.f) const
		{
			return value + a_ * step_size;
		}
		float StepY(float value, float step_size = 1.f) const
		{
			return value + b_ * step_size;
		}
	private:
		float a_;
		float b_;
		float c_;
	};


	class TriangleEquation {
	public:
		float area_twifold_;
		std::array<EdgeEquation, 3> edge_equations_;

		ParameterEquation zdw_;
		ParameterEquation invw_;
		ParameterEquation params_dw_[kMaxParamVarsCount];

		TriangleEquation(const RasterizerVertex& v0,
			const RasterizerVertex& v1,
			const RasterizerVertex& v2,
			int params_count)
		{
			edge_equations_[0].Initialize(v1, v2);
			edge_equations_[1].Initialize(v2, v0);
			edge_equations_[2].Initialize(v0, v1);
			area_twifold_ = edge_equations_[0].c_ + edge_equations_[1].c_ + edge_equations_[2].c_;

			if (area_twifold_ <= 0)
				return;

			float factor = 1.f / area_twifold_;

			float invw0 = 1.f / v0.w;
			float invw1 = 1.f / v1.w;
			float invw2 = 1.f / v2.w;

			invw_.Initialize(invw0, invw1, invw2, edge_equations_[0], edge_equations_[1], edge_equations_[2], factor);
			zdw_.Initialize(v0.z, v1.z, v2.z, edge_equations_[0], edge_equations_[1], edge_equations_[2], factor);
			for (int i = 0; i < params_count; ++i) {
				params_dw_[i].Initialize(v0.params_[i] * invw0, v1.params_[i] * invw1, v2.params_[i] * invw2,
					edge_equations_[0], edge_equations_[1], edge_equations_[2], factor);
			}
		}
	};


	class TriEdgeEvalData {
	public:
		std::array<float, 3> evaluates_;
		const TriangleEquation* tri_;

		void Initialize(const TriangleEquation& tri, float x, float y)
		{
			tri_ = &tri;
			for (int i = 0; i < evaluates_.size(); ++i)
			{
				evaluates_[i] = tri_->edge_equations_[i].Evaluate(x, y);
			}
		}
		void StepX(float step_size = 1.f)
		{
			for (int i = 0; i < evaluates_.size(); ++i)
			{
				evaluates_[i] = tri_->edge_equations_[i].StepX(evaluates_[i], step_size);
			}
		}
		void StepY(float step_size = 1.f)
		{
			for (int i = 0; i < evaluates_.size(); ++i)
			{
				evaluates_[i] = tri_->edge_equations_[i].StepY(evaluates_[i], step_size);
			}
		}
		bool IsInTriangle(void)
		{
			return tri_->edge_equations_[0].Test(evaluates_[0]) &&
				tri_->edge_equations_[1].Test(evaluates_[1]) &&
				tri_->edge_equations_[2].Test(evaluates_[2]);
		}
	};
}

#endif // !__EDGE_EQUATION_HPP__
