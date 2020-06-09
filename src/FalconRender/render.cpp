#include <cassert>
#include "line_clipper.hpp"
#include "triangle_clipper.hpp"
#include "render.hpp"
#include "vertex_cache.hpp"

namespace flr {

	Render::Render()
	{
		setCullMode(CullMode::kCW);
		setDepthRange(1.0f, 100.0f);
		setVertexShader<DummyVertexShader>();
	}

	void Render::setViewport(int x, int y, int width, int height)
	{
		viewport_.x = x;
		viewport_.y = y;
		viewport_.width = width;
		viewport_.height = height;

		viewport_.scale_x = width / 2.f;
		viewport_.scale_y = height / 2.f;
		viewport_.trans_x = x + width / 2.f;
		viewport_.trans_y = y + height / 2.f;

		rasterizer_.ResizeBuffer(width, height);
	}

	void Render::setDepthRange(float n, float f)
	{
		depthrange_.n = n;
		depthrange_.f = f;
	}

	/// Set the cull mode.
	/** Default is CullMode::CW to cull clockwise triangles. */
	void Render::setCullMode(CullMode mode)
	{
		cull_mode_ = mode;
	}

	/// Set a vertex attrib pointer.
	void Render::setVertexAttribPointer(int attrib_idx, int stride, const void* buffer)
	{
		assert(attrib_idx < kMaxVertexAttribs);
		vertex_attributes_[attrib_idx].stride = stride;
		vertex_attributes_[attrib_idx].buffer = buffer;
	}

	void Render::DrawElements(Primitive mode, size_t count, int* indices)
	{
		output_vertices_.clear();
		output_indices_.clear();

		VertexCache cache;

		for (size_t i = 0; i < count; i++)
		{
			int elem_idx = indices[i];
			int vertex_idx = cache.Lookup(elem_idx);

			if (vertex_idx != -1){
				output_indices_.push_back(vertex_idx);
			}
			else
			{
				VertexShaderInput user_vertex_shader_inputs;
				InitVertexInput(user_vertex_shader_inputs, elem_idx);

				vertex_idx = static_cast<int>(output_vertices_.size());
				output_indices_.push_back(vertex_idx);
				output_vertices_.resize(output_vertices_.size() + 1);
				VertexShaderOutput& vertex_shader_output = output_vertices_.back();

				ProcessVertex(user_vertex_shader_inputs, &vertex_shader_output);

				cache.set(elem_idx, vertex_idx);
			}

			if (PrimitiveCount(mode) >= 1024)
			{
				ProcessPrimitives(mode);
				output_vertices_.clear();
				output_indices_.clear();
				cache.Clear();
			}
		}
		ProcessPrimitives(mode);
	}

	void Render::InitVertexInput(VertexShaderInput in, int elem_idx)
	{
		for (int i = 0; i < attrib_count_; ++i)
			in[i] = AttribPointer(i, elem_idx);
	}
	const void* Render::AttribPointer(int attrib_idx, int element_idx)
	{
		const VertexAttribute& attrib = vertex_attributes_[attrib_idx];
		return static_cast<const void*>(
				static_cast<const char*>(attrib.buffer) + attrib.stride * element_idx
			);
	}
	void Render::ProcessVertex(VertexShaderInput in, VertexShaderOutput* out)
	{
		(*fp_process_vertex_)(in, out);
	}

	void Render::ProcessPrimitives(Primitive mode)
	{
		ClipPrimitives(mode);
		TransformVertices();
		DrawPrimitives(mode);
	}

	void Render::ClipPrimitives(Primitive mode)
	{
		switch (mode)
		{
		case Primitive::Point:
			ClipPoints();
			break;
		case Primitive::Line:
			ClipLines();
			break;
		case Primitive::Triangle:
			ClipTriangles();
			break;
		}
	}

