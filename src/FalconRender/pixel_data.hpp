#ifndef __PIXEL_DATA_HPP__
#define __PIXEL_DATA_HPP__

#include "triangle_edge_equation.hpp"

namespace flr {

	class PixelData {
	public:
		int x_;
		int y_;

		float z_;
		float zdw_;
		float w_;
		float invw_;

		float params_[kMaxParamVarsCount];
		float params_dw_[kMaxParamVarsCount];

		const TriangleEquation* tri_{ nullptr };

		void Initialize(const TriangleEquation& tri, float x, float y, int params_count)
		{
			tri_ = &tri;
			invw_ = tri.invw_.Evaluate(x, y);
			w_ = 1 / invw_;
			zdw_ = tri.zdw_.Evaluate(x, y);
			z_ = zdw_ * w_;
			for (int i = 0; i < params_count; ++i) {
				params_dw_[i] = tri.params_dw_[i].Evaluate(x, y);
				params_[i] = params_dw_[i] * w_;
			}
		}
		void StepX(int params_count, float step_size = 1.f)
		{
			invw_ = tri_->invw_.StepX(invw_, step_size);
			w_ = 1 / invw_;
			zdw_ = tri_->zdw_.StepX(zdw_, step_size);
			z_ = zdw_ * w_;
			for (int i = 0; i < params_count; ++i)
			{
				params_dw_[i] = tri_->params_dw_[i].StepX(params_dw_[i], step_size);
				params_[i] = params_dw_[i] * w_;
			}
		}
		void StepY(int params_count, float step_size = 1.f)
		{
			invw_ = tri_->invw_.StepY(invw_, step_size);
			w_ = 1 / invw_;
			zdw_ = tri_->zdw_.StepY(zdw_, step_size);
			z_ = zdw_ * w_;
			for (int i = 0; i < params_count; ++i) {
				params_dw_[i] = tri_->params_dw_[i].StepY(params_dw_[i], step_size);
				params_[i] = params_dw_[i] * w_;
			}
		}

	};

}



#endif // !__PIXEL_DATA_HPP__
