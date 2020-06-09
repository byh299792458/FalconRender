#include <algorithm>
#include <cstdlib>
#include <cmath>
#include <iostream>
#include <numeric>

#include "Eigen/Eigen"
#include "render.hpp"
#include "SDL.h"
#include "SDL_image.h"
#include "timer.hpp"
#include "model.hpp"
#include "mvp_matrices.hpp"

using namespace flr;

struct VertexData {
	float x, y, z;
	float r, g, b;
} vdata[3] = {
	{       0.f,      -0.5f, 0.f,        1.f, 0.f, 0.f},
	{ 160/320.f,  -40/240.f, 0.f,        1.f, 0.f, 0.f},
	{-200/320.f,   0.3f, 0.f,        1.f, 0.f, 0.f},
};
int idata[3] = { 0,1,2 };

class VertexShader :public VertexShaderBase<VertexShader> {
public:
	static const int kAttribCount_ = 1;

	static Eigen::Matrix4f mvp;

	static void ProcessVertex(VertexShaderInput in, VertexShaderOutput* out)
	{
		const VertexData* data = static_cast<const VertexData*>(in[0]); //0÷∏0∫≈ Ù–‘

		vec4f position;
		position << data->x, data->y, data->z, 1;
		position = mvp* position;

		//out->x = data->x;
		//out->y = data->y;
		//out->z = data->z;
		//out->w = 1.f;
		//out->w = 0.2;
		out->x = position.x();
		out->y = position.y();
		out->z = position.z();
		out->w = position.w();

		if (std::abs(out->w -3)>0.001) 
		{
			out->params_[0] = data->g;
			out->params_[1] = data->r;
			out->params_[2] = data->b;
		}
		else {
			out->params_[0] = data->r;
			out->params_[1] = data->g;
			out->params_[2] = data->b;
		}
	}
};
Eigen::Matrix4f VertexShader::mvp = Eigen::Matrix4f::Identity();


class FragmentShader :public FragmentShaderBase<FragmentShader> {
public:
	static SDL_Surface* surface;
	static const int params_count_ = 3;

	static void SetBackGround(float r, float g, float b) {
		auto& frame_buffer = *p_frame_buffer_;
		auto& depth_buffer = *p_depth_buffer_;
		uint32_t color = ((uint32_t)(r * 255) << 16) | ((uint32_t)(g * 255) << 8) | ((uint32_t)(b * 255));
		std::for_each(frame_buffer.begin(), frame_buffer.end(),
			[&](auto& v) mutable {
				std::fill(v.begin(), v.end(), color);
			});
		std::for_each(depth_buffer.begin(), depth_buffer.end(),
			[&](auto& v)mutable {
				std::fill(v.begin(), v.end(), std::numeric_limits<float>::infinity());
			});
	}

	static void SwapBuffer(void) {
		auto& frame_buffer = *p_frame_buffer_;
		for (int i = 0; i < frame_buffer.size(); ++i) {
			for (int j = 0; j < frame_buffer[0].size(); ++j) {
				Uint32* screenBuffer = (Uint32*)((Uint8*)surface->pixels
					+ (int)i * surface->pitch + (int)j * 4);
				*screenBuffer = frame_buffer[i][j];
			}
		}
	}

	static void DrawPixel(const PixelData& p)
	{
		auto& frame_buffer = *p_frame_buffer_;
		auto& depth_buffer = *p_depth_buffer_;
		int height = frame_buffer.size();

		auto& depth = depth_buffer[height - p.y_ - 1][p.x_];
		if (p.z_ < depth_buffer[height - p.y_ - 1][p.x_])
		{
			frame_buffer[height - p.y_ - 1][p.x_] =
				((int)(255 * p.params_[0]) << 16) +
				((int)(255 * p.params_[1]) << 8) +
				((int)255 * p.params_[2]);
			depth_buffer[height - p.y_ - 1][p.x_] = p.z_;
		}
	}
};
SDL_Surface* FragmentShader::surface;


int main(int argc, char* argv[])
{
	Timer t;
	SDL_Init(SDL_INIT_VIDEO);

	SDL_Window* window = SDL_CreateWindow(
		"RasterizerTest",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640,
		480,
		0
	);

	SDL_Surface* screen = SDL_GetWindowSurface(window);

	srand(1234);
	for (int i = 0; i < 10000; ++i)
	{
		Uint32 x = rand() % 640;
		Uint32 y = rand() % 480;
		Uint8 r = rand() % 256;
		Uint8 g = rand() % 256;
		Uint8 b = rand() % 256;
	}

	flr::Render render;

	render.setVertexShader<VertexShader>();
	render.setFragmentShader<FragmentShader>();
	FragmentShader::surface = screen;
	FragmentShader::SetBackGround(0.3, 0.3, 0.3);

	//render.setTriRasterMode(TriRasterMode::kEdgeEquation);
	render.setTriRasterMode(TriRasterMode::kScanline);
	render.setViewport(0, 0, 640, 480);
	render.setDepthRange(1.f, 100.f);
	render.setScissorRect(0, 0, 640, 480);
	render.setVertexAttribPointer(0, sizeof(VertexData), &(vdata[0]));

	auto projection = Projection(45, 1, 1, 100);
	auto view = LookAt(vec3f(0, 0, 3), vec3f(0, 0, 0), vec3f(0, 1, 0));
	VertexShader::mvp = projection * view;

	render.DrawElements(flr::Primitive::Triangle, 3, &(idata[0]));
	FragmentShader::SwapBuffer();
	SDL_UpdateWindowSurface(window);

	Eigen::Matrix4f model_matrix;
	model_matrix <<
		std::cos(flr::math::toRadians(80.)), 0, -std::sin(flr::math::toRadians(80.)), 0,
		0, 1, 0, 0,
		std::sin(flr::math::toRadians(80.)), 0, std::cos(flr::math::toRadians(80.)), 0,
		0, 0, 0, 1;
	VertexShader::mvp *= model_matrix;
	render.DrawElements(flr::Primitive::Triangle, 3, &(idata[0]));
	FragmentShader::SwapBuffer();
	SDL_UpdateWindowSurface(window);

	SDL_Event e;
	while (SDL_WaitEvent(&e) && e.type != SDL_QUIT && e.key.keysym.sym != SDLK_ESCAPE);

	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}


