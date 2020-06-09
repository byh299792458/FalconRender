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

class VertexShader :public VertexShaderBase<VertexShader> {
public:	
	static const int kAttribCount_ = 1;

	static Eigen::Matrix4f mvp;

	static void ProcessVertex(VertexShaderInput in, VertexShaderOutput* out)
	{
		const Vertex* data = static_cast<const Vertex*>(in[0]);//0指0号属性

		vec4f position = mvp * data->position;

		out->x = position.x();
		out->y = position.y();
		out->z = position.y();
		out->w = position.w();
		out->params_[0] = data->texcoords[0].x();
		out->params_[1] = data->texcoords[0].y();
	}
};
Eigen::Matrix4f VertexShader::mvp = Eigen::Matrix4f::Identity();


class FragmentShader :public FragmentShaderBase<FragmentShader>{
public:
	using Base = FragmentShaderBase<FragmentShader>;
	static SDL_Surface* surface;
	static SDL_Surface* texture;
	static const int params_count_ = 2;

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
		int tx = std::max(0, int(p.params_[0] * 255)) % 255;
		int ty = std::max(0, int(p.params_[1] * 255)) % 255;

		Uint32* texBuffer = (Uint32*)((Uint8*)texture->pixels + (int)ty * texture->pitch + (int)tx * 4);
		auto& depth = depth_buffer[height - p.y_ - 1][p.x_];
		if (p.z_ < depth_buffer[height - p.y_ - 1][p.x_]) 
		{
			frame_buffer[height - p.y_ - 1][p.x_] = *texBuffer;
			depth_buffer[height - p.y_ - 1][p.x_] = p.z_;
		}
	}
};
SDL_Surface* FragmentShader::surface;
SDL_Surface* FragmentShader::texture;




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
	SDL_Surface* temp_texture_1 = IMG_Load("../../../data/box.png");
	SDL_Surface* temp_texture_2 = IMG_Load("../../../data/box2.jpg");
	SDL_Surface* texture1 = SDL_ConvertSurface(temp_texture_1, screen->format, 0);
	SDL_Surface* texture2 = SDL_ConvertSurface(temp_texture_2, screen->format, 0);
	SDL_FreeSurface(temp_texture_1);
	SDL_FreeSurface(temp_texture_2);

	srand(1234);
	for (int i = 0; i < 10000; ++i)
	{
		Uint32 x = rand() % 640;
		Uint32 y = rand() % 480;
		Uint8 r = rand() % 256;
		Uint8 g = rand() % 256;
		Uint8 b = rand() % 256;
	}

	Model model;
	model.LoadModel("../../../data/box.obj");
	flr::Render render;

	//render.setTriRasterMode(flr::TriRasterMode::kEdgeEquation);
	render.setTriRasterMode(flr::TriRasterMode::kScanline);
	render.setVertexShader<VertexShader>();
	render.setFragmentShader<FragmentShader>();
	FragmentShader::surface = screen;

	render.setViewport(0, 0, 640, 480);
	render.setDepthRange(1.f, 100.f);
	render.setScissorRect(0, 0, 640, 480);
	render.setVertexAttribPointer(0, sizeof(flr::Vertex), &(model.vertex_buffer_obj_[0]));
	auto projection = Projection(45, 640./480., 1, 100);

	SDL_Event e;
	int counter = 0;
	while (1) {
		counter++;
		std::cout << "frame " << counter << std::endl;
		FragmentShader::SetBackGround(0.3, 0.3, 0.5);

		// 画第一个方盒
		FragmentShader::texture = texture1;
		auto view = LookAt(vec3f(15 * std::sin(counter / 10), 3, 15 * std::cos(counter / 10)),
				vec3f(0, 0, 0), vec3f(0, 1, 0));
		//auto view = LookAt(vec3f(15 * std::sin(0), 0, 15 * std::cos(0)),
		//		vec3f(0, 0, 0), vec3f(0, 1, 0));
		VertexShader::mvp = projection * view;

		render.DrawElements(flr::Primitive::Triangle,
			model.element_buffer_obj_.size(), &(model.element_buffer_obj_[0]));

		// 画第二个方盒
		FragmentShader::texture = texture2;
		Eigen::Matrix4f model_matirx;
		model_matirx << 
			std::cos(flr::math::toRadians(45.)), 0, -std::sin(flr::math::toRadians(45.)), 0,
			0, 1, 0, 0,
			std::sin(flr::math::toRadians(45.)), 0, std::cos(flr::math::toRadians(45.)), -3,
			0, 0, 0, 1;
		VertexShader::mvp *= model_matirx;
		render.DrawElements(flr::Primitive::Triangle,
			model.element_buffer_obj_.size(), &(model.element_buffer_obj_[0]));

		FragmentShader::SwapBuffer();
		SDL_UpdateWindowSurface(window);
		if(SDL_PollEvent(&e) && (e.key.keysym.sym == SDLK_ESCAPE) || (e.type == SDL_QUIT))
			break;
	}

	//SDL_UpdateWindowSurface(window);
	//SDL_Event e;
	//while (SDL_WaitEvent(&e) && e.type != SDL_QUIT && e.key.keysym.sym != SDLK_ESCAPE);

	SDL_FreeSurface(texture1);
	SDL_FreeSurface(texture2);
	SDL_DestroyWindow(window);
	SDL_Quit();

	return 0;
}







