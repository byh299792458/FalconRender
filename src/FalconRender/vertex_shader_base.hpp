#ifndef __VERTEXSHADERBASE_HPP__
#define __VERTEXSHADERBASE_HPP__

#include "rasterizer.hpp"

namespace flr {

	constexpr int kMaxVertexAttribs = 8;

	typedef RasterizerVertex VertexShaderOutput;
	typedef const void* VertexShaderInput[kMaxVertexAttribs];

	template<typename Derived>
	class VertexShaderBase {
	public:
		static const int kAttribCount_ = 0;

		static void ProcessVertex(VertexShaderInput in, VertexShaderOutput* out)
		{
		}
	};

	class DummyVertexShader:public VertexShaderBase<DummyVertexShader>{};

} // end namespace flr

#endif // !__VERTEXSHADERBASE_HPP__