	int Render::getClipMask(VertexShaderOutput& v)
	{
		int mask = 0;
		if (v.w - v.x < 0) mask |= ClipMask::kPosX;
		if (v.x + v.w < 0) mask |= ClipMask::kNegX;
		if (v.w - v.y < 0) mask |= ClipMask::kPosY;
		if (v.y + v.w < 0) mask |= ClipMask::kNegY;
		if (v.w - v.z < 0) mask |= ClipMask::kPosZ;
		if (v.z + v.w < 0) mask |= ClipMask::kNegZ;
		return mask;
	}

	void Render::ClipPoints()
	{
		clip_mask_per_vertex_.clear();
		clip_mask_per_vertex_.resize(output_vertices_.size());

		for (int i = 0; i < output_vertices_.size(); ++i) {
			clip_mask_per_vertex_[i] = getClipMask(output_vertices_[i]);
		}

		for (int i = 0; i < output_indices_.size(); ++i) {
			if (clip_mask_per_vertex_[output_indices_[i]])
				output_indices_[i] = -1;
		}
	}

	void Render::ClipLines()
	{
		clip_mask_per_vertex_.clear();
		clip_mask_per_vertex_.resize(output_vertices_.size());

		for (int i = 0; i < output_vertices_.size(); ++i)
			clip_mask_per_vertex_[i] = getClipMask(output_vertices_[i]);

		for (int i = 0; i < output_indices_.size(); i += 2)
		{
			int idx0 = output_indices_[i];
			int idx1 = output_indices_[i + 1];

			VertexShaderOutput& v0 = output_vertices_[idx0];
			VertexShaderOutput& v1 = output_vertices_[idx1];

			int clip_mask = clip_mask_per_vertex_[idx0] |
				clip_mask_per_vertex_[idx1];
			
			if (0 == clip_mask)
				continue;
			
			LineClipper clipper(v0, v1);
			if (clip_mask & ClipMask::kPosX) clipper.ClipToPlane(-1,  0,  0,  1);
			if (clip_mask & ClipMask::kNegX) clipper.ClipToPlane( 1,  0,  0,  1);
			if (clip_mask & ClipMask::kPosY) clipper.ClipToPlane( 0, -1,  0,  1);
			if (clip_mask & ClipMask::kNegY) clipper.ClipToPlane( 0,  1,  0,  1);
			if (clip_mask & ClipMask::kPosZ) clipper.ClipToPlane( 0,  0, -1,  1);
			if (clip_mask & ClipMask::kNegZ) clipper.ClipToPlane( 0,  0,  1,  1);

			if (clipper.is_fully_clipped_) {
				output_indices_[i] = -1;
				output_indices_[i + 1] = -1;
				continue;
			}

			if (clip_mask_per_vertex_[idx0]) {
				auto&& vertex = Lerp(clipper.t0_, v0, v1);
				output_vertices_.push_back(vertex);
				output_indices_[i] = output_vertices_.size() - 1;
			}

			if (clip_mask_per_vertex_[idx1]) {
				auto&& vertex = Lerp(clipper.t1_, v0, v1);
				output_vertices_.push_back(vertex);
				output_indices_[i + 1] = output_vertices_.size() - 1;
			}
		}
	}

