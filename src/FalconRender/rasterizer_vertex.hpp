#ifndef __VERTEX_HPP__
#define __VERTEX_HPP__

namespace flr {

	constexpr int kMaxParamVarsCount = 16;

	struct RasterizerVertex
	{
		float x;
		float y;
		float z;
		float w;
		float params_[kMaxParamVarsCount];
	};

	template<typename T> 
	inline T Lerp(double t, const T& y0, const T& y1) {
		return (1 - t) * y0 + t * y1;
	}

	template<>
	inline RasterizerVertex Lerp<RasterizerVertex>(double t, const RasterizerVertex& v0, const RasterizerVertex& v1)
	{
		RasterizerVertex ret;
		ret.x = (1 - t) * v0.x + t * v1.x;
		ret.y = (1 - t) * v0.y + t * v1.y;
		ret.z = (1 - t) * v0.z + t * v1.z;
		ret.w = (1 - t) * v0.w + t * v1.w;
		for (int i = 0; i < kMaxParamVarsCount; ++i) {
			ret.params_[i] = (1 - t) * v0.params_[i] + t * v1.params_[i];
		}
		return ret;
	}

	inline float Plane(const RasterizerVertex& v, float A, float B, float C, float D) {
		return A * v.x + B * v.y + C * v.z + D * v.w;
	}

}
#endif // !__VERTEX_HPP__
