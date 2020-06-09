#ifndef __VERTEXCACHE_HPP__
#define __VERTEXCACHE_HPP__

namespace flr {

	class VertexCache {
	private:
		static const int kVertexCacheSize = 16;

		int in_idx_buffer_[kVertexCacheSize];
		int out_idx_buffer_[kVertexCacheSize];

	public:
		VertexCache()
		{
			Clear();
		}

		void Clear()
		{
			for (size_t i = 0; i < kVertexCacheSize; i++)
				in_idx_buffer_[i] = -1;
		}

		void set(int in_idx, int out_idx)
		{
			int cache_idx = in_idx % kVertexCacheSize;
			in_idx_buffer_[cache_idx] = in_idx;
			out_idx_buffer_[cache_idx] = out_idx;
		}

		int Lookup(int in_idx) const
		{
			int cache_idx = in_idx % kVertexCacheSize;
			if (in_idx_buffer_[cache_idx] == in_idx)
				return out_idx_buffer_[cache_idx];
			else
				return -1;
		}

	};

} // end namespace flr

#endif // !__VERTEXCACHE_HPP__