	void Render::ClipTriangles()
	{
		clip_mask_per_vertex_.clear();
		clip_mask_per_vertex_.resize(output_vertices_.size());

		for (int i = 0; i < output_vertices_.size(); ++i)
			clip_mask_per_vertex_[i] = getClipMask(output_vertices_[i]);

		int n = output_indices_.size();
		for (int i = 0; i < n; i += 3)
		{
			int idx0 = output_indices_[i];
			int idx1 = output_indices_[i + 1];
			int idx2 = output_indices_[i + 2];

			int clip_mask = clip_mask_per_vertex_[idx0] |
				clip_mask_per_vertex_[idx1] | clip_mask_per_vertex_[idx2];

			if (0 == clip_mask)
				continue;

			TriangleClipper triangle(output_vertices_, idx0, idx1, idx2);
			if (clip_mask & ClipMask::kPosX) triangle.ClipToPlane(-1, 0, 0, 1);
			if (clip_mask & ClipMask::kNegX) triangle.ClipToPlane(1, 0, 0, 1);
			if (clip_mask & ClipMask::kPosY) triangle.ClipToPlane(0, -1, 0, 1);
			if (clip_mask & ClipMask::kNegY) triangle.ClipToPlane(0, 1, 0, 1);
			if (clip_mask & ClipMask::kPosZ) triangle.ClipToPlane(0, 0, -1, 1);
			if (clip_mask & ClipMask::kNegZ) triangle.ClipToPlane(0, 0, 1, 1);

			if (triangle.IsFullyClipped()) {
				output_indices_[i] = -1;
				output_indices_[i + 1] = -1;
				output_indices_[i + 2] = -1;
				continue;
			}

			output_indices_[i] = triangle.tri_idx[0];
			output_indices_[i + 1] = triangle.tri_idx[1];
			output_indices_[i + 2] = triangle.tri_idx[2];
			for (int j = 3; j < triangle.tri_idx.size(); ++j) {
				output_indices_.push_back(triangle.tri_idx[0]);
				output_indices_.push_back(triangle.tri_idx[j - 1]);
				output_indices_.push_back(triangle.tri_idx[j]);
			}
		}
	}

	int Render::PrimitiveCount(Primitive mode)
	{
		switch (mode)
		{
		case Primitive::Point: 
			return output_indices_.size();
		case Primitive::Line:
			return output_indices_.size() / 2;
		case Primitive::Triangle:
			return output_indices_.size() / 3;
		}
	}

	void Render::DrawPrimitives(Primitive mode)
	{
		switch (mode)
		{
		case Primitive::Triangle:
			CullTriangles();
			rasterizer_.DrawTriangleList(&output_vertices_[0], &output_indices_[0], output_indices_.size());
			break;
		case Primitive::Line:
			rasterizer_.DrawLineList(&output_vertices_[0], &output_indices_[0], output_indices_.size());
			break;
		case Primitive::Point:
			rasterizer_.DrawPointList(&output_vertices_[0], &output_indices_[0], output_indices_.size());
			break;
		}
	}

	void Render::CullTriangles()
	{
		for (size_t i = 0; i < output_indices_.size(); i += 3)
		{
			if (output_indices_[i] == -1)
				continue;

			VertexShaderOutput& v0 = output_vertices_[output_indices_[i]];
			VertexShaderOutput& v1 = output_vertices_[output_indices_[i + 1]];
			VertexShaderOutput& v2 = output_vertices_[output_indices_[i + 2]];

			// z-coordinate of (vec v1v0) cross (vec v1v2)
			float facing = (v0.x - v1.x) * (v2.y - v1.y) - (v2.x - v1.x) * (v0.y - v1.y);

			if (facing > 0)
			{
				if (cull_mode_ == CullMode::kCW)
					output_indices_[i] = output_indices_[i + 1] = output_indices_[i + 2] = -1;
			}
			else
			{
				if (cull_mode_ == CullMode::kCCW)
					output_indices_[i] = output_indices_[i + 1] = output_indices_[i + 2] = -1;
			}
		}

	}

	void Render::TransformVertices()
	{
		std::vector<bool> processed(output_vertices_.size(), false);

		for (size_t i = 0; i < output_indices_.size(); i++)
		{
			int index = output_indices_[i];

			if (index == -1)
				continue;

			if (processed[index])
				continue;

			VertexShaderOutput& out_vex = output_vertices_[index];

			// Perspective divide
			float invw = 1.0f / out_vex.w;
			out_vex.x *= invw;
			out_vex.y *= invw;
			out_vex.z *= invw;

			// Viewport transform
			out_vex.x = (viewport_.scale_x * out_vex.x + viewport_.trans_x);
			out_vex.y = (viewport_.scale_y * out_vex.y + viewport_.trans_y);
			out_vex.z = 0.5f * (depthrange_.f - depthrange_.n) * out_vex.z + 0.5f * (depthrange_.n + depthrange_.f);

			processed[index] = true;
		}
	}

} // end namespace flr
