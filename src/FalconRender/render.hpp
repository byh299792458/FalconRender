#ifndef __RENDER_HPP__
#define __RENDER_HPP__

#include <vector>
#include "rasterizer.hpp"
#include "vertex_shader_base.hpp"
#include "fragment_shader_base.hpp"

namespace flr {

	enum class Primitive {
		Point,
		Line,
		Triangle
	};

	enum class CullMode {
		kNone,
		kCCW,		//counter-clockwise
		kCW			//clockwise
	};

	class Render {
	public:
		/// Constructor.
		Render();

		void setTriRasterMode(TriRasterMode mode)noexcept{
			rasterizer_.setTriRasterMode(mode);
		}

		void setScissorRect(int x, int y, int width, int height) noexcept{
			rasterizer_.setScissorRect(x, y, width, height);
		}

		/// Set the viewport.
		/** Top-Left is (0, 0) */
		void setViewport(int x, int y, int width, int height);

		/// Set the depth range.
		/** Default is (0, 1) */
		void setDepthRange(float n, float f);

		/// Set the cull mode.
		/** Default is CullMode::CW to cull clockwise triangles. */
		void setCullMode(CullMode mode);

		/// Set the vertex shader.
		template <class VertexShader>
		void setVertexShader(void)
		{
			static_assert(VertexShader::kAttribCount_ <= kMaxVertexAttribs);
			attrib_count_ = VertexShader::kAttribCount_;
			fp_process_vertex_ = VertexShader::ProcessVertex;
		}


		template<class FragmentShader>
		void setFragmentShader() 
		{
			rasterizer_.setFragmentShader<FragmentShader>();
		}

		/// Set a vertex attrib pointer.
		void setVertexAttribPointer(int index, int stride, const void* buffer);

		/// Draw a number of points, lines or triangles.
		void DrawElements(Primitive mode, size_t count, int* indices) ;

	private:
		enum ClipMask {
			kPosX = 0x01,
			kNegX = 0x02,
			kPosY = 0x04,
			kNegY = 0x08,
			kPosZ = 0x10,
			kNegZ = 0x20
		};

		int getClipMask(VertexShaderOutput& v);

		const void* AttribPointer(int attribIndex, int elementIndex) ;
		void ProcessVertex(VertexShaderInput in, VertexShaderOutput* out) ;
		void InitVertexInput(VertexShaderInput in, int index) ;

		void ClipPoints();
		void ClipLines();
		void ClipTriangles();

		void ClipPrimitives(Primitive mode) ;
		void ProcessPrimitives(Primitive mode) ;
		int PrimitiveCount(Primitive mode) ;

		void DrawPrimitives(Primitive mode) ;
		void CullTriangles();
		void TransformVertices();

	private:
		struct {
			int x, y, width, height;
			float scale_x, scale_y, trans_x, trans_y;
		} viewport_;

		struct {
			float n, f;
		} depthrange_;

		CullMode cull_mode_;
		Rasterizer rasterizer_;

		void (*fp_process_vertex_)(VertexShaderInput, VertexShaderOutput*);
		int attrib_count_;

		struct VertexAttribute {
			const void* buffer;
			int stride;
		} vertex_attributes_[kMaxVertexAttribs];

		std::vector<VertexShaderOutput> output_vertices_;
		std::vector<int> output_indices_;
		std::vector<int> clip_mask_per_vertex_;
	};

} // end namespace flr

#endif // !__RENDER_HPP__
